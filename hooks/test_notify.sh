#!/bin/bash
# test_notify.sh — Send a test message to HOMM3 while the game is running
# Usage: ./hooks/test_notify.sh "Hello from the terminal!"
# Usage: ./hooks/test_notify.sh  (sends a default test message)

INBOX="/tmp/h3claude/inbox.jsonl"
mkdir -p /tmp/h3claude

MSG="${1:-Test notification from macOS!}"
FROM="${2:-test}"

echo "{\"from\":\"$FROM\",\"text\":\"$MSG\",\"event\":\"test\"}" >> "$INBOX"
echo "Sent: [$FROM] $MSG"
