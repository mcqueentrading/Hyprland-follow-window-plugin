#pragma once

#include <src/plugins/PluginAPI.hpp>
#include <src/plugins/HookSystem.hpp>
#include <src/managers/KeybindManager.hpp>

#include <hyprutils/signal/Listener.hpp>

#include <functional>
#include <vector>

inline HANDLE PHANDLE = nullptr;
inline std::vector<PHLWINDOWREF> g_markedWindows;
inline std::function<SDispatchResult(std::string)> g_originalWorkspaceDispatcher;
