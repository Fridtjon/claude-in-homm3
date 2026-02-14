# Milestones

## Milestone 1: Skeleton Plugin (Build & Load)
> Get a minimal HD Mod plugin that compiles on macOS via MinGW and loads in HOMM3.

- [ ] Set up MinGW cross-compilation toolchain
- [ ] Obtain H3API headers and patcher_x86 headers/lib
- [ ] Create minimal DLL with `DllMain` + `GetPatcher()` + `CreateInstance()`
- [ ] Build Makefile for cross-compilation
- [ ] Test that the plugin loads in HOMM3 (HD Mod log confirms it)

**Done when:** Plugin loads without crashing HOMM3. Visible in HD Mod plugin list or logs.

---

## Milestone 2: Display Text In-Game
> Show a hardcoded message in the HOMM3 chat or status bar from the plugin.

- [ ] Research chat display hooks (H3API chat structures, message rendering)
- [ ] Hook into chat display or status bar text
- [ ] Display a test message ("Hello from Claude") on game load or key press
- [ ] Verify text appears correctly in-game

**Done when:** A hardcoded message appears in the game UI.

---

## Milestone 3: File-Based IPC (One-Way: Host → Game)
> Read messages from a file on the macOS host and display them in-game.

- [ ] Define message file format (newline-delimited JSON in `/tmp/h3claude/inbox.jsonl`)
- [ ] Plugin polls the file on a timer (every ~1 second, hooked to game loop)
- [ ] Parse new lines, display as chat messages
- [ ] Write a test script that appends messages to the file
- [ ] Handle file rotation / cleanup

**Done when:** Running `echo '{"from":"claude-1","text":"Task complete!"}' >> /tmp/h3claude/inbox.jsonl` shows the message in HOMM3 chat.

---

## Milestone 4: Claude Code Integration
> Wire up Claude Code hooks so that real Claude events trigger in-game notifications.

- [ ] Create Claude Code hook config (PostToolUse / Stop events)
- [ ] Hook script formats event data and writes to `/tmp/h3claude/inbox.jsonl`
- [ ] Test with a live Claude Code session
- [ ] Support multiple Claude instances (include instance ID in messages)

**Done when:** Playing HOMM3 while Claude Code works in the background, you see notifications appear in chat when Claude finishes tasks.

---

## Milestone 5: Bidirectional Communication (Game → Host)
> Send text from the HOMM3 chat field back to the host.

- [ ] Hook chat input submission (intercept text entered via TAB → Enter)
- [ ] Write outgoing messages to `/tmp/h3claude/outbox.jsonl`
- [ ] Create host-side daemon that watches outbox and routes commands
- [ ] Define command protocol (e.g., `@1 do something` routes to Claude instance 1)
- [ ] Test round-trip: type in HOMM3 → daemon receives → response appears in HOMM3

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
