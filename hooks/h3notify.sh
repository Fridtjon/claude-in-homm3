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

# Use CWD basename as session identifier
SESSION_NAME=$(basename "${CWD:-unknown}")

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
