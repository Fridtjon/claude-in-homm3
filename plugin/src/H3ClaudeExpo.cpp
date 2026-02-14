#include "H3ClaudeExpo.hpp"

static Patcher*         gpPatcher  = nullptr;
static PatcherInstance*  gpInstance = nullptr;
static bool             gStartupMessageShown = false;

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

// Get the ScreenChat singleton pointer
static void* GetScreenChat()
{
    // The address stores a pointer to a dword that contains the address
    // of another dword with the actual ScreenChat pointer
    // _H3API_GET_INFO_(0x405F27 + 1, H3ScreenChat) means:
    // StructAt dereferences: **(ptr at 0x405F28)
    void* chat = *reinterpret_cast<void**>(ADDR_SCREEN_CHAT_PTR);
    return chat;
}

// Display a message in the in-game chat
static void ChatShow(const char* text)
{
    void* chat = GetScreenChat();
    if (!chat)
    {
        Log("ChatShow: ScreenChat pointer is null");
        return;
    }

    // CDECL_3(VOID, 0x553C40, this, "%s", text)
    typedef void (__cdecl *ShowFunc)(void* thisPtr, const char* fmt, const char* text);
    ShowFunc showChat = reinterpret_cast<ShowFunc>(ADDR_SCREEN_CHAT_SHOW);
    showChat(chat, "%s", text);
}

//=============================================================================
// Adventure map hook — fires on mouse movement over the map
// We use this as our "game loop" tick to show a one-time startup message
// and later to poll for new IPC messages
//=============================================================================
_LHF_(OnAdventureMapUpdate)
{
    if (!gStartupMessageShown)
    {
        gStartupMessageShown = true;
        Log("Adventure map active — showing startup chat message");
        ChatShow(">> H3.ClaudeExpo active - Claude Code notifications enabled");
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

    // Hook the adventure map hint update — fires when mouse moves on the map
    // This gives us a periodic callback while on the adventure screen
    gpInstance->WriteLoHook(ADDR_MAP_HINT_HOOK, OnAdventureMapUpdate);
    Log("Installed adventure map hook at 0x%X", ADDR_MAP_HINT_HOOK);
}

// Global instance — constructor runs when DLL is loaded by HD Mod
struct PluginLoader
{
    PluginLoader() { PluginInit(); }
} gPluginLoader;
