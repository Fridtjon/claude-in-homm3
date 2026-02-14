#include "H3ClaudeExpo.hpp"
#include <cstring>

static Patcher*         gpPatcher  = nullptr;
static PatcherInstance*  gpInstance = nullptr;
static bool             gStartupMessageShown = false;
static long             gInboxOffset = 0; // file read position
static UINT_PTR         gTimerID = 0;
static HWND             gGameWindow = nullptr;
static WNDPROC          gOrigWndProc = nullptr;

// Timer interval in milliseconds (poll inbox every second)
constexpr UINT TIMER_INTERVAL_MS = 1000;
constexpr UINT_PTR TIMER_ID = 0xC1A0DE; // unique timer ID

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
// IPC inbox reader
//=============================================================================

static bool JsonGetString(const char* json, const char* key, char* out, int outSize)
{
    char pattern[128];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char* pos = strstr(json, pattern);
    if (!pos) return false;

    pos += strlen(pattern);
    while (*pos == ' ' || *pos == ':') pos++;
    if (*pos != '"') return false;
    pos++;

    int i = 0;
    while (*pos && *pos != '"' && i < outSize - 1)
    {
        if (*pos == '\\' && *(pos + 1)) { pos++; }
        out[i++] = *pos++;
    }
    out[i] = '\0';
    return i > 0;
}

static void PollInbox()
{
    FILE* f = fopen(INBOX_PATH, "r");
    if (!f) return;

    fseek(f, gInboxOffset, SEEK_SET);

    char line[512];

    while (fgets(line, sizeof(line), f))
    {
        int len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
            line[--len] = '\0';

        if (len == 0) continue;

        char chatMsg[256];

        if (line[0] == '{')
        {
            char from[64] = "claude";
            char text[256] = "";
            char timestr[16] = "";
            JsonGetString(line, "from", from, sizeof(from));
            JsonGetString(line, "text", text, sizeof(text));
            JsonGetString(line, "time", timestr, sizeof(timestr));

            if (text[0])
            {
                if (timestr[0])
                    snprintf(chatMsg, sizeof(chatMsg), "[%s %s] %s", timestr, from, text);
                else
                    snprintf(chatMsg, sizeof(chatMsg), "[%s] %s", from, text);
                ChatShow(chatMsg);
                Log("Inbox: %s", chatMsg);
            }
        }
        else
        {
            ChatShow(line);
            Log("Inbox: %s", line);
        }
    }

    gInboxOffset = ftell(f);
    fclose(f);
}

//=============================================================================
// Windows timer — fires every second regardless of mouse/screen state
//=============================================================================

static LRESULT CALLBACK SubclassWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_TIMER && wParam == TIMER_ID)
    {
        PollInbox();
        return 0;
    }
    return CallWindowProcA(gOrigWndProc, hwnd, msg, wParam, lParam);
}

static void StartTimer()
{
    // Find the game's main window
    gGameWindow = FindWindowA("Heroes3", nullptr);
    if (!gGameWindow)
        gGameWindow = FindWindowA(nullptr, "Heroes of Might and Magic III");
    if (!gGameWindow)
    {
        // Fallback: enumerate top-level windows to find HOMM3
        gGameWindow = GetForegroundWindow();
    }

    if (gGameWindow)
    {
        // Subclass the window to receive WM_TIMER
        gOrigWndProc = reinterpret_cast<WNDPROC>(
            SetWindowLongA(gGameWindow, GWL_WNDPROC, reinterpret_cast<LONG>(SubclassWndProc)));
        gTimerID = SetTimer(gGameWindow, TIMER_ID, TIMER_INTERVAL_MS, nullptr);
        Log("Timer started (ID=%u, interval=%ums, hwnd=%p)", TIMER_ID, TIMER_INTERVAL_MS, gGameWindow);
    }
    else
    {
        Log("WARNING: Could not find game window for timer");
    }
}

//=============================================================================
// Adventure map hook — fires on first mouse movement to show startup message
// and start the timer
//=============================================================================

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

        // Start the polling timer now that the game is ready
        StartTimer();
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

    // Hook the adventure map — used only for one-time startup + timer init
    gpInstance->WriteLoHook(ADDR_MAP_HINT_HOOK, OnAdventureMapUpdate);
    Log("Installed adventure map hook at 0x%X", ADDR_MAP_HINT_HOOK);
}

// Global instance — constructor runs when DLL is loaded by HD Mod
struct PluginLoader
{
    PluginLoader() { PluginInit(); }
} gPluginLoader;
