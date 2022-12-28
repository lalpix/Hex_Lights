
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
    char msg[length+1];
    for (int i = 0; i < length;i++){
        msg[i] = (char)payload[i];
    }
    msg[length] = '\0';

    Serial.println(msg);
    if(strstr(topic,"Mode")){
        hexController->change_mode((Mode)atoi((char*)payload));
        Mode m = (Mode)atoi((char *)payload);
        Serial.printf("Mode is now %d\n", m);
    }
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

    Serial.println("Connecting_to_AWS_IOT");

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
    client.subscribe("Mode");


    Serial.println("AWS_IoT_Connected!");
}

void setup()
{
    
    Serial.begin(115200);
    delay(2000); // power-up safety delay
    Serial.println("> Setup....");
    WifiSetup();
    setupI2Smic();
    setupSpectrumAnalysis();
    
    hexController = new Hex_controller();
    hexController->set_serial(Serial);
    hexController->init();

    hexController->change_mode(AudioFreqPool);
    hexController->set_rainbow(1);
    hexController->set_fade(true);
    hexController->set_speed(1);
    Serial.println("Setup DONE");

    delay(2000);
}

void loop()
{
    // delay( 5000 );
    client.loop();
    hexController->update();
    /*
    float magnitudeBand[FREQ_BAND_COUNT] = {0.0f};

    float magnitudeBandWeightedMax = 0.0f;

    newAudioReading(magnitudeBand, &magnitudeBandWeightedMax);

    // ----- Beat detection -----

    // Maintain history of last three magnitude values of the bass band
    beatHist_[kBeatDetectBandOffset][0] = beatHist_[kBeatDetectBandOffset][1];
    beatHist_[kBeatDetectBandOffset][1] = beatHist_[kBeatDetectBandOffset][2];
    beatHist_[kBeatDetectBandOffset][2] = magnitudeBand[kBeatDetectBandOffset] * kFreqBandAmp[kBeatDetectBandOffset] * sensitivityFactor_;

    float diff1 = beatHist_[1] - beatHist_[0];
    float diff2 = beatHist_[2] - beatHist_[1];
    if (((diff1 >= kBeatThreshold) && (diff2 < 0)) || ((diff1 > 0) && (diff2 <= -kBeatThreshold)))
    {
        beatVisIntensity_[1] = 250;
    }
    else
    {
        if (beatVisIntensity_[1] >= 25)
            beatVisIntensity_[1] -= 25;
    }
    // Detect magnitude peak
   
     // ----- Update the Led strip -----
    CRGB tmp=CRGB::Black;
    hexController->fill_all_hex(tmp.setHSV(250,255, beatVisIntensity_[1]));
    */
    /*
    if (kNumLeds <= 2 * FREQ_BAND_COUNT + 4)
    {
        // Show beat detection at the beginning of the strip
        const uint8_t numBassLeds = (kNumLeds - FREQ_BAND_COUNT) / 2;

        for (int i = 0; i < numBassLeds; i++)
        {
            ledStrip_[i].setHSV(250, 255, beatVisIntensity_[1]);
        }

        // Show frequency intensities on the remaining Leds
        const uint8_t colorStart = 30;
        const uint8_t colorEnd = 210;
        const uint8_t colorStep = (colorEnd - colorStart) / FREQ_BAND_COUNT;

        for (int k = 0; k < FREQ_BAND_COUNT; k++)
        {
            uint8_t color = colorStart + k * colorStep;
            uint8_t lightness = min(int(magnitudeBand[k] * kFreqBandAmp[k] * sensitivityFactor_), 255);

            ledStrip_[k + numBassLeds].setHSV(color, 255, lightness);
        }

        // Show beat detection at the end of the strip
        for (int i = numBassLeds + FREQ_BAND_COUNT; i < kNumLeds; i++)
        {
            ledStrip_[i].setHSV(250, 255, beatVisIntensity_[1]);
        }
        }
        else
        {
            // Show beat detection at the beginning of the strip
            const uint8_t numBassLeds = (kNumLeds - 2 * FREQ_BAND_COUNT) / 2;

            for (int i = 0; i < numBassLeds; i++)
            {
                ledStrip_[i].setHSV( 250, 255, beatVisIntensity_ );
            }

            // Show frequency intensities on the remaining Leds
            const uint8_t colorStart = 30;
            const uint8_t colorEnd   = 210;
            const uint8_t colorStep = (colorEnd - colorStart) / FREQ_BAND_COUNT;

            for (int k = 0; k < FREQ_BAND_COUNT; k++)
            {
                uint8_t color = colorStart + k * colorStep;
                uint8_t lightness = min( int(magnitudeBand[k] * kFreqBandAmp[k] * sensitivityFactor_), 255);

                ledStrip_[numBassLeds + k].setHSV(color, 255, lightness);

                ledStrip_[numBassLeds + 2 * FREQ_BAND_COUNT - k - 1].setHSV(color, 255, lightness);
            }

            // Show beat detection at the beginning of the strip
            for (int i = numBassLeds + 2 * FREQ_BAND_COUNT; i < kNumLeds; i++)
            {
                ledStrip_[i].setHSV( 250, 255, beatVisIntensity_ );
            }
        }*/

        FastLED.show();
        
    
   /*float nf;

    if (fabs(magnitudeBand[1]) < 0.001f)
    {
        nf = 1.0f;
        }
        else
        {
            nf = 1.0f / magnitudeBand[1];
        }
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
}
