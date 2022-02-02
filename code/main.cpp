#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiClient.h>
#include <stdlib.h>
#include "structs.h"
#include <Hex_draw.h>
#include "Arduino.h"

const char* WIFI_SSID = "KT-NET"; // The SSID (name) of the Wi-Fi network you want to connect to
const char* WIFI_PASSWORD = "PASSWORD"; // The password of the Wi-Fi network
//const char* auth = ""; /The Blynk Authorization Token

//---my
const PROGMEM char* MQTT_CLIENT_ID = "esp8266";
const PROGMEM char* MQTT_SERVER_IP = "192.168.11.15";
const PROGMEM uint16_t MQTT_SERVER_PORT = 1883;
WiFiClient espClient;
PubSubClient client(espClient);

Hex_controller *hexController;
String topic = "";
String payload="";
String last_payload="";
int red = 100;
int green = 100; 
int blue = 0;
long t=0;

void callback(char* p_topic, byte* p_payload, unsigned int p_length) {
  payload = "";
  topic = "";
  // concat the payload into a string
  Serial.print("Message arrived in topic: ");
  int top_size = sizeof(p_topic) / sizeof(char);
  for (int i = 0; i <= top_size; i++) {
    topic += p_topic[i];
  }

  Serial.println(topic);

  for (uint8_t i = 0; i < p_length; i++) {
    payload += (char)p_payload[i];
  }
  // int x = payload.toInt();
  Serial.print("payload: ");
  Serial.println(payload);
}

void setup() {
  delay( 2000 ); // power-up safety delay
  Serial.println("> Setup.");
  hexController = new Hex_controller();
  Serial.begin(115200);
  hexController->set_serial(Serial);
  hexController->calculate_outer_leds();
  hexController->create_outer_path();
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  client.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
  client.setCallback(callback);
}

void reconnect() {
  Serial.print("INFO: Attempting MQTT connection...");
  // Attempt to connect
  if (client.connect(MQTT_CLIENT_ID)) {
    Serial.println("INFO: connected");
    client.subscribe("chmod");
    client.subscribe("cone");
    client.subscribe("ctwo");
    client.subscribe("ctre");
    client.subscribe("cfor");
    client.subscribe("pr_st");
    client.subscribe("speed");
    client.subscribe("shine");
    client.subscribe("info");
    client.subscribe("fade");
    client.subscribe("rainb");
    client.subscribe("stop");
    // Once connected, publish an announcement...
    Serial.println("INFO: connected");
    // ... and resubscribe
  } else {
    Serial.print("ERROR: failed, rc=");
    Serial.print(client.state());
    Serial.println("DEBUG: try again in 5 seconds");
    // Wait 5 seconds before retrying
  }
}
int hexcolorToInt(char upper, char lower)
{
  int uVal = (int)upper;
  int lVal = (int)lower;
  uVal = uVal > 64 ? uVal - 55 : uVal - 48;
  uVal = uVal << 4;
  lVal = lVal > 64 ? lVal - 55 : lVal - 48;
  //  Serial.println(uVal+lVal);
  return uVal + lVal;
}  
//for testing 
//int i=0;  
void loop() {
 // hexController->test();
 // delay(10000);

  if(payload!=last_payload){
    Serial.print("new topic is: ");
    Serial.println(topic);
    if(payload.startsWith("#")){
      char c[7];
      payload.toCharArray(c, 8);
      red = hexcolorToInt(c[1], c[2]);
      green = hexcolorToInt(c[3], c[4]);
      blue = hexcolorToInt(c[5], c[6]);
    }
    if(topic == "chmod"){ 
      int pinValue =payload.toInt(); // Mode
      Serial.print("changing mode to: ");
      Serial.println(pinValue);
      hexController->change_mode(pinValue);
    }else if (topic == "pr_st") {
      int pinValue =payload.toInt();//preset animation 
      Serial.print("Preset_animation_set to: ");
      Serial.println(pinValue);
      hexController->set_pre_anim(pinValue);
    }else if(topic == "cone"){
      hexController->set_color(CRGB(red,green,blue),0);
      Serial.print("color one set");
    }else if(topic == "ctwo"){
      hexController->set_color(CRGB(red,green,blue),1);
    }else if(topic == "ctre"){
      hexController->set_color(CRGB(red,green,blue),2);
    }else if(topic == "cfor"){
      hexController->set_color(CRGB(red,green,blue),3);
    }else if(topic == "speed"){
      int speedVal = payload.toInt();
      hexController->set_speed(speedVal);
    }else if(topic == "shine"){
      uint8_t b = payload.toInt();
      hexController->set_brigtness(b);
    }else if(topic == "fade"){
      hexController->set_fade(payload.toInt());
    }else if(topic == "rainb"){
      Serial.println("chaning Rainbow");
      hexController->set_rainbow(payload.toInt());
    }else if(topic == "stop"){
      hexController->pause_play(payload.toInt());
    }
    last_payload = payload;
  }
  if ( !client.connected()) {

    reconnect(); //Reconnect MQTT if it fails
  }
  client.loop();
  hexController->update();
}
