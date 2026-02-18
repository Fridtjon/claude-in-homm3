# Claude In Homm 3 (Heroes of Might and Magic III)

A Heroes of Might and Magic III (HD Mod) plugin that displays Claude Code notifications in the in-game chat — so you can play HOMM3 while your AI agents work in the background.

## What is this?

When you're playing HOMM3 and running Claude Code instances in terminal windows, this plugin lets you see what your Claude agents are doing without leaving the game. Messages appear directly in the HOMM3 chat overlay.

HOMM3 runs inside Wine on macOS. Claude Code runs natively. A file-based IPC bridge connects them.

## Demo

Messages appear in the HOMM3 chat area:

```
>> H3.ClaudeExpo active - Claude Code notifications enabled
[21:30:45 claude-a2] Done working
[21:31:12 h3-claude-expo] Needs attention
```

## Architecture

```
Terminal 1: Claude Code ──┐                    ┌─── Wine / HOMM3
Terminal 2: Claude Code ──┼── h3notify.sh ──▶  │    HD Mod plugin
Terminal 3: Claude Code ──┘    (hook)          │    reads inbox.jsonl
                                               │    displays in chat
           /tmp/h3claude/inbox.jsonl           └────────────────────
```

### Components

1. **HD Mod Plugin** (`plugin/`) — C++ DLL loaded by HOMM3's HD Mod. Polls an inbox file on a 1-second timer and displays new messages via `ScreenChat::Show`.
2. **Claude Code Hook** (`hooks/h3notify.sh`) — Bash script triggered on Claude Code `Stop` and `Notification` events. Writes JSON lines to the inbox file.
3. **Test Script** (`hooks/test_notify.sh`) — Send messages manually from the terminal.

## Requirements

- Heroes of Might and Magic III: Shadow of Death (v3.2) with [HD Mod](https://sites.google.com/site/heroes3hd/)
- Wine (macOS)
- MinGW cross-compiler for building the plugin DLL
- Claude Code (for the notification source — but the plugin works with anything that writes to the inbox)

## Quick Start

### 1. Build the plugin

```bash
brew install mingw-w64    # one-time setup
git clone https://github.com/Fridtjon/claude-in-homm3.git
cd claude-in-homm3

# Clone vendored headers (needed for build)
git clone https://github.com/RoseKavalier/H3API.git plugin/include/H3API
git clone https://github.com/RoseKavalier/H3Plugins.git plugin/include/H3Plugins
cp plugin/include/H3Plugins/H3API/lib/h3api/../../../H3API/Single\ Header/H3API.hpp plugin/include/
cp plugin/include/H3Plugins/H3API/lib/patcher_x86.hpp plugin/include/
cp plugin/include/H3Plugins/H3API/lib/Plugin.hpp plugin/include/
cp plugin/include/H3Plugins/H3API/lib/H3Types.hpp plugin/include/

make plugin
```

### 2. Install the plugin

```bash
# Copy DLL to your HOMM3 HD Mod packs directory
mkdir -p "<HOMM3_DIR>/_HD3_Data/Packs/H3.ClaudeExpo"
cp build/H3.ClaudeExpo.dll "<HOMM3_DIR>/_HD3_Data/Packs/H3.ClaudeExpo/"
```

### 3. Test it

Launch HOMM3, start a game, and on the adventure map:

```bash
# From another terminal:
./hooks/test_notify.sh "Hello from macOS!"
```

The message should appear in the HOMM3 chat within ~1 second.

### 4. Wire up Claude Code hooks

Add to `~/.claude/settings.json` (inside the `"hooks"` object, alongside any existing hooks):

```json
"Stop": [
  {
    "matcher": "",
    "hooks": [
      {
        "type": "command",
        "command": "/path/to/claude-in-homm3/hooks/h3notify.sh",
        "timeout": 5
      }
    ]
  }
],
"Notification": [
  {
    "matcher": "",
    "hooks": [
      {
        "type": "command",
        "command": "/path/to/claude-in-homm3/hooks/h3notify.sh",
        "timeout": 5
      }
    ]
  }
]
```

Now when any Claude Code session finishes or needs attention, you'll see it in HOMM3.

## Not just for Claude

The plugin is completely generic. Anything that appends a line to `/tmp/h3claude/inbox.jsonl` shows up in the game:

```bash
# Plain text
echo "Build complete!" >> /tmp/h3claude/inbox.jsonl

# JSON (with sender name and timestamp)
echo '{"from":"ci","text":"Tests passed","time":"14:30:00"}' >> /tmp/h3claude/inbox.jsonl
```

Use it for CI/CD notifications, system alerts, cron jobs, or anything else.

## Project Status

See [MILESTONES.md](MILESTONES.md) for detailed progress.

- **Milestones 1-4**: Complete (build, display, IPC, Claude integration)
- **Milestone 5**: Planned (bidirectional — type in HOMM3, send to Claude)
- **Milestone 6**: Planned (TCP sockets for lower latency)
- **Milestone 7**: Planned (colors, sounds, polish)

## License

MIT
