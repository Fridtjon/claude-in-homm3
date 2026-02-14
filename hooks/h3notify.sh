#!/bin/bash
# h3notify.sh — Claude Code hook that sends notifications to HOMM3
# Writes messages to /tmp/h3claude/inbox.jsonl, read by the H3.ClaudeExpo plugin.
#
# Claude Code passes hook context as JSON on stdin, including:
#   hook_event_name, session_id, cwd, and event-specific fields.

set -uo pipefail

INBOX="/tmp/h3claude/inbox.jsonl"
mkdir -p /tmp/h3claude

# Read JSON context from stdin
INPUT=$(cat)

# Extract fields using jq (falls back gracefully if jq missing)
if command -v jq &>/dev/null; then
    EVENT=$(echo "$INPUT" | jq -r '.hook_event_name // "unknown"')
    CWD=$(echo "$INPUT" | jq -r '.cwd // ""')
else
    # Fallback: rough extraction without jq
    EVENT=$(echo "$INPUT" | grep -o '"hook_event_name":"[^"]*"' | cut -d'"' -f4)
    CWD=$(echo "$INPUT" | grep -o '"cwd":"[^"]*"' | cut -d'"' -f4)
    EVENT="${EVENT:-unknown}"
fi

# Identify session by finding the git root, then checking if it's a worktree.
#
# For worktrees (claude-a2/web-client-dolphin):
#   git root = web-client-dolphin, parent = claude-a2 -> use "claude-a2"
#
# For standalone repos (h3-claude-expo):
#   git root = h3-claude-expo -> use "h3-claude-expo"
#
# This is stable regardless of which subdirectory Claude has cd'd into.
SESSION_NAME="unknown"
if [ -n "$CWD" ]; then
    GIT_ROOT=$(git -C "$CWD" rev-parse --show-toplevel 2>/dev/null) || true

    if [ -n "$GIT_ROOT" ]; then
        # Check if this is a git worktree (.git is a file, not a directory)
        if [ -f "$GIT_ROOT/.git" ]; then
            # Worktree — use the parent directory (workstation name)
            SESSION_NAME=$(basename "$(dirname "$GIT_ROOT")")
        else
            # Normal repo — use the repo name
            SESSION_NAME=$(basename "$GIT_ROOT")
        fi
    else
        SESSION_NAME=$(basename "$CWD")
    fi
fi

# Map events to short, informative messages
case "$EVENT" in
    Stop)
        MSG="Done working"
        ;;
    Notification)
        MSG="Needs attention"
        ;;
    SessionStart)
        MSG="Session started"
        ;;
    UserPromptSubmit)
        # Don't notify on every prompt — too noisy
        exit 0
        ;;
    SubagentStop)
        MSG="Subagent finished"
        ;;
    *)
        MSG="Event: $EVENT"
        ;;
esac

# Write JSON message to inbox
echo "{\"from\":\"$SESSION_NAME\",\"text\":\"$MSG\",\"event\":\"$EVENT\"}" >> "$INBOX"
