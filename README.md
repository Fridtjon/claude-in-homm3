# Claude In Homm 3 (Heroes of Might and Magic III)

A Heroes of Might and Magic III (HD Mod) plugin that turns the in-game chat into a terminal for Claude Code notifications — and optionally, bidirectional communication.

## What is this?

When you're playing HOMM3 and running Claude Code instances in terminal windows, this plugin lets you see what your Claude agents are doing without leaving the game. Messages from Claude appear in the HOMM3 chat. You can even send commands back.

HOMM3 runs inside Wine on macOS. Claude Code runs natively. This project bridges the two.

## Architecture

```
Terminal 1: Claude Code ──┐
Terminal 2: Claude Code ──┼── hook/script ──▶ IPC (file or TCP)
Terminal 3: Claude Code ──┘                        │
                                                   ▼
                                       Wine / HOMM3 HD Mod plugin
                                       displays in chat, accepts input
```

### Components

1. **HD Mod Plugin** (`plugin/`) — C++ DLL loaded by HOMM3's HD Mod. Hooks the chat system to display incoming messages and capture outgoing commands.
2. **Host Daemon** (`daemon/`) — Lightweight macOS-side process that bridges IPC between Claude Code and the plugin.
3. **Claude Code Hooks** (`hooks/`) — Claude Code hook configurations that fire notifications on key events (task completion, errors, etc.).

## Requirements

- Heroes of Might and Magic III with HD Mod (unofficial HD Edition)
- Wine (macOS)
- MinGW cross-compiler (`i686-w64-mingw32-g++`) for building the plugin DLL
- Claude Code (for the notification source)

## Building

```bash
# Install MinGW cross-compiler (macOS)
brew install mingw-w64

# Build the plugin
make plugin

# The output DLL goes into your HD Mod packs directory
```

## Installation

1. Build the plugin DLL
2. Copy it to `<HOMM3>/_HD3_Data/Packs/H3ClaudeExpo/`
3. Configure the Claude Code hooks (see `hooks/README.md`)
4. Start the host daemon
5. Launch HOMM3 — press TAB to see Claude messages in chat

## License

MIT
