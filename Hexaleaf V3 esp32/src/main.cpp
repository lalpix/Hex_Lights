
#include <Arduino.h>
#include <FastLED.h>
#include <stdint.h>
#include <stdlib.h>
#include <WiFi.h>
#include <aws_iot.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#include "WiFiManager.h"

#include "keys.h"
#include "structs.h"
#include "HEX_node.h"
#include "HEX_controller.h"

// esp_rmaker_device_t *device = esp_rmaker_device_create("Light", NULL, NULL);
/* ----- Fastled constants ----- */

const long wifiTimeout = 20000;
const uint8_t kPinLedStrip = 22; // M5StickC grove port, yellow cable

const uint8_t kNumLeds = 150;

const uint8_t kLedStripBrightness = 150;

Hex_controller *hexController;
String topic = "";
String payload = "";
String last_payload = "";
int red = 100;
int green = 100;
int blue = 0;
long t = 0;
long last_ch = 0;
int period = 5000;
Mode m = Stationar;
CRGB ledStrip_[kNumLeds];
bool power = true;

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

void messageHandler(char *topic, byte *payload, unsigned int length)
{
    Serial.print("incoming topic: ");
    Serial.print(topic);
    char msg[length + 1];
    for (int i = 0; i < length; i++)
    {
        msg[i] = (char)payload[i];
    }
    msg[length] = '\0';
    Serial.print(" msg: ");
    Serial.println(msg);
    if (strstr(topic, "power"))
    {
        if (strstr((char*)payload,"off"))
        {
            Serial.print("power off");
            power = false;
        }
        else if (strstr((char *)payload, "on"))
        {
            Serial.print("power on");
            power = true;
        }
    }
    if (power)
    {
        if (strstr(topic, "mode"))
        {
            Mode m = (Mode)atoi((char *)payload);
            if (m >= Mode::Mode_num - 1)
            {
                Serial.printf("ERROR:not valid mode %d\n", m);
            }
            else
            {
                hexController->change_mode(m);
                Serial.printf("Mode is now %d\n", m);
            }
        }
        else if (strstr(topic, "speed"))
        {
            int s = atoi((char *)payload);
            hexController->set_speed(s);
            Serial.printf("speed is now %d\n", s);
        }
        else if (strstr(topic, "anim"))
        {
            int s = atoi((char *)payload);
            hexController->set_pre_anim(s);
            Serial.printf("animation is now %d\n", s);
        }
        else if (strstr(topic, "fade"))
        {
            int s = atoi((char *)payload);
            hexController->set_fade(s);
            Serial.printf("fade is now %d\n", s);
        }
    }
}
void WifiSetup()
{
    Serial.println("connecting to WiFi\n");

    WiFiManager wm;
    // wm.resetSettings();
    wm.setDebugOutput(false);
    //WiFiManagerParameter custom_text("btn", "ender mode num here to start offline", "", 1);
    //wm.addParameter(&custom_text);
    leds[0] = CRGB::Red;
    hexController->show();
    WiFi.mode(WIFI_STA);
    bool res = wm.autoConnect("Hexaleaf");
    if (res)
    {
        Serial.println("WiFi connected\n");
    }

    leds[0] = CRGB::Green;
    hexController->show();
    leds[1] = CRGB::Red;
    hexController->show();

    net.setCACert(AWS_CERT_CA);
    net.setCertificate(AWS_CERT_CRT);
    net.setPrivateKey(AWS_CERT_PRIVATE);

    // Connect to the MQTT broker on the AWS endpoint we defined earlier
    client.setServer(AWS_IOT_ENDPOINT, 8883);

    // Create a message handler
    client.setCallback(messageHandler);

    Serial.println("Connecting_to_AWS_IOT\n");

    while (!client.connect(THINGNAME))
    {
        Serial.print(".");
        delay(100);
    }

    if (!client.connected())
    {
        Serial.println("AWS_IoT_Timeout!");
        return;
    }

    // Subscribe to a topic
    client.subscribe("mode");
    client.subscribe("speed");
    client.subscribe("anim");
    client.subscribe("fade");
    client.subscribe("power");

    leds[1] = CRGB::Green;
    hexController->show();
    Serial.println("AWS_IoT_Connected!");
}

void setup()
{

    Serial.begin(115200);
    delay(2000); // power-up safety delay
    Serial.println("> Setup....");
    hexController = new Hex_controller();
    hexController->set_serial(Serial);
    hexController->init();
    WifiSetup();
    setupI2Smic();
    setupSpectrumAnalysis();
    hexController->set_rainbow(1);
    hexController->set_fade(30);
    hexController->change_mode(AudioFreqPool);
    Serial.println("Setup DONE");

    delay(2000);
}

void loop()
{
    // delay( 5000 );
    client.loop();
    if (power)
    {
        hexController->update();
    }else{
        hexController->fill_all_hex(CRGB::Black);
        hexController->show();
    }
}
