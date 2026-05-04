
#include <Arduino.h>
#include <FastLED.h>
#include <stdint.h>
#include <stdlib.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

#include "WiFiManager.h"

#include "ha_config.h"
#include "structs.h"
#include "HEX_controller.h"
#include "common_data.h"

// Forward declarations
void hexControllerSetupFromText(const char *payload, bool firstSetup);
void publishState();

// Legacy topics kept for backwards compatibility with the old app
char const *topicArray[] = {
    "mode",
    "speed",
    "fade",
    "brightness",
    "singleHexColor",
    "primaryColor",
    "secondaryColor",
    "layout",
    "rainbow",
    "power",
};
int num_topics = 10;

Hex_controller *hexController;
int localChangePeriod = 20000;
long lastChangeMs = 0;
bool power = true;
bool localRun = false;

// HA state tracking — kept in sync with every command so we can publish state
uint8_t ha_brightness = 255;
uint8_t ha_r = 0, ha_g = 0, ha_b = 255;
String  ha_effect = "RotationOuter";
uint8_t ha_speed = 1;
uint8_t ha_fade  = 1;

// HA MQTT topics built from device_id at setup
char ha_cmd_topic[64];
char ha_state_topic[64];
char ha_speed_cmd_topic[64];
char ha_speed_state_topic[64];
char ha_fade_cmd_topic[64];
char ha_fade_state_topic[64];

WiFiManager wm;
WiFiClient  net;
PubSubClient client(net);

// ---------- helpers ----------

void publishMessage(const char *topic, const char *payload, bool retained = true)
{
    if (client.publish(topic, payload, retained))
        Serial.println("Published [" + String(topic) + "]: " + String(payload));
    else
        Serial.println("Publish FAILED [" + String(topic) + "] (buffer too small?)");
}

String modeToEffectName(Mode m)
{
    switch (m)
    {
    case Stationar:          return "Stationar";
    case RotationOuter:      return "RotationOuter";
    case StationarOuter:     return "StationarOuter";
    case RandColorRandHex:   return "RandColorRandHex";
    case RandColorRandHexFade: return "RandColorRandHexFade";
    case RainbowSwipeVert:   return "RainbowSwipeVert";
    case RainbowSwipeHorz:   return "RainbowSwipeHorz";
    case AudioBeatReact:     return "AudioBeatReact";
    case AudioFreqPool:      return "AudioFreqPool";
    case TwoColorFading:     return "TwoColorFading";
    case PresetAnim:         return "PresetAnim1";
    default:                 return "Stationar";
    }
}

// ---------- HA state publish ----------

void publishState()
{
    StaticJsonDocument<256> doc;
    doc["state"] = power ? "ON" : "OFF";
    if (power)
    {
        doc["brightness"] = ha_brightness;
        JsonObject color = doc.createNestedObject("color");
        color["r"] = ha_r;
        color["g"] = ha_g;
        color["b"] = ha_b;
        doc["effect"] = ha_effect;
    }
    char buf[256];
    serializeJson(doc, buf);
    publishMessage(ha_state_topic, buf);
    publishMessage(ha_speed_state_topic, String(ha_speed).c_str());
    publishMessage(ha_fade_state_topic,  String(ha_fade).c_str());
}

// ---------- HA MQTT discovery ----------

void publishDiscovery()
{
    char topic[80];
    char buf[1024];

    // Light entity
    {
        StaticJsonDocument<1024> doc;
        doc["name"]          = "Hex Lights";
        doc["unique_id"]     = device_id;
        doc["schema"]        = "json";
        doc["command_topic"] = ha_cmd_topic;
        doc["state_topic"]   = ha_state_topic;
        doc["brightness"]    = true;
        doc["color_mode"]    = true;
        JsonArray cmodes = doc.createNestedArray("supported_color_modes");
        cmodes.add("rgb");
        doc["effect"] = true;
        JsonArray elist = doc.createNestedArray("effect_list");
        const char *effects[] = {
            "Stationar", "RotationOuter", "StationarOuter",
            "RandColorRandHex", "RandColorRandHexFade",
            "RainbowSwipeVert", "RainbowSwipeHorz",
            "TwoColorFading", "AudioBeatReact", "AudioFreqPool",
            "PresetAnim1", "PresetAnim2", "PresetAnim3", "PresetAnim4"
        };
        for (const char *e : effects) elist.add(e);
        JsonObject dev = doc.createNestedObject("device");
        JsonArray  ids = dev.createNestedArray("identifiers");
        ids.add(device_id);
        dev["name"]         = "Hex Lights";
        dev["model"]        = "Hexaleaf V3";
        dev["manufacturer"] = "DIY";
        serializeJson(doc, buf);
        snprintf(topic, sizeof(topic), "homeassistant/light/%s/config", device_id);
        publishMessage(topic, buf);
    }

    // Speed number entity
    {
        StaticJsonDocument<512> doc;
        char uid[40];
        snprintf(uid, sizeof(uid), "%s_speed", device_id);
        doc["name"]          = "Hex Lights Speed";
        doc["unique_id"]     = uid;
        doc["command_topic"] = ha_speed_cmd_topic;
        doc["state_topic"]   = ha_speed_state_topic;
        doc["min"]  = 0;
        doc["max"]  = 100;
        doc["step"] = 1;
        JsonObject dev = doc.createNestedObject("device");
        JsonArray  ids = dev.createNestedArray("identifiers");
        ids.add(device_id);
        serializeJson(doc, buf);
        snprintf(topic, sizeof(topic), "homeassistant/number/%s_speed/config", device_id);
        publishMessage(topic, buf);
    }

    // Fade number entity
    {
        StaticJsonDocument<512> doc;
        char uid[40];
        snprintf(uid, sizeof(uid), "%s_fade", device_id);
        doc["name"]          = "Hex Lights Fade";
        doc["unique_id"]     = uid;
        doc["command_topic"] = ha_fade_cmd_topic;
        doc["state_topic"]   = ha_fade_state_topic;
        doc["min"]  = 0;
        doc["max"]  = 255;
        doc["step"] = 1;
        JsonObject dev = doc.createNestedObject("device");
        JsonArray  ids = dev.createNestedArray("identifiers");
        ids.add(device_id);
        serializeJson(doc, buf);
        snprintf(topic, sizeof(topic), "homeassistant/number/%s_fade/config", device_id);
        publishMessage(topic, buf);
    }
}

// ---------- effect application ----------

void applyEffect(const char *effect)
{
    ha_effect = effect;
    Mode m = Stationar;
    bool isPreset = false;
    uint8_t presetNum = 0;

    if      (strcmp(effect, "Stationar")            == 0) m = Stationar;
    else if (strcmp(effect, "RotationOuter")        == 0) m = RotationOuter;
    else if (strcmp(effect, "StationarOuter")       == 0) m = StationarOuter;
    else if (strcmp(effect, "RandColorRandHex")     == 0) m = RandColorRandHex;
    else if (strcmp(effect, "RandColorRandHexFade") == 0) m = RandColorRandHexFade;
    else if (strcmp(effect, "RainbowSwipeVert")     == 0) m = RainbowSwipeVert;
    else if (strcmp(effect, "RainbowSwipeHorz")     == 0) m = RainbowSwipeHorz;
    else if (strcmp(effect, "TwoColorFading")       == 0) m = TwoColorFading;
    else if (strcmp(effect, "AudioBeatReact")       == 0) m = AudioBeatReact;
    else if (strcmp(effect, "AudioFreqPool")        == 0) m = AudioFreqPool;
    else if (strcmp(effect, "PresetAnim1") == 0) { isPreset = true; presetNum = 1; }
    else if (strcmp(effect, "PresetAnim2") == 0) { isPreset = true; presetNum = 2; }
    else if (strcmp(effect, "PresetAnim3") == 0) { isPreset = true; presetNum = 3; }
    else if (strcmp(effect, "PresetAnim4") == 0) { isPreset = true; presetNum = 4; }

    if (isPreset)
    {
        hexController->set_pre_anim(presetNum);
        EEPROM.write(modeAddr, PresetAnim);
    }
    else
    {
        hexController->change_mode(m);
        EEPROM.write(modeAddr, m);
    }
    EEPROM.commit();
}

// ---------- HA JSON command handler ----------

void handleHACommand(byte *payload, unsigned int length)
{
    StaticJsonDocument<256> doc;
    if (deserializeJson(doc, payload, length))
    {
        Serial.println("HA JSON parse error");
        return;
    }

    if (doc.containsKey("state"))
    {
        power = strcmp(doc["state"], "ON") == 0;
        EEPROM.write(powerAddr, power);
        EEPROM.commit();
        if (!power)
        {
            hexController->fill_all_hex(CRGB::Black);
            hexController->show();
        }
    }

    if (power)
    {
        if (doc.containsKey("brightness"))
        {
            ha_brightness = doc["brightness"];
            hexController->set_brightness(ha_brightness);
            EEPROM.write(brightnessAddr, ha_brightness);
            EEPROM.commit();
        }
        if (doc.containsKey("color"))
        {
            ha_r = doc["color"]["r"];
            ha_g = doc["color"]["g"];
            ha_b = doc["color"]["b"];
            hexController->set_color(CRGB(ha_r, ha_g, ha_b), 0);
            char clrStr[12];
            snprintf(clrStr, sizeof(clrStr), "%d,%d,%d", ha_r, ha_g, ha_b);
            EEPROM.writeString(primaryClrAddr, clrStr);
            EEPROM.commit();
        }
        if (doc.containsKey("effect"))
        {
            applyEffect(doc["effect"]);
        }
    }

    publishState();
}

// ---------- legacy + HA message router ----------

void messageHandler(char *topic, byte *payload, unsigned int length)
{
    char msg[length + 1];
    for (unsigned int i = 0; i < length; i++) msg[i] = (char)payload[i];
    msg[length] = '\0';
    Serial.printf("incoming [%s]: %s\n", topic, msg);

    // HA light command (JSON)
    if (strcmp(topic, ha_cmd_topic) == 0)
    {
        handleHACommand(payload, length);
        return;
    }
    // HA speed command
    if (strcmp(topic, ha_speed_cmd_topic) == 0)
    {
        ha_speed = atoi(msg);
        hexController->set_speed(ha_speed);
        EEPROM.write(speedAddr, ha_speed);
        EEPROM.commit();
        publishMessage(ha_speed_state_topic, msg);
        return;
    }
    // HA fade command
    if (strcmp(topic, ha_fade_cmd_topic) == 0)
    {
        ha_fade = atoi(msg);
        hexController->set_fade(ha_fade);
        EEPROM.write(fadeAddr, ha_fade);
        EEPROM.commit();
        publishMessage(ha_fade_state_topic, msg);
        return;
    }

    // --- legacy topic handling (old app backwards compat) ---
    power = true;

    if (strstr(topic, "power"))
    {
        if (strstr((char *)payload, "off"))
        {
            power = false;
            Serial.println("powering off");
            hexController->fill_all_hex(CRGB::Black);
            hexController->show();
        }
        else
        {
            power = true;
            Serial.println("powering on");
        }
    }
    if (power)
    {
        if (strstr(topic, "mode"))
        {
            inputMode m = Stationar_;
            for (inputMode i = Stationar_; i != InputMode_num_; i = (inputMode)(i + 1))
            {
                if (strstr((char *)payload, inputModeName(i).c_str()))
                {
                    m = i;
                    Serial.printf("found mode: %s", inputModeName(i).c_str());
                }
            }
            if (m >= inputMode::TopBottom_)
            {
                uint8_t anim = m - inputMode::TopBottom_ + 1;
                Serial.printf("animation is now %d\n", anim);
                hexController->set_pre_anim(anim);
            }
            else
            {
                hexController->change_mode((Mode)m);
                Serial.printf("Mode is now %d\n", m);
                EEPROM.write(modeAddr, m);
            }
        }
        else if (strstr(topic, "primaryColor"))
        {
            std::string str((char *)payload);
            EEPROM.writeString(primaryClrAddr, (char *)payload);
            EEPROM.commit();
            CRGB *clr = parseColorFromText(str);
            ha_r = clr->r; ha_g = clr->g; ha_b = clr->b;
            hexController->set_color(*clr, 0);
            free(clr);
        }
        else if (strstr(topic, "secondaryColor"))
        {
            std::string str((char *)payload);
            EEPROM.writeString(secondaryClrAddr, (char *)payload);
            EEPROM.commit();
            CRGB *clr = parseColorFromText(str);
            hexController->set_color(*clr, 1);
            free(clr);
        }
        else if (strstr(topic, "speed"))
        {
            ha_speed = atoi((char *)payload);
            hexController->set_speed(ha_speed);
            EEPROM.write(speedAddr, ha_speed);
            EEPROM.commit();
            Serial.printf("speed is now %d\n", ha_speed);
        }
        else if (strstr(topic, "fade"))
        {
            ha_fade = atoi((char *)payload);
            hexController->set_fade(ha_fade);
            EEPROM.write(fadeAddr, ha_fade);
            EEPROM.commit();
            Serial.printf("fade is now %d\n", ha_fade);
        }
        else if (strstr(topic, "rainbow"))
        {
            int s = atoi((char *)payload);
            hexController->set_rainbow(s);
            EEPROM.write(rainbowAddr, s);
            EEPROM.commit();
            Serial.printf("rainbow is now %d\n", s);
        }
        else if (strstr(topic, "brightness"))
        {
            ha_brightness = atoi((char *)payload);
            hexController->set_brightness(ha_brightness);
            EEPROM.write(brightnessAddr, ha_brightness);
            EEPROM.commit();
            Serial.printf("brightness is now %d\n", ha_brightness);
        }
        else if (strstr(topic, "layout"))
        {
            hexControllerSetupFromText((char *)payload, false);
        }
        else if (strstr(topic, "singleHexColor"))
        {
            std::string delimiterBox("::");
            std::string str((char *)payload);
            int pos = str.find_first_of(delimiterBox);
            int boxId = atoi(str.substr(0, pos).c_str()) - 1;
            str.erase(0, pos + delimiterBox.length());
            CRGB *clr = parseColorFromText(str);
            Serial.printf("should print box %d with color:\n", boxId);
            printCRGB(*clr);
            hexController->fill_one_hex(boxId, *clr);
            hexController->change_mode(NotUpdating);
            free(clr);
        }
    }
}

// ---------- hexController helpers ----------

void hexControllerSetup(int numBoxes, int **layout, bool isFirstTime)
{
    hexController = new Hex_controller(numBoxes, layout);
    hexController->set_serial(Serial);
    Serial.println("Set serial Done");
    Serial.printf("present %d nodes with positions:", numBoxes);
    hexController->init(isFirstTime);
    Serial.println("hex init Done");
    hexController->set_rainbow(1);
    hexController->change_mode(RotationOuter);
    hexController->set_brightness(255);
}

void hexControllerSetupFromText(const char *payload, bool firstSetup)
{
    std::string str((char *)payload);
    std::string delimiterBox("::");
    EEPROM.writeString(layoutAddr, (char *)payload);
    EEPROM.commit();
    int pos = str.find_first_of(delimiterBox);
    int boxNum = atoi(str.substr(0, pos).c_str());
    Serial.printf("running setup from text numbox %d first %d chars %s\n", boxNum, firstSetup, payload);
    str.erase(0, pos + delimiterBox.length());
    int **layout = parseLayout(str, boxNum);
    hexControllerSetup(boxNum, layout, firstSetup);
}

// ---------- MQTT connect / reconnect ----------

void mqttSubscribeAll()
{
    for (int i = 0; i < num_topics; i++)
        client.subscribe(topicArray[i], 1);
    client.subscribe(ha_cmd_topic,       1);
    client.subscribe(ha_speed_cmd_topic, 1);
    client.subscribe(ha_fade_cmd_topic,  1);
}

void reconnectMQTT()
{
    static long lastAttemptMs = 0;
    if (client.connected() || WiFi.status() != WL_CONNECTED) return;
    if (millis() - lastAttemptMs < 5000) return;
    lastAttemptMs = millis();

    Serial.print("Reconnecting MQTT... ");
    String clientId = "HexaleafClient-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass))
    {
        Serial.println("connected");
        mqttSubscribeAll();
        publishDiscovery();
        publishState();
    }
    else
    {
        Serial.print("failed rc=");
        Serial.println(client.state());
    }
}

// ---------- WiFi + initial MQTT setup ----------

void WifiSetup()
{
    Serial.println("Connecting to WiFi...");
    wm.setDebugOutput(false);

    hexController->fill_one_side_one_hex(CRGB::Red, 0, 0);
    hexController->show();
    WiFi.mode(WIFI_STA);
    bool res = wm.autoConnect("Hexaleaf");
    if (!res)
    {
        Serial.println("WiFi timeout — running locally");
        localRun = true;
        return;
    }
    Serial.println("WiFi connected");
    hexController->fill_one_side_one_hex(CRGB::Green, 0, 0);
    hexController->show();
    localRun = false;

    // Build HA topic strings from device_id
    snprintf(ha_cmd_topic,        sizeof(ha_cmd_topic),        "hexlights/%s/set",         device_id);
    snprintf(ha_state_topic,      sizeof(ha_state_topic),      "hexlights/%s/state",       device_id);
    snprintf(ha_speed_cmd_topic,  sizeof(ha_speed_cmd_topic),  "hexlights/%s/speed/set",   device_id);
    snprintf(ha_speed_state_topic,sizeof(ha_speed_state_topic),"hexlights/%s/speed/state", device_id);
    snprintf(ha_fade_cmd_topic,   sizeof(ha_fade_cmd_topic),   "hexlights/%s/fade/set",    device_id);
    snprintf(ha_fade_state_topic, sizeof(ha_fade_state_topic), "hexlights/%s/fade/state",  device_id);

    hexController->fill_one_side_one_hex(CRGB::Red, 0, 1);
    hexController->show();

    // Increase MQTT buffer for discovery payloads (~700 bytes)
    client.setBufferSize(1024);
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(messageHandler);

    Serial.println("Connecting to MQTT...");
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        String clientId = "HexaleafClient-" + String(random(0xffff), HEX);
        if (!client.connect(clientId.c_str(), mqtt_user, mqtt_pass))
        {
            Serial.print("failed rc=");
            Serial.print(client.state());
            Serial.println(" — retrying in 5 s");
            delay(5000);
        }
    }

    hexController->fill_one_side_one_hex(CRGB::Green, 0, 1);
    hexController->show();

    mqttSubscribeAll();
    publishDiscovery();
    publishState();
    Serial.println("MQTT connected!");
}

// ---------- EEPROM ----------

void loadDataFromEEPROM()
{
    // Primary color
    String primaryClr = EEPROM.readString(primaryClrAddr);
    CRGB *priClr = parseColorFromText(std::string(primaryClr.c_str()));
    ha_r = priClr->r; ha_g = priClr->g; ha_b = priClr->b;
    hexController->set_color(*priClr, 0);
    free(priClr);

    // Secondary color
    String secondaryClr = EEPROM.readString(secondaryClrAddr);
    CRGB *secClr = parseColorFromText(std::string(secondaryClr.c_str()));
    hexController->set_color(*secClr, 1);
    free(secClr);

    // Mode
    Mode modeFromEEPROM = (Mode)EEPROM.read(modeAddr);
    hexController->change_mode(modeFromEEPROM);
    ha_effect = modeToEffectName(modeFromEEPROM);

    // Scalars
    ha_brightness = EEPROM.read(brightnessAddr);
    ha_fade       = EEPROM.read(fadeAddr);
    ha_speed      = EEPROM.read(speedAddr);
    hexController->set_brightness(ha_brightness);
    hexController->set_fade(ha_fade);
    hexController->set_speed(ha_speed);
    hexController->set_rainbow(EEPROM.read(rainbowAddr));

    power = EEPROM.read(powerAddr);
    if (!power)
    {
        hexController->fill_all_hex(CRGB::Black);
        hexController->show();
    }
}

// ---------- Arduino entry points ----------

void setup()
{
    eepromInit();
    pinMode(0, INPUT_PULLUP);
    Serial.begin(115200);
    delay(2000);
    Serial.println("> Setup....");

    String layoutLoad = EEPROM.readString(layoutAddr);
    Serial.println(layoutLoad);
    hexControllerSetupFromText(layoutLoad.c_str(), true);
    WifiSetup();
    loadDataFromEEPROM();

    // Re-publish state after loading EEPROM (WiFiSetup published before EEPROM load)
    if (!localRun && client.connected())
        publishState();

    Serial.println("Setup DONE");
    delay(2000);
}

void loop()
{
    if (!localRun)
    {
        if (!client.connected())
            reconnectMQTT();
        client.loop();
    }
    else
    {
        if (millis() > lastChangeMs + localChangePeriod)
        {
            hexController->next_mode();
            lastChangeMs = millis();
        }
    }

    if (power)
    {
        hexController->update();
    }
    else
    {
        hexController->fill_all_hex(CRGB::Black);
        delay(1000);
    }
}
