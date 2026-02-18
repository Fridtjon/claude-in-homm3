# Milestones

## Milestone 1: Skeleton Plugin (Build & Load) ✅

> Get a minimal HD Mod plugin that compiles on macOS via MinGW and loads in HOMM3.

- [x] Set up MinGW cross-compilation toolchain (`brew install mingw-w64`)
- [x] Obtain patcher_x86 headers (cloned from H3Plugins, patched for GCC)
- [x] Create minimal DLL with `DllMain` + `GetPatcher()` + `CreateInstance()`
- [x] Build Makefile for cross-compilation (`i686-w64-mingw32-g++`)
- [x] Test that the plugin loads in HOMM3 (confirmed via `/tmp/h3claude/plugin.log`)

**Notes:**
- H3API single header (35K lines) has too many MSVC-isms for MinGW. We use patcher_x86.hpp directly and call game functions by address.
- Created `mingw_compat.h` with `#define __intN` macros for GCC compatibility.
- Patched `patcher_x86.hpp`: replaced `__asm{__asm int 3}` with `__builtin_trap()`.

---

## Milestone 2: Display Text In-Game ✅

> Show a message in the HOMM3 chat from the plugin.

- [x] Research chat display hooks (found `ScreenChat::Show` at `0x553C40`)
- [x] Hook adventure map hint update (`0x40D0DB`) as a periodic callback
- [x] Call `ScreenChat::Show` via direct function pointer cast (CDECL)
- [x] Display startup confirmation message on first adventure map mouse-move
- [x] Added file-based logging to `/tmp/h3claude/plugin.log`

**Key finding:** `ScreenChat::Show` signature is `CDECL_3(VOID, 0x553C40, chatPtr, "%s", text)`. The ScreenChat singleton pointer is at `*(void**)0x405F28`.

---

## Milestone 3: File-Based IPC (One-Way: Host → Game) ✅

> Read messages from a file on the macOS host and display them in-game.

- [x] Message format: newline-delimited JSON in `/tmp/h3claude/inbox.jsonl`
- [x] Plugin polls file with tracked offset (only reads new lines)
- [x] Parse JSON fields (`from`, `text`, `time`) with minimal hand-rolled parser
- [x] Supports both JSON and plain text lines
- [x] Created `test_notify.sh` for manual testing
- [x] Plugin skips old messages on startup (seeks to EOF)
- [x] Replaced mouse-move-based polling with Windows `SetTimer` (1s interval, WM_TIMER)

**IPC path:** `/tmp/h3claude/inbox.jsonl` on macOS = `Z:\tmp\h3claude\inbox.jsonl` in Wine.

---

## Milestone 4: Claude Code Integration ✅

> Wire up Claude Code hooks so that real Claude events trigger in-game notifications.

- [x] Created `hooks/h3notify.sh` — reads hook JSON from stdin, writes to inbox
- [x] Registered in `~/.claude/settings.json` on `Stop` and `Notification` events
- [x] Session identification: uses git root for normal repos, parent dir for worktrees
- [x] Messages include HH:MM:SS timestamp
- [x] Tested with multiple concurrent Claude Code sessions
- [x] Messages display as: `[21:30:45 claude-a2] Done working`

**Hook protocol:** Claude Code passes JSON on stdin with `hook_event_name`, `cwd`, `session_id`, etc. The script extracts these via `jq` (with grep fallback).

---

## Milestone 5: Bidirectional Communication (Game → Host)

> Send text from the HOMM3 chat field back to the host.

- [ ] Hook chat input submission (intercept text entered via TAB → Enter)
- [ ] Write outgoing messages to `/tmp/h3claude/outbox.jsonl`
- [ ] Create host-side daemon that watches outbox and routes commands
- [ ] Define command protocol (e.g., `@1 do something` routes to Claude instance 1)
- [ ] Test round-trip: type in HOMM3 → daemon receives → response appears in HOMM3

**Research needed:** Find the chat input handling function address. The chat UI is TAB-activated — need to hook the text submission handler, not just the display.

**Done when:** You can type a message in HOMM3 chat and it reaches a Claude Code instance.

---

## Milestone 6: TCP Socket Upgrade

> Replace file polling with a proper TCP socket for lower latency.

- [ ] Plugin opens TCP listener on localhost (inside Wine)
- [ ] Daemon connects and pushes messages in real-time
- [ ] Bidirectional protocol over TCP
- [ ] Fallback to file-based if TCP fails

**Done when:** Messages appear in-game with <100ms latency.

---

## Milestone 7: Polish & UX

> Make it feel good to use.

- [ ] Color-coded messages (different colors per Claude instance)
- [ ] Notification sound (play HOMM3 sound effect on new message)
- [ ] Message history / scrollback
- [ ] Configurable hotkey (not just TAB)
- [ ] Clean install/uninstall process
- [ ] Documentation and screenshots

**Done when:** You'd actually want to use this while playing.

---

## Stretch Goals

- [ ] Overlay window instead of chat (custom drawn panel)
- [ ] Show Claude's task progress as a quest log entry
- [ ] Trello card status updates in-game
- [ ] Voice notifications using HOMM3 narrator voice
- [ ] "Advisor" portrait that speaks Claude's messages
