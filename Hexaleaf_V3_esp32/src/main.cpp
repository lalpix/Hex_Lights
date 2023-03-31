
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
#include "common_data.h"

char const *topicArray[] = {
    "mode",
    "speed",
    "fade",
    "brightness",
    "singleHexColor",
    "primaryColor",
    "secondaryColor",
    "layout",
    "power",
};
int num_topics = 9;
Hex_controller *hexController;
int localChangePeriod = 20000;
long lastChangeMs = 0;
bool power = true;
bool localRun = false;
bool firstSetup = true;
// TODO  Storing last settings, layout at least
// Upgrade audio to adjust levels over time
// SET colors prim and sec
// min/max speed for each mode,
WiFiManager wm;
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

void publishMessage(const char *topic, String payload)
{
    if (client.publish(topic, payload.c_str(), true))
        Serial.println("Message publised [" + String(topic) + "]: " + payload);
}
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
    power = true;

    if (strstr(topic, "power"))
    {
        if (strstr((char *)payload, "off"))
        {
            power = false;
            Serial.println("powering off");
            hexController->fill_all_hex(CRGB::Black);
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
            }
        }
        else if (strstr(topic, "primaryColor"))
        {
            std::string str((char *)payload);
            CRGB *clr = parseColorFromText(str);
            hexController->set_color(*clr, 1);
            free(clr);
        }
        else if (strstr(topic, "secondaryColor"))
        {
            std::string str((char *)payload);
            CRGB *clr = parseColorFromText(str);
            hexController->set_color(*clr, 2);
            free(clr);
        }
        else if (strstr(topic, "speed"))
        {
            int s = atoi((char *)payload);
            hexController->set_speed(s);
            Serial.printf("speed is now %d\n", s);
        }
        else if (strstr(topic, "fade"))
        {
            int s = atoi((char *)payload);
            hexController->set_fade(s);
            Serial.printf("fade is now %d\n", s);
        }
        else if (strstr(topic, "rainbow"))
        {
            int s = atoi((char *)payload);
            hexController->set_rainbow(s);
            Serial.printf("rainbow is now %d\n", s);
        }
        else if (strstr(topic, "brightness"))
        {
            int s = atoi((char *)payload);
            hexController->set_brightness(s);
            Serial.printf("brightness is now %d\n", s);
        }
        else if (strstr(topic, "layout"))
        {
            std::string delimiterBox("::");
            std::string str((char *)payload);
            int pos = str.find_first_of(delimiterBox);

            int boxNum = atoi(str.substr(0, pos).c_str());
            str.erase(0, pos + delimiterBox.length());
            int **layout = parseLayout(str, boxNum);
            hexControllerSetup(boxNum, layout, false);
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

    for (int i = 0; i < num_topics; i++)
    {
        client.subscribe(topicArray[i], 1);
    }

    char tpc[] = "test";
    publishMessage(tpc, "setup done");
    Serial.println("MQTT_Connected!");
}
void setup()
{
    pinMode(0, INPUT_PULLUP);
    Serial.begin(115200);
    delay(2000); // power-up safety delay
    Serial.println("> Setup....");
    int **setupLayout = new int *[1];
    setupLayout[0] = new int[2];
    setupLayout[0][0] = 0;
    setupLayout[0][1] = 0;
    hexControllerSetup(1, setupLayout, true);
    WifiSetup();
    Serial.println("Setup DONE");

    delay(2000);
}
void loop()
{

    // while (true)
    // {
    //     std::string delimiterBox("::");
    //     std::string str("4::0,0|3,0|0,5|0,4|"); // 3,4|5,6|
    //     int pos = str.find_first_of(delimiterBox);

    //     int boxNum = atoi(str.substr(0, pos).c_str());
    //     str.erase(0, pos + delimiterBox.length());
    //     // parseLayout(str, boxNum);
    //     int **res = parseLayout(str, boxNum);
    //     for (size_t i = 0; i < boxNum; i++)
    //     {
    //         for (size_t j = 0; j < 2; j++)
    //         {
    //             Serial.printf("box:%d cord:%d - %d\n", i, j, res[i][j]);
    //         }
    //     }
    //     hexControllerSetup(boxNum, res,false);
    //     delay(3000);
    //     free(res);
    // }

    if (!localRun)
    {
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
