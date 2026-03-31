# Plan: CLI and MCP Interface

## Status: Complete

## Goal

Make rpn-lang directly usable as a compute engine by LLMs (Claude) and other
automation. Two delivery mechanisms:

1. **CLI binary** (`rpn`) — stateless or session-based, callable from shell
2. **MCP server** (`rpn-mcp`) — native Claude tool integration via Model Context Protocol

Custom dictionaries (color-dict, future domain-specific dicts) are first-class:
the CLI and MCP server can load them, making rpn-lang a general-purpose
extensible compute environment for LLM-assisted workflows.

---

## Why This Is Useful

- Matrix math, statistics, geometry, unit conversions — all in one tool
- No Python ceremony for stack-oriented computations
- Session state persists across tool calls within a conversation
- Custom dicts (color science, CNC, etc.) make domain-specific computation
  available to the LLM without writing new code per task
- `wordList()` / `wordHelp()` let the LLM discover available operations at runtime
- `describeStack()` returns typed results the LLM can reason about

---

## Architecture

### CLI binary

```
rpn [options] [expression]

Options:
  -s, --session <file>    load/save session state from file
  -f, --file <script>     execute an rpn script file before expression
  -d, --dict <name>       load a named extension dictionary (repeatable)
  --json                  output stack as JSON (default: human-readable)
  --stack                 print full stack (default: TOS only)
  --words                 list all available words as JSON
  --help-word <word>      print wordHelp JSON for a word
```

Examples:
```bash
rpn "3.14159 2. *"                          # → 6.28318
rpn --json "[ 1. 2. 3. ] DUP DOT"          # → [{"type":"double",...}]
rpn -s /tmp/rpn.session "data MEAN"         # stateful
rpn -d color "sRGB ->RGB"                   # with color dict
```

### Session state

Session file is a self-contained rpn script that reconstructs stack and variables
when loaded. Uses `deparse()` for stack items and generates `STO` calls for
variables. Custom words defined via `:` are deparsed and re-compiled.

```rpn
# rpn session file
1.4142135623730951 2.718281828459045    # stack (bottom to top)
42. 'answer' STO                        # variables
: DOUBLE ( n -- 2n ) 2. * ;            # compiled words
```

### Extension dictionaries

Native C++ dictionaries compiled into the CLI binary, activated by name via `-d`.
Each dict registers itself in a static registry at startup:

```cpp
// src/cli-dicts.h
struct CliDict {
    const char *name;
    void (*add)(rpn::Interp &);
};
```

The CLI binary links against whatever dicts are desired; `-d color` looks up
`"color"` in the registry and calls its `add` function. Unknown dict names are
an error. No `dlopen` — keeps deployment simple.

### MCP server

Implements the [Model Context Protocol](https://modelcontextprotocol.io) over
stdio (JSON-RPC). One persistent `rpn::Interp` per server session.

**Tools exposed:**

| Tool | Description |
|---|---|
| `eval` | Evaluate an rpn expression; returns stack as JSON |
| `stack` | Return current stack as JSON without evaluating |
| `clear` | Clear the stack |
| `load_file` | Execute an rpn script file |
| `word_list` | Return all available words grouped by category |
| `word_help` | Return help for a specific word |
| `save_session` | Serialize current state to a file |
| `load_session` | Restore state from a file |

**Tool schema example:**
```json
{
  "name": "eval",
  "description": "Evaluate an RPN expression and return the resulting stack.",
  "inputSchema": {
    "type": "object",
    "properties": {
      "expression": { "type": "string" },
      "show_stack": { "type": "boolean", "default": true }
    },
    "required": ["expression"]
  }
}
```

---

## Source Layout

All new files live in `src/cli/`:

```
src/cli/
  rpn-main.cpp        — CLI entry point
  rpn-mcp.cpp         — MCP server entry point
  session.{h,cpp}     — session serialize/deserialize
  dict-registry.{h,cpp} — named dict registration
  output.{h,cpp}      — JSON/human stack formatting
```

CMake adds two new executables to `src/CMakeLists.txt`:

```cmake
add_executable(rpn src/cli/rpn-main.cpp src/cli/session.cpp
                   src/cli/dict-registry.cpp src/cli/output.cpp)
target_link_libraries(rpn PRIVATE rpn-lang)

add_executable(rpn-mcp src/cli/rpn-mcp.cpp src/cli/session.cpp
                       src/cli/dict-registry.cpp src/cli/output.cpp)
target_link_libraries(rpn-mcp PRIVATE rpn-lang)
```

---

## What Was Built

### Delivered

| Task | Status | Notes |
|---|---|---|
| Task 1 — CLI binary | Done | `cli/rpn-main.cpp`, `cli/output.{h,cpp}`. Single-shot eval, `--json`, `--stack`, `--words`, `--help-word`. |
| Task 2 — Session state | Deferred | `-s/--session` flag parses but returns "not yet implemented". Needs `variables()` API on `rpn::Interp`. |
| Task 3 — Dict registry | Deferred | `-d/--dict` flag parses but returns "not yet implemented". |
| Task 4 — MCP server | Done | `rpn::McpServer` (pimpl, TCP) in `src/rpn-mcp.{h,cpp}`. `rpn --mcp` for stdio transport. `rpn --serve [port]` for TCP. Tools: `eval`, `stack`, `clear`, `load_file`, `word_list`, `word_help`. |
| Task 5 — CMake/install | Done | `cli/CMakeLists.txt` with install rules. Single `rpn` binary. |

### Architecture changes from original design

- `rpn-mcp` merged into `rpn` as `--mcp` (stdio) and `--serve` (TCP) flags — one binary.
- MCP server is `rpn::McpServer` class in the library (`src/`), not a CLI-only concern. Embedders can expose their own `rpn::Interp` (with custom words) via `McpServer` and bridge to Claude with `socat - TCP:localhost:<port>`.
- Transport: both stdio (for Claude Desktop/Code) and TCP (for embedded/long-running app use).

### Future work

See "Possible Future Work" in `development-plan.md`:
- MCP custom tool registration (`McpServer::addTool(...)`)
- socat TCP bridge pattern (documentation)
- Session state (Task 2) — needs `variables()` API
- Dict registry (Task 3)
