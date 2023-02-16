#include <WiFi.h>
#include <PubSubClient.h>
#include "max6675.h"
#include <ArduinoJson.h>

//Wifi Details
//Fill in your details in between the quotes ""

const char* ssid = "<WiFi SSID>";
const char* password = "<WiFi Password>";



//MQTT server configuration
const char* mqtt_server = "<MQTT Server address>";
const char* mqtt_user = "<MQTT Server username>";
const char* mqtt_pass = "<MQTT Server password>";

//MQTT Topic initialization
const char* pub_topic = "tele/temp/SENSOR"; // Publication topic
const char* sub_topic = "cmd/temp/SENSOR";  // Subscription topic
float tele_rate = 2;  //Telemetry rate in seconds

//thermocouple configuration
const int thermoDO = 12;  //Data Out aka MISO; SPI data bus is from the perpective of master chip
const int thermoCLK = 14; //Clock Signal
const int thermoCS1 = 15;  //Chip Select for Thermocouple 1
const int thermoCS2 = 5;  //Chip Select for Thermocouple 2
const int thermoCS3 = 13;  //Chip Select for Thermocouple 3
const int thermoCS4 = 12;  //Chip Select for Thermocouple 4

//thermocouples initialization
MAX6675 thermocouple1(thermoCLK, thermoCS1, thermoDO);
MAX6675 thermocouple2(thermoCLK, thermoCS2, thermoDO);
MAX6675 thermocouple3(thermoCLK, thermoCS3, thermoDO);
MAX6675 thermocouple4(thermoCLK, thermoCS4, thermoDO);

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;  //timestamp for lastmsg broadcast over MQTT
#define MSG_BUFFER_SIZE  (64)
char msg[MSG_BUFFER_SIZE];

// allocate the memory for the document
StaticJsonDocument<64> doc;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println("Attempting to connect to Wifi");
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "Temperature Logger";
    // Attempt to connect
    if (client.connect(clientId.c_str(),mqtt_user,mqtt_pass)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  delay(500);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > (tele_rate*1000)) {
    lastMsg = now;
    
    doc["therm1"] = thermocouple1.readCelsius();
    doc["therm2"] = thermocouple2.readCelsius();
    doc["therm3"] = thermocouple3.readCelsius();
    doc["therm4"] = thermocouple4.readCelsius();
    serializeJson(doc,msg);
    client.publish(pub_topic, msg);
    }
}
