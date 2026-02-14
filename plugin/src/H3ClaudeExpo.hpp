#pragma once

#include "mingw_compat.h"
#include "patcher_x86.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// Plugin name as seen by HD Mod's patcher system
constexpr const char* PLUGIN_NAME = "H3.ClaudeExpo";

// IPC file path (as seen from inside Wine, Z:\ maps to macOS /)
constexpr const char* INBOX_PATH  = "Z:\\tmp\\h3claude\\inbox.jsonl";
constexpr const char* OUTBOX_PATH = "Z:\\tmp\\h3claude\\outbox.jsonl";

// How often to poll for new messages (in game loop ticks)
constexpr int POLL_INTERVAL = 30; // roughly once per second at ~30fps

// Initialize the plugin - called from global constructor
void PluginInit();
