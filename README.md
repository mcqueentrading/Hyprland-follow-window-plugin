# Hyprland Follow Window Plugin

A Hyprland plugin that lets marked windows follow workspace changes.

This folder contains the current shareable source snapshot and prebuilt binary for the dispatcher-override version.

## Current Behavior

- plugin loads cleanly
- custom follow-window dispatchers work
- marked windows follow workspace changes
- no background polling thread
- no `hyprctl` shell calls
- no late `workspace.active` callback path

The current implementation overrides Hyprland's `workspace` dispatcher early in the pipeline, moves marked windows first, then calls the original dispatcher.

## Files

- `src/main.cpp`
- `src/globals.hpp`
- `Makefile`
- `build.sh`
- `hyprland-follow-window.so`
- `demo.gif`
- `demo.mp4`

## Demo

![Follow Window Demo](./demo.gif)

[Watch the MP4 demo](./demo.mp4)

## What The Plugin Adds

Custom dispatchers:

- `plugin:follow:markfollowwindow`
- `plugin:follow:clearfollowwindow`
- `plugin:follow:clearallfollowwindows`

Optional config:

- `plugin:follow:mute_notifications = 1`

Example config:

```ini
plugin = /home/unknown/.config/hypr/plugins/hyprland-follow-window.so
plugin:follow:mute_notifications = 0

bind = $mainMod, G, plugin:follow:markfollowwindow
bind = $mainMod SHIFT, G, plugin:follow:clearfollowwindow
bind = $mainMod CTRL SHIFT, G, plugin:follow:clearallfollowwindows
```

To mute plugin notifications:

```ini
plugin:follow:mute_notifications = 1
```

## Build

Build against a Hyprland source tree:

```bash
chmod +x build.sh
HYPRLAND_SRC="/path/to/Hyprland" ./build.sh
```

This produces:

- `hyprland-follow-window.so`

## Install

```bash
mkdir -p ~/.config/hypr/plugins
cp hyprland-follow-window.so ~/.config/hypr/plugins/hyprland-follow-window.so
```

Then load it from `hyprland.conf` and restart Hyprland.

## Prebuilt Binary

This folder also includes:

- `hyprland-follow-window.so`

You can copy that directly if you are running the same Hyprland build this was tested on:

- Hyprland `v0.54.3`
- commit `521ece463c4a9d3d128670688a34756805a4328f`

Otherwise, rebuild the plugin against your own matching Hyprland source tree.

## Implementation Notes

The current version:

- stores marked windows as `PHLWINDOWREF`
- uses direct `PHLWORKSPACE` and compositor APIs
- moves windows through `g_pCompositor->moveWindowToWorkspaceSafe(...)`
- overrides `g_pKeybindManager->m_dispatchers["workspace"]`
- restores the original workspace dispatcher on unload

## Status

This is the current exported version.

Known profile:

- works for normal keybind-driven workspace changes using the `workspace` dispatcher
- designed specifically to move earlier than the fullscreen stack
- still different from the older direct Hyprland core patch, which remains the strictest implementation
