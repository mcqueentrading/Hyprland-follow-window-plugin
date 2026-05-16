# Hyprland Follow Window for Hyprland 0.55

Follow marked windows across workspace changes in Hyprland `0.55.x`, with Lua-friendly integration.

![Demo](demo.gif)

## What this does

This project lets you:

- mark the focused window as a follow-window
- switch workspaces normally
- move the marked window to the target workspace before the workspace switch completes
- clear the focused marked window
- clear all marked windows

This is aimed at users running:

- Hyprland `0.55.x`
- `hyprland.lua`

## Why this exists

Pure Lua was enough for:

- binds
- notifications
- config logic
- direct plugin Lua function calls

Pure Lua was **not** enough for the part that actually matters here:

- intercepting the `workspace` dispatcher early
- moving windows at the right point in the switch sequence
- using Hyprland internals like `moveWindowToWorkspaceSafe(...)`

So this repo uses a small plugin backend and exposes a Lua-facing API:

- `hl.plugin.follow.mark()`
- `hl.plugin.follow.clear()`
- `hl.plugin.follow.clear_all()`
- `hl.plugin.follow.workspace(i)`

## Repo contents

- `src/main.cpp`
- `src/globals.hpp`
- `Makefile`
- `build.sh`
- `hyprland.lua-snippet`
- `hyprland-follow-window-v055-lua.so`
- `demo.gif`

## Prebuilt binary

This repo includes a prebuilt plugin binary:

- `hyprland-follow-window-v055-lua.so`

Important:

- it is intended for Hyprland `0.55.x`
- plugins are not ABI-stable across Hyprland versions
- if your build does not match, rebuild it from source

## Build from source

Build against a Hyprland source tree that matches the exact Hyprland version you are running.

Example:

```bash
chmod +x build.sh
HYPRLAND_SRC="/path/to/Hyprland-0.55.0" ./build.sh
```

This produces:

- `hyprland-follow-window.so`

## Install

Create the plugin directory if needed:

```bash
mkdir -p ~/.config/hypr/plugins
```

If you want to use the prebuilt binary:

```bash
cp hyprland-follow-window-v055-lua.so ~/.config/hypr/plugins/hyprland-follow-window-v055-lua.so
```

If you built from source:

```bash
cp hyprland-follow-window.so ~/.config/hypr/plugins/hyprland-follow-window-v055-lua.so
```

The install target name is kept consistent so the Lua snippet can load one stable filename.

## Lua integration

This repo ships a minimal snippet:

- `hyprland.lua-snippet`

Paste the relevant parts into your:

- `~/.config/hypr/hyprland.lua`

Minimal example:

```lua
local mainMod = "SUPER"

hl.plugin.load(os.getenv("HOME") .. "/.config/hypr/plugins/hyprland-follow-window-v055-lua.so")

hl.bind(mainMod .. " + G", function()
    if hl.plugin and hl.plugin.follow and hl.plugin.follow.mark then
        hl.plugin.follow.mark()
    end
end)

hl.bind(mainMod .. " + SHIFT + G", function()
    if hl.plugin and hl.plugin.follow and hl.plugin.follow.clear then
        hl.plugin.follow.clear()
    end
end)

hl.bind(mainMod .. " + CTRL + SHIFT + G", function()
    if hl.plugin and hl.plugin.follow and hl.plugin.follow.clear_all then
        hl.plugin.follow.clear_all()
    end
end)

for i = 1, 10 do
    local key = i % 10
    hl.bind(mainMod .. " + " .. key, function()
        if hl.plugin and hl.plugin.follow and hl.plugin.follow.workspace then
            hl.plugin.follow.workspace(i)
        else
            hl.dispatch(hl.dsp.focus({ workspace = i }))
        end
    end)
    hl.bind(mainMod .. " + SHIFT + " .. key, hl.dsp.window.move({ workspace = i }))
end
```

## Default behavior

- `SUPER + G`
  - mark focused window
- `SUPER + SHIFT + G`
  - clear focused marked window
- `SUPER + CTRL + SHIFT + G`
  - clear all marked windows
- `SUPER + 1..0`
  - if a window is marked, move it first and then switch workspace
  - if nothing is marked, behave like a normal workspace switch

## Notes

- the plugin also registers legacy dispatchers internally:
  - `plugin:follow:markfollowwindow`
  - `plugin:follow:clearfollowwindow`
  - `plugin:follow:clearallfollowwindows`
- for Hyprland `0.55` Lua configs, calling the exported Lua functions directly is the safer path

## Author

- Toni McQueen
