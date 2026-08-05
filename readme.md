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
  wled-usermod-my_sensor/
    library.json
    my_sensor.cpp
```

In `platformio_override.ini` inside the WLED folder, add a `symlink://` reference to your local clone:

```ini
[env:esp32dev]
extends = env:esp32dev
custom_usermods =
  ${env:esp32dev.custom_usermods}
  symlink:///home/you/projects/wled-usermod-my_sensor
```

Add both projects to the same VS Code workspace if you want to edit them together. PlatformIO picks up your changes on each build.

### 3. Share it

Tag your working version and add your usermod to the [Community Usermods page](https://kno.wled.ge/advanced/community-usermods/) by sending a PR to [WLED-Docs](https://github.com/wled/WLED-Docs).  Other developers can add your usermod to their builds by adding your repository to their build's `custom_usermods`!

```ini
custom_usermods =
  ${env:esp32dev.custom_usermods}
  https://github.com/you/wled-usermod-my_sensor.git#v1.0.0
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
