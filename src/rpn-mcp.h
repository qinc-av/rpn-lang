/***************************************************
 * file: QInc/Projects/RP42/rpn-lang/src/rpn-mcp.h
 *
 * @brief   MCP server for rpn::Interp.
 *
 * rpn::McpServer owns the TCP socket and accept thread.
 * The caller owns the Interp and is responsible for keeping it alive
 * for the lifetime of the McpServer.
 *
 * Usage:
 *   rpn::Interp interp(true);            // async mode required
 *   rpn::McpServer server(interp, 5150); // starts background thread
 *   ...
 *   // ~McpServer() stops the server and joins the thread
 */

#pragma once
#include <memory>
#include "rpn.h"

namespace rpn {

// Run MCP over stdio (stdin→stdout, newline-delimited JSON-RPC).
// Blocks until EOF. interp must be in async mode.
void mcp_run_stdio(rpn::Interp &interp);

class McpServer {
public:
  McpServer(rpn::Interp &interp, int port);
  ~McpServer();

  McpServer(const McpServer &) = delete;
  McpServer &operator=(const McpServer &) = delete;

private:
  struct Impl;
  std::unique_ptr<Impl> _impl;
};

} // namespace rpn

/* end of QInc/Projects/RP42/rpn-lang/src/rpn-mcp.h */
