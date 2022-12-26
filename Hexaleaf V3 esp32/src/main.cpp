
#include <Arduino.h>
#include <FastLED.h>
#include <stdint.h>
#include <stdlib.h>
#include <WiFi.h>
#include <aws_iot.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "keys.h"
#include "structs.h"
#include "HEX_node.h"
#include "HEX_controller.h"
// #include "Audio.h"

// esp_rmaker_device_t *device = esp_rmaker_device_create("Light", NULL, NULL);
/* ----- Fastled constants ----- */

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

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);
void messageHandler(char *topic, byte *payload, unsigned int length)
{
    Serial.print("incoming: ");
    Serial.println(topic);

    // /*##################### Lamp 1 #####################*/
    // if (strstr(topic, "esp32/lamp1"))
    // {
    //     StaticJsonDocument<200> doc;
    //     deserializeJson(doc, payload);
    //     String Relay1 = doc["status"];
    //     int r1 = Relay1.toInt();
    //     if (r1 == 1)
    //     {
    //         digitalWrite(lamp1, LOW);
    //         Serial.print("Lamp1 is ON");
    //     }
    //     else if (r1 == 0)
    //     {
    //         digitalWrite(lamp1, HIGH);
    //         Serial.print("Lamp1 is OFF");
    //     }
    // }

    // /*##################### Lamp 2 #####################*/
    // if (strstr(topic, "esp32/lamp2"))
    // {
    //     StaticJsonDocument<200> doc;
    //     deserializeJson(doc, payload);
    //     String Relay2 = doc["status"];
    //     int r2 = Relay2.toInt();
    //     if (r2 == 1)
    //     {
    //         digitalWrite(lamp2, LOW);
    //         Serial.print("Lamp2 is ON");
    //     }
    //     else if (r2 == 0)
    //     {
    //         digitalWrite(lamp2, HIGH);
    //         Serial.print("Lamp2 is OFF");
    //     }
    // }

    // /*##################### Lamp 3 #####################*/
    // if (strstr(topic, "esp32/lamp3"))
    // {
    //     StaticJsonDocument<200> doc;
    //     deserializeJson(doc, payload);
    //     String Relay3 = doc["status"];
    //     int r3 = Relay3.toInt();
    //     if (r3 == 1)
    //     {
    //         digitalWrite(lamp3, LOW);
    //         Serial.print("Lamp3 is ON");
    //     }
    //     else if (r3 == 0)
    //     {
    //         digitalWrite(lamp3, HIGH);
    //         Serial.print("Lamp3 is OFF");
    //     }
    // }

    // /*##################### Lamp 4 #####################*/
    // if (strstr(topic, "esp32/lamp4"))
    // {
    //     StaticJsonDocument<200> doc;
    //     deserializeJson(doc, payload);
    //     String Relay4 = doc["status"];
    //     int r4 = Relay4.toInt();
    //     if (r4 == 1)
    //     {
    //         digitalWrite(lamp4, LOW);
    //         Serial.print("Lamp4 is ON");
    //     }
    //     else if (r4 == 0)
    //     {
    //         digitalWrite(lamp4, HIGH);
    //         Serial.print("Lamp4 is OFF");
    //     }
    // }
    Serial.println();
}
void WifiSetup()
{

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_ADR, WIFI_PASS);
    Serial.println("Connecting to Wi-Fi ");

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print("~");
    }
    Serial.println();
    net.setCACert(AWS_CERT_CA);
    net.setCertificate(AWS_CERT_CRT);
    net.setPrivateKey(AWS_CERT_PRIVATE);

    // Connect to the MQTT broker on the AWS endpoint we defined earlier
    client.setServer(AWS_IOT_ENDPOINT, 8883);

    // Create a message handler
    client.setCallback(messageHandler);

    Serial.println("Connecting to AWS IOT");

    while (!client.connect(THINGNAME))
    {
        Serial.print(".");
        delay(100);
    }

    if (!client.connected())
    {
        Serial.println("AWS IoT Timeout!");
        return;
    }

    // Subscribe to a topic
    client.subscribe("testTopic");
    // client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC2);
    // client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC3);
    // client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC4);

    Serial.println("AWS IoT Connected!");
}
void setupLedStrip()
{
    FastLED.addLeds<WS2812B, kPinLedStrip>(ledStrip_, kNumLeds);
    FastLED.clear();
    // FastLED.setBrightness(kLedStripBrightness);
    // FastLED.setMaxPowerInVoltsAndMilliamps(5, 450); // Set maximum power consumption to 5 V and 450 mA
    ledStrip_[0].setRGB(150, 0, 0);

    FastLED.show();
}
void setup()
{
    Serial.begin(115200);
    delay(2000); // power-up safety delay

    Serial.println("> Setup.");
    WifiSetup();
    hexController = new Hex_controller();
    hexController->set_serial(Serial);
    hexController->init();
    hexController->change_mode(RotationOuter);
    hexController->set_pre_anim(1);
    hexController->set_rainbow(1);
    hexController->set_fade(true);
    hexController->set_speed(100);
    hexController->update();
    last_ch = millis();
    /*
      setupI2Smic();
      setupSpectrumAnalysis();
      setupLedStrip();*/
    delay(2000);
}

void loop()
{
    // delay( 5000 );
    client.loop();
    hexController->update();
    /*
        float magnitudeBand[kFreqBandCount] = { 0.0f };

        float magnitudeBandWeightedMax = 0.0f;

        newAudioReading(magnitudeBand, &magnitudeBandWeightedMax);

        // ----- Beat detection -----

        // Maintain history of last three magnitude values of the bass band
        beatHist_[kBeatDetectBand][0] = beatHist_[kBeatDetectBand][1];
        beatHist_[kBeatDetectBand][1] = beatHist_[kBeatDetectBand][2];
        beatHist_[kBeatDetectBand][2] = magnitudeBand[kBeatDetectBand] * kFreqBandAmp[kBeatDetectBand] * sensitivityFactor_;

        float diff1 = beatHist_[1] - beatHist_[0];
        float diff2 = beatHist_[2] - beatHist_[1];

        // Detect magnitude peak
        if ( ((diff1 >= kBeatThreshold) && (diff2 < 0)) || ((diff1 > 0) && (diff2 <= -kBeatThreshold)) )
        {
            Serial.printf("beat detected");
            beatVisIntensity_ = 250;
        }
        else {
            if ( beatVisIntensity_ >= 25 )
                beatVisIntensity_ -= 25;
        }
     // ----- Update the Led strip -----

        if (kNumLeds <= 2*kFreqBandCount + 4)
        {
            // Show beat detection at the beginning of the strip
            const uint8_t numBassLeds = (kNumLeds - kFreqBandCount) / 2;

            for (int i = 0; i < numBassLeds; i++)
            {
                ledStrip_[i].setHSV( 250, 255, beatVisIntensity_ );
            }

            // Show frequency intensities on the remaining Leds
            const uint8_t colorStart = 30;
            const uint8_t colorEnd   = 210;
            const uint8_t colorStep  = (colorEnd - colorStart) / kFreqBandCount;

            for (int k = 0; k < kFreqBandCount; k++)
            {
                uint8_t color = colorStart + k * colorStep;
                uint8_t lightness = min( int(magnitudeBand[k] * kFreqBandAmp[k] * sensitivityFactor_), 255);

                ledStrip_[k+numBassLeds].setHSV(color, 255, lightness);
            }

            // Show beat detection at the beginning of the strip
            for (int i = numBassLeds + kFreqBandCount; i < kNumLeds; i++)
            {
                ledStrip_[i].setHSV( 250, 255, beatVisIntensity_ );
            }
        }
        else
        {
            // Show beat detection at the beginning of the strip
            const uint8_t numBassLeds = (kNumLeds - 2 * kFreqBandCount) / 2;

            for (int i = 0; i < numBassLeds; i++)
            {
                ledStrip_[i].setHSV( 250, 255, beatVisIntensity_ );
            }

            // Show frequency intensities on the remaining Leds
            const uint8_t colorStart = 30;
            const uint8_t colorEnd   = 210;
            const uint8_t colorStep  = (colorEnd - colorStart) / kFreqBandCount;

            for (int k = 0; k < kFreqBandCount; k++)
            {
                uint8_t color = colorStart + k * colorStep;
                uint8_t lightness = min( int(magnitudeBand[k] * kFreqBandAmp[k] * sensitivityFactor_), 255);

                ledStrip_[numBassLeds + k].setHSV(color, 255, lightness);

                ledStrip_[numBassLeds + 2*kFreqBandCount - k - 1].setHSV(color, 255, lightness);
            }

            // Show beat detection at the beginning of the strip
            for (int i = numBassLeds + 2*kFreqBandCount; i < kNumLeds; i++)
            {
                ledStrip_[i].setHSV( 250, 255, beatVisIntensity_ );
            }
        }

        FastLED.show();
        float nf;

        if (fabs(magnitudeBand[1]) < 0.001f)
        {
            nf = 1.0f;
        }
        else
        {
            nf = 1.0f / magnitudeBand[1];
        }
        */
    /*
       Serial.printf("0:%04.2f 1:%04.2f 2:%04.2f 3:%04.2f 4:%04.2f 5:%04.2f 6:%04.2f 7:%04.2f 8:%04.2f 9:%04.2f 10:%04.2f 11:%04.2f 12:%04.2f 13:%04.2f 14:%04.2f 15:%04.2f 16:%04.2f 17:%04.2f 18:%04.2f 19:%04.2f Sum:%05.1f Sens: %04.1f \n",//t: %d
                      magnitudeBand[0] * nf,
                      magnitudeBand[1] * nf,
                      magnitudeBand[2] * nf,
                      magnitudeBand[3] * nf,
                      magnitudeBand[4] * nf,
                      magnitudeBand[5] * nf,
                      magnitudeBand[6] * nf,
                      magnitudeBand[7] * nf,
                      magnitudeBand[8] * nf,
                      magnitudeBand[9] * nf,
                      magnitudeBand[10] * nf,
                      magnitudeBand[11] * nf,
                      magnitudeBand[12] * nf,
                      magnitudeBand[13] * nf,
                      magnitudeBand[14] * nf,
                      magnitudeBand[15] * nf,
                      magnitudeBand[16] * nf,
                      magnitudeBand[17] * nf,
                      magnitudeBand[18] * nf,
                      magnitudeBand[19] * nf,
                      0,//magnitudeSum,
                      0//sensitivityFactor_
                      );*/
    // timeDeltaMicros);
}
