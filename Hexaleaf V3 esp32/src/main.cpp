
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
#include "HEX_controller.h"

// esp_rmaker_device_t *device = esp_rmaker_device_create("Light", NULL, NULL);
/* ----- Fastled constants ----- */

const long wifiTimeout = 20000;
String topicArray[] = {"mode", "speed", "anim", "fade", "power", "brightness"};

Hex_controller *hexController;
int period = 5000;
Mode m = Stationar;
bool power = true;
bool localRun = true;
WiFiManager wm;
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
        if (strstr((char *)payload, "off"))
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
        else if (strstr(topic, "brightness"))
        {
            int s = atoi((char *)payload);
            hexController->set_fade(s);
            Serial.printf("brightness is now %d\n", s);
        }
    }
}
void WifiSetup()
{
    Serial.println("connecting to WiFi\n");

    // wm.resetSettings();
    wm.setDebugOutput(false);

    hexController->fill_one_side_one_hex(CRGB::Red, 0, 0);
    hexController->show();
    WiFi.mode(WIFI_STA);
    bool res = wm.autoConnect("Hexaleaf");
    if (res)
    {
        Serial.println("WiFi connected\n");
        hexController->fill_one_side_one_hex(CRGB::Green, 0, 0);
        hexController->show();
        localRun = false;
    }
    else
    {
        Serial.println("Wifi timeout - running localy");
        localRun = true;
        return;
    }
    hexController->fill_one_side_one_hex(CRGB::Red, 0, 1);
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
    hexController->fill_one_side_one_hex(CRGB::Green, 0, 1);
    hexController->show();
    if (!client.connected())
    {
        Serial.println("AWS_IoT_Timeout!");
        return;
    }

    // Subscribe to a topic
    for (int i = 0; i < topicArray->length(); i++)
    {
        // not tested
        char tmp[topicArray[i].length()];
        topicArray[i].toCharArray(tmp, topicArray[i].length());
        client.subscribe(tmp);
    }

    Serial.println("AWS_IoT_Connected!");
}

void setup()
{
    pinMode(0, INPUT_PULLUP);
    Serial.begin(115200);
    delay(2000); // power-up safety delay
    Serial.println("> Setup....");
    hexController = new Hex_controller();
    hexController->set_serial(Serial);
    hexController->init();
    // WifiSetup();
    hexController->set_rainbow(0);
    hexController->change_mode(StationarOuter);
    hexController->set_pre_anim(5);
    hexController->set_fade(100);
    Serial.println("Setup DONE");

    delay(2000);
}

void loop()
{

    if (!localRun)
    {
        client.loop();
    }
    if (power)
    {
        hexController->update();
    }
    else
    {
        hexController->fill_all_hex(CRGB::Black);
    }
}
