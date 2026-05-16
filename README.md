# hyprland-follow-window for Hyprland 0.55 (Lua integration)

This is the Git-exportable `0.55/lua` package for the working follow-window implementation.

It uses a small Hyprland plugin for the compositor-internal hook that Lua alone does not expose:

- overrides the `workspace` dispatcher early
- moves marked windows with `g_pCompositor->moveWindowToWorkspaceSafe(...)`
- exports Lua functions under `hl.plugin.follow.*`

## What this package contains

- `src/main.cpp`
- `src/globals.hpp`
- `Makefile`
- `build.sh`
- `examples/hyprland.lua.snippet.lua`

It intentionally does **not** include:

- local absolute machine paths
- local binaries from your working directory
- built `.so` artifacts committed for one specific machine
- your personal Hyprland config

## Lua API exported by the plugin

After loading the plugin in `hyprland.lua`, these functions are available:

- `hl.plugin.follow.mark()`
- `hl.plugin.follow.clear()`
- `hl.plugin.follow.clear_all()`
- `hl.plugin.follow.workspace(i)`

## Build

Build against a Hyprland source tree that matches the exact installed Hyprland version.

For the version this package was built against:

- Hyprland `0.55.0`

Example:

```bash
chmod +x build.sh
HYPRLAND_SRC="/path/to/Hyprland-0.55.0" ./build.sh
```

This produces:

- `hyprland-follow-window.so`

## Install

```bash
mkdir -p ~/.config/hypr/plugins
cp hyprland-follow-window.so ~/.config/hypr/plugins/hyprland-follow-window.so
```

Then load it from `hyprland.lua`.

## Minimal Lua integration

See:

- `examples/hyprland.lua.snippet.lua`

The important detail is that normal workspace binds need to call:

```lua
hl.plugin.follow.workspace(i)
```

when follow-window behavior is desired.

## Why this is not pure Lua

Lua 0.55 was enough for:

- binds
- notifications
- config logic
- direct plugin Lua function calls

It was **not** enough for the critical behavior by itself:

- early `workspace` dispatcher interception
- native workspace move timing before the switch
- `moveWindowToWorkspaceSafe(...)`

That is why this export keeps a small plugin backend and a Lua-facing API instead of pretending the feature is fully implementable in pure Lua.
