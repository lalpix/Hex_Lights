
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

char *topicArray[] = {"mode", "speed", "anim", "fade", "power", "brightness"};
int num_topics = 6;
Hex_controller *hexController;
int localChangePeriod = 20000;
long lastChangeMs = 0;
bool power = true;
bool localRun = false;
WiFiManager wm;
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

void publishMessage(const char *topic, String payload)
{
    if (client.publish(topic, payload.c_str(), true))
        Serial.println("Message publised [" + String(topic) + "]: " + payload);
}

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
            hexController->set_brightness(s);
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
    net.setCACert(root_ca);

    client.setServer(mqtt_server, 8883);
    // Create a message handler
    client.setCallback(messageHandler);

    Serial.println("Connecting to Mqtt\n");
    // Loop until we're reconnected
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...\n");
        String clientId = "ESP8266Client-"; // Create a random client ID
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (!client.connect(clientId.c_str(), mqtt_username, mqtt_password))
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds"); // Wait 5 seconds before retrying
            delay(5000);
        }
    }
    hexController->fill_one_side_one_hex(CRGB::Green, 0, 1);
    hexController->show();
    if (!client.connected())
    {
        Serial.println("Mqtt timeout!");
        return;
    }
   
    client.subscribe(topicArray[0], 1);
    // Subscribe to a topic
    for (int i = 0; i < num_topics; i++)
    {
        client.subscribe(topicArray[i], 1);
       
    }
   
    char *tpc = "test";
    publishMessage(tpc, "setup done");
    Serial.println("MQTT_Connected!");
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
    WifiSetup();
    hexController->set_rainbow(1);
    hexController->change_mode(AudioFreqPool);
    hexController->set_brightness(255);
    Serial.println("Setup DONE");

    delay(2000);
}

void loop()
{

    if (!localRun)
    {
        client.loop();
    }
    else
    {
        // if (millis() > lastChangeMs + localChangePeriod){
        //     hexController->next_mode();
        //     lastChangeMs = millis();
        // }
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
