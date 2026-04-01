# Hyprland Follow Window

A Hyprland plugin that lets marked windows follow workspace changes.

This version uses Hyprland main-thread workspace events rather than a background polling thread.

## Demo

<p align="center">
  <img src="./demo.gif" alt="Follow Window demo" />
</p>

## Features

- Mark the focused window to follow workspace changes
- Clear the focused marked window
- Clear all marked windows
- Optional muted mode for plugin notifications
- Built as a plugin, not a direct Hyprland core patch

## Dispatchers

- `plugin:follow:markfollowwindow`
- `plugin:follow:clearfollowwindow`
- `plugin:follow:clearallfollowwindows`

## Example Config

```ini
plugin = /home/unknown/.config/hypr/plugins/hyprland-follow-window.so
plugin:follow:mute_notifications = 0

bind = $mainMod, G, plugin:follow:markfollowwindow
bind = $mainMod SHIFT, G, plugin:follow:clearfollowwindow
bind = $mainMod CTRL SHIFT, G, plugin:follow:clearallfollowwindows
```

## Mute Notifications

```ini
plugin:follow:mute_notifications = 1
```

## Build

Build against a Hyprland source tree that matches your installed Hyprland version.

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

Then restart Hyprland after adding the plugin line and binds to `hyprland.conf`.

## Compatibility

Current known working build target:

- Hyprland `v0.54.3`
- commit `521ece463c4a9d3d128670688a34756805a4328f`

Because Hyprland plugin internals are not ABI-stable, rebuild the plugin against the exact Hyprland version you are running whenever possible.

## Implementation Notes

The working version uses:

- `Event::bus()->m_events.workspace.active.listen(...)`
- `g_pEventLoopManager->doLater(...)`

That keeps workspace-follow behavior on Hyprland’s main thread.

## Status

This is the current working plugin branch preserved from local testing. Earlier versions used a separate thread and were unstable.
