# WLED usermod example

This repository is a [GitHub template](https://docs.github.com/en/repositories/creating-and-managing-repositories/creating-a-repository-from-a-template) for building your own [WLED](https://github.com/wled/WLED) usermod as a standalone project. Create a new repository from it, add your code, link it to WLED, and make the world a brighter place!

## Getting started

### 1. Create from template

Click **Use this template** → **Create a new repository** on GitHub. You get a clean copy to start building your project from. Then:

- Rename `usermod_example.cpp` to something descriptive (e.g. `my_sensor.cpp`)
- Rename the class inside from `MyExampleUsermod` to match
- Update `"name"` in `library.json` to match your repository name

### 2. Wire it into your WLED build

Clone your new repository alongside your WLED checkout:

```
~/projects/
  WLED/
  my-wled-usermod/
    library.json
    my_sensor.cpp
```

In `platformio_override.ini` inside the WLED folder, add a `symlink://` reference to your local clone:

```ini
[env:esp32dev]
extends = env:esp32dev
custom_usermods =
  ${env:esp32dev.custom_usermods}
  symlink:///home/you/projects/my-wled-usermod
```

Both projects are now open in the same VS Code session. PlatformIO picks up your changes on each build.

### 3. Share it

Once ready, others can reference your usermod directly by URL — no local clone needed:

```ini
custom_usermods =
  ${env:esp32dev.custom_usermods}
  symlink://github.com/you/my-wled-usermod.git#main
```

## What's in this repo

**`library.json`** — PlatformIO library manifest. The `"libArchive": false` setting is required; without it the build will fail. Add any library dependencies here.

**`usermod_example.cpp`** — A fully annotated example covering all available lifecycle hooks:

| Method | When called |
|---|---|
| `setup()` | Once at boot, after config is loaded, before WiFi |
| `connected()` | Each time WiFi (re)connects |
| `loop()` | Every main loop iteration |
| `addToJsonInfo()` | When `/json/info` is requested |
| `addToJsonState()` / `readFromJsonState()` | On `/json/state` get/post |
| `addToConfig()` / `readFromConfig()` | Persistent settings in `cfg.json` |
| `appendConfigData()` | When the Usermod Settings page renders |
| `handleOverlayDraw()` | Just before each LED strip update |
| `handleButton()` | On button events |
| `onMqttMessage()` / `onMqttConnect()` | MQTT events |
| `onStateChange()` | When WLED state changes |

`REGISTER_USERMOD(instance)` at the bottom of the file handles self-registration — there is no `usermods_list.cpp` to edit.

For full documentation see the [WLED Custom Features](https://kno.wled.ge/advanced/custom-features/) page.
