#include "H3ClaudeExpo.hpp"
#include <cstring>

static Patcher*         gpPatcher  = nullptr;
static PatcherInstance*  gpInstance = nullptr;
static bool             gStartupMessageShown = false;
static long             gInboxOffset = 0; // file read position

// Log to /tmp/h3claude/plugin.log on macOS host via Wine's Z: drive
void Log(const char* fmt, ...)
{
    CreateDirectoryA("Z:\\tmp\\h3claude", nullptr);
    FILE* f = fopen(LOG_PATH, "a");
    if (!f) return;

    time_t now = time(nullptr);
    char timebuf[32];
    strftime(timebuf, sizeof(timebuf), "%H:%M:%S", localtime(&now));
    fprintf(f, "[%s] ", timebuf);

    va_list args;
    va_start(args, fmt);
    vfprintf(f, fmt, args);
    va_end(args);

    fprintf(f, "\n");
    fclose(f);
}

//=============================================================================
// Chat display — call the game's ScreenChat::Show function directly
//=============================================================================

static void* GetScreenChat()
{
    void* chat = *reinterpret_cast<void**>(ADDR_SCREEN_CHAT_PTR);
    return chat;
}

static void ChatShow(const char* text)
{
    void* chat = GetScreenChat();
    if (!chat) return;

    typedef void (__cdecl *ShowFunc)(void* thisPtr, const char* fmt, const char* text);
    ShowFunc showChat = reinterpret_cast<ShowFunc>(ADDR_SCREEN_CHAT_SHOW);
    showChat(chat, "%s", text);
}

//=============================================================================
// IPC inbox reader — reads new lines from /tmp/h3claude/inbox
//
// Message format: one message per line, plain text.
// Lines starting with { are JSON (future use), otherwise displayed as-is.
// The plugin tracks its read position and only processes new lines.
//=============================================================================

// Simple JSON field extractor — finds "key":"value" and returns value
// No dependencies, no allocations. Returns false if key not found.
static bool JsonGetString(const char* json, const char* key, char* out, int outSize)
{
    // Search for "key"
    char pattern[128];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char* pos = strstr(json, pattern);
    if (!pos) return false;

    // Skip past "key" and find the colon + opening quote
    pos += strlen(pattern);
    while (*pos == ' ' || *pos == ':') pos++;
    if (*pos != '"') return false;
    pos++; // skip opening quote

    // Copy value until closing quote
    int i = 0;
    while (*pos && *pos != '"' && i < outSize - 1)
    {
        if (*pos == '\\' && *(pos + 1)) { pos++; } // skip escape
        out[i++] = *pos++;
    }
    out[i] = '\0';
    return i > 0;
}

static void PollInbox()
{
    FILE* f = fopen(INBOX_PATH, "r");
    if (!f) return;

    // Seek to where we left off
    fseek(f, gInboxOffset, SEEK_SET);

    char line[512];
    int messagesRead = 0;

    while (fgets(line, sizeof(line), f))
    {
        // Strip trailing newline
        int len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
            line[--len] = '\0';

        if (len == 0) continue;

        // Format the chat message
        char chatMsg[256];

        if (line[0] == '{')
        {
            // JSON message: extract "from" and "text" fields
            char from[64] = "claude";
            char text[256] = "";
            JsonGetString(line, "from", from, sizeof(from));
            JsonGetString(line, "text", text, sizeof(text));

            if (text[0])
            {
                snprintf(chatMsg, sizeof(chatMsg), "[%s] %s", from, text);
                ChatShow(chatMsg);
                Log("Inbox: %s", chatMsg);
                messagesRead++;
            }
        }
        else
        {
            // Plain text message — display as-is
            ChatShow(line);
            Log("Inbox: %s", line);
            messagesRead++;
        }
    }

    // Save position for next poll
    gInboxOffset = ftell(f);
    fclose(f);
}

//=============================================================================
// Adventure map hook — fires on mouse movement over the map
//=============================================================================
static int gTickCount = 0;

_LHF_(OnAdventureMapUpdate)
{
    if (!gStartupMessageShown)
    {
        gStartupMessageShown = true;
        Log("Adventure map active — showing startup chat message");
        ChatShow(">> H3.ClaudeExpo active - Claude Code notifications enabled");

        // Initialize inbox offset to end of file (skip old messages)
        FILE* f = fopen(INBOX_PATH, "r");
        if (f)
        {
            fseek(f, 0, SEEK_END);
            gInboxOffset = ftell(f);
            fclose(f);
            Log("Inbox initialized at offset %ld", gInboxOffset);
        }
    }

    // Poll inbox periodically
    gTickCount++;
    if (gTickCount >= POLL_INTERVAL)
    {
        gTickCount = 0;
        PollInbox();
    }

    return EXEC_DEFAULT;
}

//=============================================================================
// Plugin initialization
//=============================================================================
void PluginInit()
{
    Log("PluginInit() called");

    gpPatcher = GetPatcher();
    if (!gpPatcher)
    {
        Log("ERROR: GetPatcher() returned null");
        return;
    }
    Log("GetPatcher() OK");

    if (gpPatcher->GetInstance(PLUGIN_NAME))
    {
        Log("WARNING: instance already exists, skipping");
        return;
    }

    gpInstance = gpPatcher->CreateInstance(PLUGIN_NAME);
    if (!gpInstance)
    {
        Log("ERROR: CreateInstance() failed");
        return;
    }
    Log("Plugin registered as '%s'", PLUGIN_NAME);

    // Create IPC directory
    CreateDirectoryA("Z:\\tmp\\h3claude", nullptr);

    // Hook the adventure map hint update
    gpInstance->WriteLoHook(ADDR_MAP_HINT_HOOK, OnAdventureMapUpdate);
    Log("Installed adventure map hook at 0x%X", ADDR_MAP_HINT_HOOK);
}

// Global instance — constructor runs when DLL is loaded by HD Mod
struct PluginLoader
{
    PluginLoader() { PluginInit(); }
} gPluginLoader;
