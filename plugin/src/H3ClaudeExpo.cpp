#include "H3ClaudeExpo.hpp"

static Patcher*         gpPatcher  = nullptr;
static PatcherInstance*  gpInstance = nullptr;

// Plugin initialization — connects to patcher_x86 and registers with HD Mod
void PluginInit()
{
    gpPatcher = GetPatcher();
    if (!gpPatcher)
        return;

    if (gpPatcher->GetInstance(PLUGIN_NAME))
        return; // already loaded

    gpInstance = gpPatcher->CreateInstance(PLUGIN_NAME);
    if (!gpInstance)
        return;

    // Milestone 1: plugin loads and registers successfully.
    // HD Mod's patcher log will show "H3.ClaudeExpo" as a loaded instance.
    // A MessageBox confirms it visually during testing.
    MessageBoxA(nullptr,
        "H3.ClaudeExpo loaded!\n\n"
        "Claude Code notifications will appear in chat.\n"
        "This message will be removed after testing.",
        PLUGIN_NAME,
        MB_OK | MB_ICONINFORMATION);
}

// Global instance — constructor runs when DLL is loaded by HD Mod
struct PluginLoader
{
    PluginLoader() { PluginInit(); }
} gPluginLoader;
