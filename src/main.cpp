#include "globals.hpp"

#include <src/desktop/Workspace.hpp>
#include <src/event/EventBus.hpp>
#include <src/managers/eventLoop/EventLoopManager.hpp>
#include <src/plugins/PluginAPI.hpp>

#include <format>
#include <regex>
#include <string>

static std::string firstMatch(const std::string& haystack, const std::regex& pattern, size_t group = 1) {
    std::smatch match;
    if (std::regex_search(haystack, match, pattern) && match.size() > group)
        return match[group].str();
    return "";
}

static std::string activeWindowKey() {
    const auto activeWindow = HyprlandAPI::invokeHyprctlCommand("activewindow", "", "j");

    if (activeWindow.empty())
        return "";

    return firstMatch(activeWindow, std::regex(R"re("address"\s*:\s*"([^"]+)")re"));
}

static bool isSpecialWorkspaceTarget(const std::string& target) {
    return target.starts_with("special") || target.starts_with("name:special");
}

static std::string workspaceTargetFromWorkspace(const PHLWORKSPACE& workspace) {
    if (!workspace)
        return "";

    if (workspace->m_id > 0)
        return std::to_string(workspace->m_id);

    if (workspace->m_name.empty())
        return "";

    return std::format("name:{}", workspace->m_name);
}

static size_t moveMarkedFollowWindowsToWorkspaceTarget(const std::string& workspaceTarget) {
    if (workspaceTarget.empty() || isSpecialWorkspaceTarget(workspaceTarget))
        return 0;

    size_t moved = 0;
    for (const auto& key : g_markedWindowKeys) {
        if (key.empty())
            continue;

        const auto args = std::format("movetoworkspacesilent {},address:{}", workspaceTarget, key);
        HyprlandAPI::invokeHyprctlCommand("dispatch", args, "");
        ++moved;
    }

    return moved;
}

static SDispatchResult markFollowWindow(std::string) {
    const auto key = activeWindowKey();
    if (key.empty())
        return {.success = false, .error = "Could not resolve active window"};

    if (g_markedWindowKeys.contains(key))
        return {.success = false, .error = "Window is already marked"};

    g_markedWindowKeys.insert(key);
    HyprlandAPI::addNotification(PHANDLE, std::format("Follow window marked {}", key), CHyprColor{0.15, 0.7, 0.95, 1.0}, 2500);
    return {};
}

static SDispatchResult clearFollowWindow(std::string) {
    const auto key = activeWindowKey();
    if (key.empty()) {
        g_markedWindowKeys.clear();
        HyprlandAPI::addNotification(PHANDLE, "All follow windows cleared", CHyprColor{0.95, 0.55, 0.15, 1.0}, 2500);
        return {};
    }

    if (!g_markedWindowKeys.erase(key))
        return {.success = false, .error = "Focused window was not marked"};

    HyprlandAPI::addNotification(PHANDLE, std::format("Follow window cleared {}", key), CHyprColor{0.95, 0.55, 0.15, 1.0}, 2500);
    return {};
}

static SDispatchResult clearAllFollowWindows(std::string) {
    g_markedWindowKeys.clear();
    HyprlandAPI::addNotification(PHANDLE, "All follow windows cleared", CHyprColor{0.95, 0.2, 0.3, 1.0}, 2500);
    return {};
}

APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    PHANDLE = handle;

    HyprlandAPI::addDispatcherV2(PHANDLE, "plugin:follow:markfollowwindow", ::markFollowWindow);
    HyprlandAPI::addDispatcherV2(PHANDLE, "plugin:follow:clearfollowwindow", ::clearFollowWindow);
    HyprlandAPI::addDispatcherV2(PHANDLE, "plugin:follow:clearallfollowwindows", ::clearAllFollowWindows);

    g_workspaceActiveListener = Event::bus()->m_events.workspace.active.listen([](PHLWORKSPACE workspace) {
        const auto target = workspaceTargetFromWorkspace(workspace);
        if (target.empty() || isSpecialWorkspaceTarget(target))
            return;

        HyprlandAPI::addNotification(
            PHANDLE,
            std::format("workspace.active -> {}", target),
            CHyprColor{0.25, 0.75, 0.95, 1.0},
            1200
        );

        g_pEventLoopManager->doLater([target] {
            const auto moved = moveMarkedFollowWindowsToWorkspaceTarget(target);
            HyprlandAPI::addNotification(
                PHANDLE,
                std::format("Follow moved {} window(s) to {}", moved, target),
                moved > 0 ? CHyprColor{0.2, 0.7, 0.95, 1.0} : CHyprColor{0.95, 0.6, 0.2, 1.0},
                1600
            );
        });
    });

    HyprlandAPI::addNotification(
        PHANDLE,
        "Follow-window plugin loaded with workspace.active listener",
        CHyprColor{0.3, 0.9, 0.55, 1.0},
        5000
    );

    return {"hyprland-follow-window", "Follow marked windows across workspace changes", "George McQueen", "0.1.0"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    g_workspaceActiveListener.reset();
    g_markedWindowKeys.clear();
}
