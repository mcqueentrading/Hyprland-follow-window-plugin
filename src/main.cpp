#include "globals.hpp"

#include <src/Compositor.hpp>
#include <src/desktop/history/WorkspaceHistoryTracker.hpp>
#include <src/desktop/Workspace.hpp>
#include <src/desktop/state/FocusState.hpp>
#include <src/helpers/Monitor.hpp>
#include <src/helpers/MiscFunctions.hpp>
#include <src/managers/KeybindManager.hpp>
#include <src/plugins/PluginAPI.hpp>

#include <algorithm>
#include <format>
#include <string>

static bool notificationsMuted() {
    const auto* value = HyprlandAPI::getConfigValue(PHANDLE, "plugin:follow:mute_notifications");
    if (!value)
        return false;

    try {
        return std::any_cast<Hyprlang::INT>(value->getValue()) != 0;
    } catch (...) {
        return false;
    }
}

static void notify(const std::string& text, const CHyprColor& color, float timeMs) {
    if (notificationsMuted())
        return;

    HyprlandAPI::addNotification(PHANDLE, text, color, timeMs);
}

static PHLWINDOW focusedWindow() {
    return Desktop::focusState()->window();
}

static void compactMarkedWindows() {
    std::erase_if(g_markedWindows, [](const auto& ref) { return ref.expired(); });
}

static bool isWindowMarked(const PHLWINDOW& window) {
    if (!window)
        return false;

    compactMarkedWindows();

    return std::ranges::any_of(g_markedWindows, [&](const auto& ref) {
        return ref.lock() == window;
    });
}

static size_t moveMarkedFollowWindowsToWorkspace(const PHLWORKSPACE& workspace) {
    if (!workspace || workspace->m_isSpecialWorkspace)
        return 0;

    compactMarkedWindows();

    size_t moved = 0;
    for (const auto& ref : g_markedWindows) {
        const auto window = ref.lock();
        if (!window || window->m_workspace == workspace)
            continue;

        g_pCompositor->moveWindowToWorkspaceSafe(window, workspace);
        ++moved;
    }

    return moved;
}

static SWorkspaceIDName getWorkspaceToChangeFromArgs(std::string args, const PHLWORKSPACE& currentWorkspace, const PHLMONITORREF& monitor) {
    if (!args.starts_with("previous"))
        return getWorkspaceIDNameFromString(args);

    const bool             perMonitor = args.contains("_per_monitor");
    const SWorkspaceIDName previous   = perMonitor ? Desktop::History::workspaceTracker()->previousWorkspaceIDName(currentWorkspace, monitor.lock()) :
                                                     Desktop::History::workspaceTracker()->previousWorkspaceIDName(currentWorkspace);

    if (previous.id == -1 || previous.id == currentWorkspace->m_id)
        return {.id = WORKSPACE_NOT_CHANGED};

    if (const auto targetWorkspace = g_pCompositor->getWorkspaceByID(previous.id); targetWorkspace)
        return {.id = targetWorkspace->m_id, .name = targetWorkspace->m_name};

    return {.id = previous.id, .name = previous.name.empty() ? std::to_string(previous.id) : previous.name};
}

static PHLWORKSPACE resolveTargetWorkspaceForChange(const std::string& args) {
    const auto monitor = Desktop::focusState()->monitor();
    if (!monitor)
        return nullptr;

    const auto currentWorkspace = monitor->m_activeWorkspace;
    if (!currentWorkspace)
        return nullptr;

    const bool explicitPrevious = args.contains("previous");
    const auto [workspaceToChangeTo, workspaceName, isAutoID] = getWorkspaceToChangeFromArgs(args, currentWorkspace, monitor);

    if (workspaceToChangeTo == WORKSPACE_INVALID || workspaceToChangeTo == WORKSPACE_NOT_CHANGED)
        return nullptr;

    const SWorkspaceIDName previousWorkspace = args.contains("_per_monitor") ? Desktop::History::workspaceTracker()->previousWorkspaceIDName(currentWorkspace, monitor) :
                                                                             Desktop::History::workspaceTracker()->previousWorkspaceIDName(currentWorkspace);

    const bool workspaceIsCurrent = workspaceToChangeTo == currentWorkspace->m_id;
    if (workspaceIsCurrent && (!explicitPrevious || previousWorkspace.id == -1))
        return nullptr;

    auto targetWorkspace = g_pCompositor->getWorkspaceByID(workspaceIsCurrent ? previousWorkspace.id : workspaceToChangeTo);
    if (!targetWorkspace) {
        targetWorkspace = g_pCompositor->createNewWorkspace(workspaceIsCurrent ? previousWorkspace.id : workspaceToChangeTo, monitor->m_id,
            workspaceIsCurrent ? previousWorkspace.name : workspaceName);
    }

    return targetWorkspace;
}

static SDispatchResult followAwareWorkspace(std::string args) {
    if (const auto targetWorkspace = resolveTargetWorkspaceForChange(args); targetWorkspace && !targetWorkspace->m_isSpecialWorkspace)
        moveMarkedFollowWindowsToWorkspace(targetWorkspace);

    if (!g_originalWorkspaceDispatcher)
        return {.success = false, .error = "Original workspace dispatcher missing"};

    return g_originalWorkspaceDispatcher(std::move(args));
}

static bool installWorkspaceDispatcherOverride() {
    const auto it = g_pKeybindManager->m_dispatchers.find("workspace");
    if (it == g_pKeybindManager->m_dispatchers.end()) {
        notify("workspace dispatcher not found", CHyprColor{0.95, 0.45, 0.2, 1.0}, 3500);
        return false;
    }

    g_originalWorkspaceDispatcher = it->second;
    if (!g_originalWorkspaceDispatcher) {
        notify("original workspace dispatcher missing", CHyprColor{0.95, 0.45, 0.2, 1.0}, 3500);
        return false;
    }

    g_pKeybindManager->m_dispatchers["workspace"] = followAwareWorkspace;
    notify("workspace dispatcher override installed", CHyprColor{0.3, 0.9, 0.55, 1.0}, 2500);
    return true;
}

static SDispatchResult markFollowWindow(std::string) {
    const auto window = focusedWindow();
    if (!window)
        return {.success = false, .error = "Could not resolve active window"};

    if (isWindowMarked(window))
        return {.success = false, .error = "Window is already marked"};

    g_markedWindows.emplace_back(window);
    notify(std::format("Follow window marked {:p}", static_cast<const void*>(window.get())), CHyprColor{0.15, 0.7, 0.95, 1.0}, 2500);
    return {};
}

static SDispatchResult clearFollowWindow(std::string) {
    const auto window = focusedWindow();
    if (!window) {
        g_markedWindows.clear();
        notify("All follow windows cleared", CHyprColor{0.95, 0.55, 0.15, 1.0}, 2500);
        return {};
    }

    compactMarkedWindows();

    const auto oldSize = g_markedWindows.size();
    std::erase_if(g_markedWindows, [&](const auto& ref) {
        return ref.lock() == window;
    });

    if (g_markedWindows.size() == oldSize)
        return {.success = false, .error = "Focused window was not marked"};

    notify(std::format("Follow window cleared {:p}", static_cast<const void*>(window.get())), CHyprColor{0.95, 0.55, 0.15, 1.0}, 2500);
    return {};
}

static SDispatchResult clearAllFollowWindows(std::string) {
    g_markedWindows.clear();
    notify("All follow windows cleared", CHyprColor{0.95, 0.2, 0.3, 1.0}, 2500);
    return {};
}

APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    PHANDLE = handle;

    HyprlandAPI::addConfigValue(PHANDLE, "plugin:follow:mute_notifications", Hyprlang::CConfigValue(Hyprlang::INT{0}));
    HyprlandAPI::addDispatcherV2(PHANDLE, "plugin:follow:markfollowwindow", ::markFollowWindow);
    HyprlandAPI::addDispatcherV2(PHANDLE, "plugin:follow:clearfollowwindow", ::clearFollowWindow);
    HyprlandAPI::addDispatcherV2(PHANDLE, "plugin:follow:clearallfollowwindows", ::clearAllFollowWindows);

    const bool hookOk = installWorkspaceDispatcherOverride();
    notify(hookOk ? "Follow-window plugin loaded with workspace dispatcher override" : "Follow-window plugin loaded, but workspace override failed",
        hookOk ? CHyprColor{0.3, 0.9, 0.55, 1.0} : CHyprColor{0.95, 0.45, 0.2, 1.0}, 5000);

    return {"hyprland-follow-window", "Follow marked windows across workspace changes", "George McQueen", "0.1.0"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    if (g_originalWorkspaceDispatcher)
        g_pKeybindManager->m_dispatchers["workspace"] = g_originalWorkspaceDispatcher;
    g_originalWorkspaceDispatcher = {};
    g_markedWindows.clear();
}
