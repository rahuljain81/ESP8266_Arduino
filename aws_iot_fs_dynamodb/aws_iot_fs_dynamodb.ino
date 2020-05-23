//https://electronicsinnovation.com/storing-esp8266-data-into-amazon-dynamodb-using-aws-iot-coremqtt-arduino/
/*Developed by M V Subrahmanyam - https://www.linkedin.com/in/veera-subrahmanyam-mediboina-b63997145/
Project: AWS | NodeMCU ESP32 Tutorials
Electronics Innovation - www.electronicsinnovation.com

GitHub - https://github.com/VeeruSubbuAmi
YouTube - http://bit.ly/Electronics_Innovation

Upload date: 07 October 2019

AWS Iot Core

This example needs https://github.com/esp8266/arduino-esp8266fs-plugin

It connects to AWS IoT server then:
- publishes "hello world" to the topic "outTopic" every two seconds
- subscribes to the topic "inTopic", printing out any messages
*/

#include "FS.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Update these with values suitable for your network.

const char* ssid = "RAHUL";
const char* password = "12345678";

#define SUBSCRIBE "ESP8266_IN"
#define PUBLISH "ESP8266_OUT"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

const char* AWS_endpoint = "a3psj0keptsnuf-ats.iot.ap-south-1.amazonaws.com"; //MQTT broker ip

//==========================================================================
#define BUFFER_LEN 256
long lastMsg = 0;
char msg[BUFFER_LEN];
int value = 0;
byte mac[6];
char mac_Id[18];
//===========================================================================

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

WiFiClientSecure espClient;
PubSubClient client(AWS_endpoint, 8883, callback, espClient); //set MQTT port number to 8883 as per //standard

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  espClient.setBufferSizes(512, 512);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();
  while(!timeClient.update()){
    timeClient.forceUpdate();
  }

  espClient.setX509Time(timeClient.getEpochTime());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESPthing")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(PUBLISH, "hello world");
      // ... and resubscribe
    client.subscribe(SUBSCRIBE);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");

      char buf[256];
      espClient.getLastSSLError(buf,256);
      Serial.print("WiFiClientSecure SSL error: ");
      Serial.println(buf);

      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {

  Serial.begin(115200);
  Serial.setDebugOutput(true);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  setup_wifi();
 
  delay(1000);
  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }

  Serial.print("Heap: "); 
  Serial.println(ESP.getFreeHeap());

  // Load certificate file
  File cert = SPIFFS.open("/fcb29a4f29-certificate.pem.crt", "r"); //replace cert.crt eith your uploaded file name
  if (!cert) {
    Serial.println("Failed to open cert file");
  }
  else
    Serial.println("Success to open cert file");

  delay(1000);

  if (espClient.loadCertificate(cert))
    Serial.println("cert loaded");
  else
    Serial.println("cert not loaded");

  // Load private key file
  File private_key = SPIFFS.open("/fcb29a4f29-private.pem.key", "r"); //replace private eith your uploaded file name
  if (!private_key) {
    Serial.println("Failed to open private cert file");
  }
  else
    Serial.println("Success to open private cert file");

  delay(1000);

  if (espClient.loadPrivateKey(private_key))
  Serial.println("private key loaded");
  else
  Serial.println("private key not loaded");

  // Load CA file
  File ca = SPIFFS.open("/AmazonRootCA1.pem", "r"); //replace ca eith your uploaded file name
  if (!ca) {
  Serial.println("Failed to open ca ");
  }
  else
  Serial.println("Success to open ca");

  delay(1000);

  if(espClient.loadCACert(ca))
  Serial.println("ca loaded");
  else
  Serial.println("ca failed");

  Serial.print("Heap: "); Serial.println(ESP.getFreeHeap());
  
  //==========================================================================
  WiFi.macAddress(mac);
  snprintf(mac_Id, sizeof(mac_Id), "%02x:%02x:%02x:%02x:%02x:%02x",
  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print(mac_Id);
  //============================================================================
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    //========================================================================
    String macIdStr = mac_Id;
    uint8_t randomNumber = random(20, 50);
    String randomString = String(random(0xffff), HEX);
    snprintf (msg, BUFFER_LEN, "{\"mac_Id\" : \"%s\", \"random_number\" : %d, \"random_string\" : \"%s\"}", macIdStr.c_str(), randomNumber, randomString.c_str());
    
    Serial.print("Publish message: ");
    Serial.println(msg);
    //mqttClient.publish("outTopic", msg);
    client.publish(PUBLISH, msg);
    //=========================================================================
    //Serial.print("Heap: "); 
    //Serial.println(ESP.getFreeHeap()); //Low heap can cause problems
  }
  digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
  delay(100); // wait for a second
  digitalWrite(LED_BUILTIN, LOW); // turn the LED off by making the voltage LOW
  delay(100); // wait for a second
}
