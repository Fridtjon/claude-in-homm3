#!/bin/bash
# h3notify.sh — Claude Code hook that sends notifications to HOMM3
# Writes messages to /tmp/h3claude/inbox.jsonl, read by the H3.ClaudeExpo plugin.
#
# Supports both JSON and plain text formats.
# JSON: {"from":"session-name","text":"message","event":"Stop"}
# Plain: just a string displayed as-is in chat.

set -uo pipefail

INBOX="/tmp/h3claude/inbox.jsonl"
mkdir -p /tmp/h3claude

# Get the hook event type from Claude Code's environment
EVENT="${HOOK_EVENT:-unknown}"

# Try to identify this Claude session (use CWD basename as session name)
SESSION_NAME="${CLAUDE_SESSION_NAME:-$(basename "${PWD:-unknown}")}"

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
