# Hyprland Follow Window Plugin Repro

This is a small Hyprland plugin repro for a custom follow-window idea.

Current behavior:

- plugin loads
- custom dispatchers work
- marked windows follow workspace changes
- current callback-based version is working cleanly in local testing

The current source in this folder is the main-thread callback version.

## Files

- `src/main.cpp`
- `src/globals.hpp`
- `Makefile`
- `build.sh`
- `hyprland-follow-window.so`

## What The Plugin Adds

Custom dispatchers:

- `plugin:follow:markfollowwindow`
- `plugin:follow:clearfollowwindow`
- `plugin:follow:clearallfollowwindows`

Optional config:

- `plugin:follow:mute_notifications = 1`
  disables plugin notifications

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
cd hyprland-follow-window-discord-share
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

Then load it from `hyprland.conf`.

## Prebuilt Binary

This folder also includes:

- `hyprland-follow-window.so`

You can copy that directly to:

```bash
mkdir -p ~/.config/hypr/plugins
cp hyprland-follow-window.so ~/.config/hypr/plugins/hyprland-follow-window.so
```

if you are running the same Hyprland build this was tested on:

- Hyprland `v0.54.3`
- commit `521ece463c4a9d3d128670688a34756805a4328f`

Otherwise, rebuild the plugin against your own Hyprland source tree instead of reusing the included `.so`.

## Runtime / Version Context

Current working test target:

- Hyprland `v0.54.3`
- commit `521ece463c4a9d3d128670688a34756805a4328f`

The plugin was rebuilt locally against the exact matching Hyprland source checkout for that version.

## Goal

Desired end state:

- mark one or more windows
- when the user changes workspace, those marked windows follow
- plugin implementation, not a direct core Hyprland source patch

## Notes

The current implementation does not use a background polling thread.
It uses:

- `Event::bus()->m_events.workspace.active.listen(...)`
- `g_pEventLoopManager->doLater(...)`

An earlier plugin attempt used a separate thread and was unstable. This folder now contains the cleaner main-thread callback version.
