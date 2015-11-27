#include <ESP8266WiFi.h>

#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_SI1145.h>
 
/************************* WiFi Access Point *********************************/
 
#define WLAN_SSID       "?????"
#define WLAN_PASS       "?????"
 
/************************* MQTT Broker Setup *********************************/
 
//const int MQTT_PORT = 1883;
//const char MQTT_SERVER[] PROGMEM    = "192.168.0.61";
//const char MQTT_CLIENTID[] PROGMEM  = "ESP-PUBLISHER-SERVICE";
//const char MQTT_USERNAME[] PROGMEM  = "********";
//const char MQTT_PASSWORD[] PROGMEM  = "********";

const int MQTT_PORT = 15948;
const char MQTT_SERVER[] PROGMEM    = "?????";
const char MQTT_CLIENTID[] PROGMEM  = "ESP-PUBLISHER-SERVICE";
const char MQTT_USERNAME[] PROGMEM  = "?????";
const char MQTT_PASSWORD[] PROGMEM  = "?????";
 
// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
 
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_PORT, MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD);
 
/****************************** Feeds ***************************************/
const char VISIBLE_LIGHT_FEED[] PROGMEM = "sweethome/sensors/outdoor/visible_light";
Adafruit_MQTT_Publish visible_light_feed = Adafruit_MQTT_Publish(&mqtt, VISIBLE_LIGHT_FEED);

const char UV_LIGHT_FEED[] PROGMEM = "sweethome/sensors/outdoor/uv_light";
Adafruit_MQTT_Publish uv_light_feed = Adafruit_MQTT_Publish(&mqtt, UV_LIGHT_FEED);

const char UV_INDEX_FEED[] PROGMEM = "sweethome/sensors/outdoor/uv_index";
Adafruit_MQTT_Publish uv_index_feed = Adafruit_MQTT_Publish(&mqtt, UV_INDEX_FEED);

Adafruit_SI1145 uv = Adafruit_SI1145();
 
/*************************** Sketch Code ************************************/
 
void setup() {
  Serial.begin(115200);
  delay(10);
 
  Serial.println("Sensor Test");

  if (!uv.begin()) {
    Serial.println("Didn't find Si1145");
    while (1);
  }
  else {
    Serial.println("Si1145 ready.");
  }
 
  // Connect to WiFi access point.
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
 
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
 
  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
}

void MQTT_connect() {
  int8_t ret;
 
  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }
 
  Serial.print("Connecting to MQTT... ");
 
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    switch (ret) {
      case 1: Serial.println("Wrong protocol"); break;
      case 2: Serial.println("ID rejected"); break;
      case 3: Serial.println("Server unavailable"); break;
      case 4: Serial.println("Bad user/password"); break;
      case 5: Serial.println("Not authenticated"); break;
      case 6: Serial.println("Failed to subscribe"); break;
      default: Serial.print("Couldn't connect to server, code: ");
        Serial.println(ret);
        break;
    }
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
  }
  Serial.println("MQTT Connected!");
}

void loop() {
  MQTT_connect();
  
  float uv_index_original = uv.readUV();
  float uv_index = uv_index_original / 100.0;
  uint16_t visible_light = uv.readVisible();
  uint16_t uv_light = uv.readIR();

  Serial.println("");
  
  Serial.print("Visible Light: "); Serial.println(visible_light);
  Serial.print("IR Light: "); Serial.println(uv_light);
  Serial.print("UV Index: ");  Serial.println(uv_index);

  Serial.println("");
  
  Serial.print(F("\nSending visible light measurement "));
  if (!visible_light_feed.publish(visible_light)) {
    Serial.print(F("Failed"));
  } else {
    Serial.print(F("OK!"));
  }
  
  Serial.print(F("\nSending uv light measurement "));
  if (!uv_light_feed.publish(uv_light)) {
    Serial.print(F("Failed"));
  } else {
    Serial.print(F("OK!"));
  }
  
  Serial.print(F("\nSending uv index measurement "));
  if (!uv_index_feed.publish(uv_index)) {
    Serial.print(F("Failed"));
  } else {
    Serial.print(F("OK!"));
  }

  Serial.println("");
  
  delay(5000);
}

