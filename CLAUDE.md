# CLAUDE.md

## Project Overview

H3 Claude Expo — a Heroes of Might and Magic III HD Mod plugin that bridges Claude Code notifications into the HOMM3 in-game chat, with optional bidirectional communication.

**GitHub**: https://github.com/Fridtjon/claude-in-homm3

## Current Status

Milestones 1–4 are complete. The plugin compiles, loads in HOMM3 via Wine, displays messages in the in-game chat, reads from a file-based IPC inbox, and is wired up to Claude Code hooks. See MILESTONES.md for details and next steps.

## Tech Stack

- **Plugin**: C++ (x86 Windows DLL), targeting HD Mod's patcher_x86.dll
- **Claude Code hooks**: Bash script (`hooks/h3notify.sh`)
- **Build**: MinGW cross-compilation (`i686-w64-mingw32-g++`) on macOS
- **IPC**: File-based (`/tmp/h3claude/inbox.jsonl`), TCP socket planned for later

## Key Dependencies

- [H3API](https://github.com/RoseKavalier/H3API) — reverse-engineered HOMM3 headers (vendored in `plugin/include/`, gitignored)
- [patcher_x86.dll](https://github.com/RoseKavalier/H3Plugins) — HD Mod's hooking framework (header vendored, DLL provided by HD Mod at runtime)
- HD Mod (unofficial HOMM3 HD Edition by baratorch)
- Wine on macOS
- MinGW (`brew install mingw-w64`)

## Project Structure

```
h3-claude-expo/
├── plugin/
│   ├── src/
│   │   ├── dllmain.cpp          # DLL entry point (minimal)
│   │   ├── H3ClaudeExpo.hpp     # Plugin header: constants, addresses, declarations
│   │   ├── H3ClaudeExpo.cpp     # Plugin implementation: hooks, IPC, chat display
│   │   ├── patcher_x86.hpp      # Patched copy of patcher_x86 header (GCC compat)
│   │   └── mingw_compat.h       # MinGW/MSVC compatibility defines (__intN types)
│   └── include/                  # Vendored deps (gitignored, cloned from GitHub)
│       ├── H3API/                # Full H3API repo (not used directly yet)
│       ├── H3Plugins/            # H3Plugins repo (examples, reference)
│       ├── H3API.hpp             # Single-header H3API (has MinGW issues, not used yet)
│       └── patcher_x86.hpp       # Original unpatched header (reference)
├── hooks/
│   ├── h3notify.sh              # Claude Code hook: writes events to inbox.jsonl
│   └── test_notify.sh           # Manual test: send a message to HOMM3
├── daemon/                       # Future: macOS-side bridge daemon
├── docs/                         # Future: research notes
├── Makefile                      # Cross-compilation build
├── MILESTONES.md
├── README.md
└── CLAUDE.md
```

## Build Commands

```bash
# Build plugin DLL (outputs to build/H3.ClaudeExpo.dll)
make plugin

# Clean build artifacts
make clean

# Install DLL to HOMM3 packs directory (set HOMM3_DIR first)
make install
```

## Important Context

### Wine / HOMM3

- HOMM3 runs inside Wine on macOS. The plugin is a 32-bit Windows DLL loaded inside Wine.
- Wine maps `Z:\` to the macOS root filesystem — this is how file-based IPC works.
- The HD Mod chat (TAB key) works in single player too, not just multiplayer.
- HD Mod plugins are loaded from `_HD3_Data/Packs/<PluginName>/` directories.
- Plugins use `GetPatcher()` → `CreateInstance()` → `WriteLoHook()`/`WriteHiHook()` pattern.

### Key HOMM3 Addresses (SoD 3.2)

These are hardcoded in `H3ClaudeExpo.hpp`:

| Address | Purpose |
|---------|---------|
| `0x553C40` | `ScreenChat::Show` — display chat message (CDECL) |
| `0x405F28` | Pointer to H3ScreenChat singleton |
| `0x697428` | Global text buffer (`h3_TextBuffer`) |
| `0x6992B8` | Adventure manager global pointer |
| `0x40D0DB` | Adventure map hint update hook point |

### MinGW Compatibility

The vendored headers are written for MSVC. We maintain compatibility via:
- `mingw_compat.h` — `#define __int8/16/32/64` as standard C types
- `patcher_x86.hpp` (in `src/`) — patched copy with `__asm` → `__builtin_trap()` for GCC
- The full H3API single header (`H3API.hpp`) has too many MSVC-isms and is NOT used currently. We call game functions by direct address instead.

### IPC Protocol

- **Inbox**: `/tmp/h3claude/inbox.jsonl` (host → game)
- **Outbox**: `/tmp/h3claude/outbox.jsonl` (game → host, not yet implemented)
- Format: one JSON object per line: `{"from":"name","text":"msg","event":"Stop","time":"HH:MM:SS"}`
- Plugin tracks file read offset, only processes new lines
- Plugin skips messages written before it initialized (seeks to EOF on startup)
- Plain text lines (non-JSON) are also supported and displayed as-is

### Plugin Architecture

1. **DLL load** → global constructor runs `PluginInit()`
2. **PluginInit** → registers with patcher_x86, installs adventure map LoHook
3. **First adventure map mouse-move** → shows startup message, seeks inbox to EOF, starts Windows timer
4. **WM_TIMER (every 1s)** → `PollInbox()` reads new lines, calls `ChatShow()` for each
5. **ChatShow** → calls HOMM3's `ScreenChat::Show` at `0x553C40` to display in chat

### Claude Code Hook

The hook (`hooks/h3notify.sh`) is registered in `~/.claude/settings.json` on `Stop` and `Notification` events. It:
1. Reads JSON context from stdin (Claude Code hook protocol)
2. Extracts `hook_event_name` and `cwd`
3. Identifies the session by git root (worktrees: parent dir name, normal repos: repo name)
4. Appends a JSON line to `/tmp/h3claude/inbox.jsonl`

## Conventions

- Keep the plugin code minimal — complex logic goes in the daemon (future)
- Use file-based IPC for now, upgrade to TCP later
- No threading in the plugin — use Windows timers for periodic work
- All IPC files go under `/tmp/h3claude/` (mapped as `Z:\tmp\h3claude\` in Wine)
- Message format: newline-delimited JSON
- Plugin logs to `/tmp/h3claude/plugin.log` for debugging
