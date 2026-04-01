#pragma once

#include <src/plugins/PluginAPI.hpp>
#include <src/plugins/HookSystem.hpp>

#include <hyprutils/signal/Listener.hpp>

#include <string>
#include <unordered_set>

inline HANDLE PHANDLE = nullptr;
inline std::unordered_set<std::string> g_markedWindowKeys;
inline Hyprutils::Signal::CHyprSignalListener g_workspaceActiveListener;
