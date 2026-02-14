# CLAUDE.md

## Project Overview

H3 Claude Expo — a Heroes of Might and Magic III HD Mod plugin that bridges Claude Code notifications into the HOMM3 in-game chat, with optional bidirectional communication.

## Tech Stack

- **Plugin**: C++ (x86 Windows DLL), targeting HD Mod's patcher_x86.dll + H3API
- **Host daemon**: TBD (likely Python or a small native binary)
- **Claude Code hooks**: Shell scripts / JSON config
- **Build**: MinGW cross-compilation (`i686-w64-mingw32-g++`) on macOS
- **IPC**: File-based initially (`/tmp/h3claude/`), TCP socket later

## Key Dependencies

- [H3API](https://github.com/RoseKavalier/H3API) — reverse-engineered HOMM3 headers
- [patcher_x86.dll](https://github.com/RoseKavalier/H3Plugins) — HD Mod's hooking framework
- HD Mod (unofficial HOMM3 HD Edition by baratorch)
- Wine on macOS

## Project Structure

```
h3-claude-expo/
├── plugin/           # C++ HD Mod plugin (DLL)
│   ├── src/          # Plugin source code
│   ├── include/      # H3API and patcher headers
│   └── Makefile      # Cross-compilation build
├── daemon/           # macOS-side bridge daemon
├── hooks/            # Claude Code hook configurations
├── docs/             # Research, notes, reverse engineering
├── MILESTONES.md     # Project milestones and progress
├── README.md
└── CLAUDE.md
```

## Important Context

- HOMM3 runs inside Wine on macOS. The plugin is a Windows DLL loaded inside Wine.
- Wine maps Z:\ to the macOS root filesystem — this is how file-based IPC works.
- The HD Mod chat (TAB key) works in single player too, not just multiplayer.
- HD Mod plugins are loaded from `_HD3_Data/Packs/<PluginName>/` directories.
- Plugins use `GetPatcher()` → `CreateInstance()` → `WriteHiHook()`/`WriteLoHook()` pattern.
- H3API uses the `h3` namespace for all game structures.

## Build Commands

```bash
# Build plugin
make plugin

# Build daemon
make daemon

# Run tests (daemon/host-side only — plugin must be tested in-game)
make test
```

## Conventions

- Keep the plugin code minimal and focused — complex logic goes in the daemon
- Use file-based IPC for the initial implementation, upgrade to TCP later
- Plugin polls for new messages (no threading in the plugin if avoidable)
- All IPC files go under `/tmp/h3claude/` (mapped as `Z:\tmp\h3claude\` in Wine)
- Message format: one JSON object per line (newline-delimited JSON)
