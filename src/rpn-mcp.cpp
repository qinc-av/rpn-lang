/***************************************************
 * file: QInc/Projects/RP42/rpn-lang/src/rpn-mcp.cpp
 *
 * @brief   MCP server implementation (rpn::McpServer).
 *
 * Transport: JSON-RPC 2.0, newline-delimited, over TCP.
 * One shared Interp per server; all connections share stack state.
 * Eval calls are serialized through the Interp's async queue.
 */

#include <iostream>
#include <sstream>
#include <map>
#include <thread>
#include <future>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define JSON_NO_IO
#include "nlohmann/json.hpp"
#include "rpn-mcp.h"

using json = nlohmann::json;

static const char *kProtocolVersion = "2024-11-05";
static const char *kServerName      = "rpn-mcp";
static const char *kServerVersion   = "1.0";

// ---------------------------------------------------------------------------
// JSON-RPC helpers
// ---------------------------------------------------------------------------

static json jok(const json &id, json result) {
  return {{"jsonrpc","2.0"}, {"id",id}, {"result",std::move(result)}};
}
static json jerr(const json &id, int code, const std::string &msg) {
  return {{"jsonrpc","2.0"}, {"id",id}, {"error",{{"code",code},{"message",msg}}}};
}
static json tool_result(const std::string &text, bool is_error = false) {
  return {{"content", json::array({{{"type","text"},{"text",text}}})}, {"isError",is_error}};
}

// ---------------------------------------------------------------------------
// Stack helpers
// ---------------------------------------------------------------------------

static std::string stack_text(rpn::Interp &interp) {
  size_t depth = interp.stack.depth();
  if (depth == 0) return "(empty stack)";
  std::ostringstream os;
  os << "Stack (" << depth << " item" << (depth==1?"":"s") << ", TOS last):\n";
  for (int i = (int)depth; i >= 1; --i) {
    const auto &obj = interp.stack.peek(i);
    os << "  " << i << ": " << (std::string)obj << "  [" << obj.type_name() << "]\n";
  }
  return os.str();
}

// ---------------------------------------------------------------------------
// Sync wrappers for async eval
// ---------------------------------------------------------------------------

static rpn::WordDefinition::Result await_eval(rpn::Interp &interp, const std::string &expr) {
  std::promise<rpn::WordDefinition::Result> p;
  auto f = p.get_future();
  interp.eval(expr, [&p](rpn::WordDefinition::Result r){ p.set_value(r); });
  return f.get();
}

static rpn::WordDefinition::Result await_parse_file(rpn::Interp &interp, const std::string &path) {
  std::promise<rpn::WordDefinition::Result> p;
  auto f = p.get_future();
  interp.parseFile(path, [&p](rpn::WordDefinition::Result r){ p.set_value(r); });
  return f.get();
}

// ---------------------------------------------------------------------------
// Tool definitions
// ---------------------------------------------------------------------------

static json tools_list() {
  return json::array({
    {{"name","eval"},
     {"description","Evaluate an RPN expression. Stack state persists across calls."},
     {"inputSchema",{{"type","object"},{"properties",{{"expression",{{"type","string"},{"description","RPN expression to evaluate"}}}}},{"required",{"expression"}}}}},
    {{"name","stack"},
     {"description","Return the current stack contents."},
     {"inputSchema",{{"type","object"},{"properties",json::object()}}}},
    {{"name","clear"},
     {"description","Clear the stack."},
     {"inputSchema",{{"type","object"},{"properties",json::object()}}}},
    {{"name","load_file"},
     {"description","Execute an RPN script file."},
     {"inputSchema",{{"type","object"},{"properties",{{"path",{{"type","string"},{"description","Path to .rpn script"}}}}},{"required",{"path"}}}}},
    {{"name","word_list"},
     {"description","List all available words grouped by category."},
     {"inputSchema",{{"type","object"},{"properties",json::object()}}}},
    {{"name","word_help"},
     {"description","Get documentation for a specific word."},
     {"inputSchema",{{"type","object"},{"properties",{{"word",{{"type","string"},{"description","Word name"}}}}},{"required",{"word"}}}}}
  });
}

// ---------------------------------------------------------------------------
// Tool dispatch
// ---------------------------------------------------------------------------

static json handle_tool(const std::string &name, const json &args, rpn::Interp &interp) {
  if (name == "eval") {
    if (!args.contains("expression") || !args["expression"].is_string())
      return tool_result("missing required argument: expression", true);
    auto r = await_eval(interp, args["expression"].get<std::string>());
    if (r != rpn::WordDefinition::Result::ok)
      return tool_result("error: " + std::string(interp.status()), true);
    return tool_result(stack_text(interp));

  } else if (name == "stack") {
    return tool_result(stack_text(interp));

  } else if (name == "clear") {
    auto r = await_eval(interp, "CLEAR");
    if (r != rpn::WordDefinition::Result::ok)
      return tool_result("error: " + std::string(interp.status()), true);
    return tool_result("stack cleared");

  } else if (name == "load_file") {
    if (!args.contains("path") || !args["path"].is_string())
      return tool_result("missing required argument: path", true);
    auto r = await_parse_file(interp, args["path"].get<std::string>());
    if (r != rpn::WordDefinition::Result::ok)
      return tool_result("error: " + std::string(interp.status()), true);
    return tool_result(stack_text(interp));

  } else if (name == "word_list") {
    std::map<std::string, std::vector<std::string>> byCategory;
    for (const auto &w : interp.wordList()) {
      auto h = interp.wordHelp(w);
      byCategory[h.category.empty() ? "other" : h.category].push_back(w);
    }
    std::ostringstream os;
    for (const auto &[cat, words] : byCategory) {
      os << cat << ": ";
      for (size_t i = 0; i < words.size(); ++i) { if (i) os << ", "; os << words[i]; }
      os << "\n";
    }
    return tool_result(os.str());

  } else if (name == "word_help") {
    if (!args.contains("word") || !args["word"].is_string())
      return tool_result("missing required argument: word", true);
    std::string word = args["word"].get<std::string>();
    auto h = interp.wordHelp(word);
    if (h.description.empty() && h.effects.empty())
      return tool_result("unknown word: " + word, true);
    std::ostringstream os;
    os << h.name;
    if (!h.category.empty()) os << " [" << h.category << "]";
    os << "\n";
    if (!h.description.empty()) os << h.description << "\n";
    for (const auto &e : h.effects) os << "  " << e << "\n";
    return tool_result(os.str());
  }

  return tool_result("unknown tool: " + name, true);
}

// ---------------------------------------------------------------------------
// MCP message dispatch
// ---------------------------------------------------------------------------

static std::optional<json> handle_message(const json &msg, rpn::Interp &interp) {
  if (!msg.contains("method")) return std::nullopt;
  std::string method = msg["method"].get<std::string>();
  bool has_id = msg.contains("id");
  json id = has_id ? msg["id"] : json(nullptr);

  if (method == "notifications/initialized") return std::nullopt;
  if (method == "initialize") {
    return jok(id, {{"protocolVersion",kProtocolVersion},
                    {"capabilities",{{"tools",json::object()}}},
                    {"serverInfo",{{"name",kServerName},{"version",kServerVersion}}}});
  }
  if (method == "ping") return jok(id, json::object());
  if (method == "tools/list") return jok(id, {{"tools", tools_list()}});
  if (method == "tools/call") {
    if (!msg.contains("params")) return jerr(id, -32602, "missing params");
    const auto &p = msg["params"];
    if (!p.contains("name") || !p["name"].is_string())
      return jerr(id, -32602, "missing tool name");
    return jok(id, handle_tool(p["name"].get<std::string>(),
                               p.value("arguments", json::object()), interp));
  }
  if (has_id) return jerr(id, -32601, "method not found: " + method);
  return std::nullopt;
}

// ---------------------------------------------------------------------------
// Per-connection handler
// ---------------------------------------------------------------------------

static void handle_connection(int fd, rpn::Interp &interp) {
  FILE *f = fdopen(fd, "r+");
  if (!f) { ::close(fd); return; }
  char buf[65536];
  while (fgets(buf, sizeof(buf), f)) {
    std::string line(buf);
    if (!line.empty() && line.back() == '\n') line.pop_back();
    if (line.empty()) continue;
    json msg;
    try { msg = json::parse(line); }
    catch (...) {
      std::string out = json({{"jsonrpc","2.0"},{"id",nullptr},
                               {"error",{{"code",-32700},{"message","parse error"}}}}).dump() + "\n";
      fwrite(out.c_str(), 1, out.size(), f); fflush(f); continue;
    }
    auto response = handle_message(msg, interp);
    if (response.has_value()) {
      std::string out = response->dump() + "\n";
      fwrite(out.c_str(), 1, out.size(), f); fflush(f);
    }
  }
  fclose(f);
}

// ---------------------------------------------------------------------------
// Stdio transport
// ---------------------------------------------------------------------------

void rpn::mcp_run_stdio(rpn::Interp &interp) {
  std::string line;
  while (std::getline(std::cin, line)) {
    if (line.empty()) continue;
    json msg;
    try { msg = json::parse(line); }
    catch (...) {
      std::cout << json({{"jsonrpc","2.0"},{"id",nullptr},
                         {"error",{{"code",-32700},{"message","parse error"}}}}).dump() << "\n";
      std::cout.flush();
      continue;
    }
    auto response = handle_message(msg, interp);
    if (response.has_value()) {
      std::cout << response->dump() << "\n";
      std::cout.flush();
    }
  }
}

// ---------------------------------------------------------------------------
// McpServer::Impl — owns socket and accept thread
// ---------------------------------------------------------------------------

struct rpn::McpServer::Impl {
  rpn::Interp &interp;
  int fd{-1};
  std::thread thread;

  Impl(rpn::Interp &interp_, int port) : interp(interp_) {
    fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { std::cerr << "McpServer: socket failed\n"; return; }

    int yes = 1;
    ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons((uint16_t)port);

    if (::bind(fd, (sockaddr *)&addr, sizeof(addr)) < 0) {
      std::cerr << "McpServer: bind failed on port " << port << "\n";
      ::close(fd); fd = -1; return;
    }
    ::listen(fd, 8);

    thread = std::thread([this, port]() {
      std::cerr << "rpn-mcp: listening on port " << port << "\n";
      while (fd >= 0) {
        int client = ::accept(fd, nullptr, nullptr);
        if (client < 0) break;
        std::thread([this, client]() {
          handle_connection(client, interp);
        }).detach();
      }
    });
  }

  ~Impl() {
    if (fd >= 0) { ::close(fd); fd = -1; }
    if (thread.joinable()) thread.join();
  }
};

// ---------------------------------------------------------------------------
// McpServer public interface
// ---------------------------------------------------------------------------

rpn::McpServer::McpServer(rpn::Interp &interp, int port)
  : _impl(std::make_unique<Impl>(interp, port)) {}

rpn::McpServer::~McpServer() = default;

/* end of QInc/Projects/RP42/rpn-lang/src/rpn-mcp.cpp */
