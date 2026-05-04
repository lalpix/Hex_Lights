# Home Assistant Integration Plan — Hex Lights

## Goal
Add native Home Assistant integration to the ESP32 firmware so the hex lights can be controlled from HA without the custom Flutter app. Control targets: on/off, RGB color, brightness, animations (effects), speed, fade.

---

## How It Works (Overview)

Home Assistant has a built-in **MQTT Light** entity type that supports RGB, brightness, and effects out of the box. The ESP32 connects to a **local Mosquitto MQTT broker** (running as an HA add-on) and publishes **MQTT Discovery** payloads on boot. HA picks those up and auto-creates the entities — no manual HA config needed.

Current firmware already has full MQTT logic (AWS IoT / TLS). We replace the broker target with the local HA one and add a discovery + JSON command layer on top of the existing control functions.

---

## Phase 1 — Home Assistant Setup (manual, one-time)

### 1.1 Install Mosquitto Add-on
- In HA: Settings → Add-ons → Add-on Store → search "Mosquitto broker" → Install → Start
- Enable "Start on boot"

### 1.2 Create MQTT User
- Settings → People → Users → Add User
- Username: `hexlights` / Password: hexlights
- Note the credentials — needed in firmware config

### 1.3 Enable MQTT Integration
- Settings → Devices & Services → Add Integration → MQTT
- Host: `localhost` (or `homeassistant.local`), Port: `1883`, enter credentials above
- HA will now listen for discovery messages

---

## Phase 2 — Firmware Changes

### 2.1 New Config File: `include/ha_config.h`
Replace `keys.h` AWS IoT params with local broker params:
```cpp
const char* mqtt_server   = "192.168.x.x";   // HA RPi IP
const uint16_t mqtt_port  = 1883;             // plain, no TLS
const char* mqtt_user     = "hexlights";
const char* mqtt_pass     = "hexlights";
const char* device_id     = "hex_lights_01"; // unique per device
```
This file goes in `.gitignore` (same as current `keys.h`).

### 2.2 Strip TLS from MQTT Connection
Current code uses `WiFiClientSecure` + root CA cert for AWS IoT.
Switch to plain `WiFiClient` — local broker does not need TLS.
- Remove `WiFiClientSecure` and root CA handling
- Use `WiFiClient` instead
- Connect on port 1883

### 2.3 Add MQTT Discovery Publisher
On every successful MQTT connect, publish discovery configs to `homeassistant/` topics (retained=true). HA reads these once and creates entities permanently.

**Entity: Light** (`homeassistant/light/hex_lights_01/config`)
```json
{
  "name": "Hex Lights",
  "unique_id": "hex_lights_01",
  "schema": "json",
  "command_topic": "hexlights/hex_lights_01/set",
  "state_topic":   "hexlights/hex_lights_01/state",
  "brightness": true,
  "color_mode": true,
  "supported_color_modes": ["rgb"],
  "effect": true,
  "effect_list": [
    "Stationar", "RotationOuter", "StationarOuter",
    "RandColorRandHex", "RandColorRandHexFade",
    "RainbowSwipeVert", "RainbowSwipeHorz",
    "TwoColorFading", "AudioBeatReact", "AudioFreqPool",
    "PresetAnim1", "PresetAnim2", "PresetAnim3", "PresetAnim4"
  ],
  "device": {
    "identifiers": ["hex_lights_01"],
    "name": "Hex Lights",
    "model": "Hexaleaf V3",
    "manufacturer": "DIY"
  }
}
```

**Entity: Speed** (`homeassistant/number/hex_lights_speed/config`)
```json
{
  "name": "Hex Lights Speed",
  "unique_id": "hex_lights_01_speed",
  "command_topic": "hexlights/hex_lights_01/speed/set",
  "state_topic":   "hexlights/hex_lights_01/speed/state",
  "min": 0, "max": 100, "step": 1,
  "device": { "identifiers": ["hex_lights_01"] }
}
```

**Entity: Fade** (`homeassistant/number/hex_lights_fade/config`)
Same pattern, topics `fade/set` and `fade/state`, range 0–255.

### 2.4 Subscribe to New HA Command Topics
Add subscriptions:
- `hexlights/<id>/set`       → main light JSON command
- `hexlights/<id>/speed/set` → speed integer string
- `hexlights/<id>/fade/set`  → fade integer string

### 2.5 JSON Command Parser for Light Entity
HA sends JSON on `hexlights/<id>/set`:
```json
{"state":"ON","brightness":200,"color":{"r":255,"g":0,"b":0},"effect":"RainbowSwipeVert"}
```
Parse with Arduino `ArduinoJson` library and dispatch to existing controller functions:
- `state` → `set_power(on/off)`
- `brightness` → `set_brightness(b)`
- `color.r/g/b` → `set_color(CRGB(r,g,b), 0)`
- `effect` → `change_mode(...)` or `set_pre_anim(n)` for presets

### 2.6 State Publisher
After every state change, publish current state JSON to `hexlights/<id>/state`:
```json
{"state":"ON","brightness":200,"color":{"r":255,"g":0,"b":0},"effect":"RainbowSwipeVert"}
```
Also publish speed/fade state to their respective state topics.
Publish on: boot, reconnect, and every time a command is received.

### 2.7 Keep Existing Topics (backwards compat)
Keep the old topic subscriptions (`mode`, `brightness`, `primaryColor`, etc.) working alongside the new HA topics. This way the old app still works if needed.

---

## Phase 3 — Build & Dependencies

### 3.1 Add ArduinoJson to platformio.ini
```ini
lib_deps =
    ...
    bblanchon/ArduinoJson @ ^6.21.0
```

### 3.2 Remove AWS IoT library dependency
`AWS IoT 0.0.22` can be removed from `lib_deps` once TLS client is gone.

---

## Phase 4 — Testing Checklist

- [ ] ESP32 connects to local Mosquitto broker
- [ ] HA auto-discovers "Hex Lights" device and entities
- [ ] On/Off toggle works from HA
- [ ] RGB color picker in HA sets correct color on LEDs
- [ ] Brightness slider works
- [ ] Effect selector shows all animations and switching works
- [ ] Speed number entity controls animation speed
- [ ] Fade number entity controls fade
- [ ] State in HA reflects actual device state (not stale)
- [ ] EEPROM persistence: settings survive reboot, state republished correctly

---

## File Change Summary

| File | Change |
|------|--------|
| `include/ha_config.h` | New — replaces `keys.h` with local broker config |
| `src/main.cpp` | Switch WiFiClientSecure → WiFiClient, update connect logic |
| `lib/HEX_controller/HEX_controller.h` | Add discovery + state publish method declarations |
| `lib/HEX_controller/HEX_controller.cpp` | Add discovery publisher, JSON parser, state publisher |
| `platformio.ini` | Add ArduinoJson, remove AWS IoT lib |
| `.gitignore` | Add `ha_config.h` |

---

## Notes
- The ESP32 and HA RPi must be on the same local network/VLAN.
- `device_id` in `ha_config.h` should be unique if you ever run multiple Hexaleaf units.
- EEPROM settings (brightness, mode, colors) continue to work as before — they persist across reboots independently of HA.
- Audio-reactive modes (AudioBeatReact, AudioFreqPool) are available as effects but HA has no mic control — the mic just activates when those effects are selected.
