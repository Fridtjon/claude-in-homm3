#pragma once

#include "mingw_compat.h"
#include "patcher_x86.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdio>
#include <cstdarg>
#include <ctime>

// Plugin name as seen by HD Mod's patcher system
constexpr const char* PLUGIN_NAME = "H3.ClaudeExpo";

// IPC file path (as seen from inside Wine, Z:\ maps to macOS /)
constexpr const char* INBOX_PATH  = "Z:\\tmp\\h3claude\\inbox.jsonl";
constexpr const char* OUTBOX_PATH = "Z:\\tmp\\h3claude\\outbox.jsonl";
constexpr const char* LOG_PATH    = "Z:\\tmp\\h3claude\\plugin.log";

// How often to poll for new messages (in game loop ticks)
constexpr int POLL_INTERVAL = 30; // roughly once per second at ~30fps

//=============================================================================
// HOMM3 SoD 3.2 addresses (reverse-engineered via H3API)
//=============================================================================

// ScreenChat::Show — displays a message in the in-game chat
// Calling convention: CDECL — Show(this, format, ...)
constexpr _ptr_ ADDR_SCREEN_CHAT_SHOW = 0x553C40;

// Pointer to the H3ScreenChat singleton
constexpr _ptr_ ADDR_SCREEN_CHAT_PTR  = 0x405F27 + 1;

// Global text buffer used by H3 for sprintf
constexpr _ptr_ ADDR_H3_TEXT_BUFFER   = 0x697428;

// Adventure manager global pointer
constexpr _ptr_ ADDR_ADVENTURE_MGR    = 0x6992B8;

// Adventure map mouse-move hook point (runs frequently on adventure map)
constexpr _ptr_ ADDR_MAP_HINT_HOOK    = 0x40D0DB;

// Log to /tmp/h3claude/plugin.log on macOS host
void Log(const char* fmt, ...);

// Initialize the plugin
void PluginInit();
