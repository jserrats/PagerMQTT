//#include "secrets_juliet.h"
#include "secrets_sierra.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include "EasyBuzzer.h"

#define PIN_LED 5
#define PIN_BUZZER 23
#define PIN_BUTTON 22

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);

void nuke_buzz()
{
  for (int i = 880; i > 440; i--)
  {
    EasyBuzzer.beep(i);
    delay(10);
  }
  EasyBuzzer.beep(50);
  delay(1000);
  EasyBuzzer.stopBeep();
}

void init_buzz()
{
  EasyBuzzer.beep(440);
  delay(150);
  EasyBuzzer.beep(554);
  delay(150);
  EasyBuzzer.beep(659);
  delay(150);
  EasyBuzzer.stopBeep();
}

void short_buzz()
{
  EasyBuzzer.beep(440);
  delay(250);
  EasyBuzzer.stopBeep();
}

void buzz()
{
  EasyBuzzer.beep(440);
  delay(500);
  EasyBuzzer.beep(880);
  delay(1000);
  EasyBuzzer.stopBeep();
}

void chord_buzz()
{
  EasyBuzzer.beep(440);
  delay(250);
  EasyBuzzer.beep(554);
  delay(250);
  EasyBuzzer.beep(659);
  delay(250);
  EasyBuzzer.stopBeep();
}

void short_led()
{
  digitalWrite(PIN_LED, HIGH);
  delay(500);
  digitalWrite(PIN_LED, LOW);
}

void led()
{
  digitalWrite(PIN_LED, HIGH);
  delay(1000);
  digitalWrite(PIN_LED, LOW);
  delay(1000);
  digitalWrite(PIN_LED, HIGH);
  delay(1000);
  digitalWrite(PIN_LED, LOW);
}

void publish(char *command)
{
  StaticJsonDocument<200> doc;
  doc["command"] = command; //"short_buzz";
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
  Serial.println(F("Message published"));
}

void messageHandler(String &topic, String &payload)
{
  Serial.println("incoming: " + topic + " - " + payload);
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (error)
    Serial.println(F("Failed to read file, using default configuration"));

  // Copy values from the JsonDocument to the Config
  if (doc["command"] == "short")
  {
    short_buzz();
    short_led();
  }
  if (doc["command"] == "buzz")
  {
    buzz();
    led();
  }
  if (doc["command"] == "chord")
  {
    chord_buzz();
    led();
  }
  if (doc["command"] == "nuke")
  {
    nuke_buzz();
    led();
  }
}

void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.begin(AWS_IOT_ENDPOINT, 8883, net);

  // Create a message handler
  client.onMessage(messageHandler);

  Serial.print("Connecting to AWS IOT");

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
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  Serial.println("AWS IoT Connected!");
}

void setup()
{
  Serial.begin(9600);
  connectAWS();
  EasyBuzzer.setPin(PIN_BUZZER);
  pinMode(PIN_BUTTON, INPUT_PULLDOWN);
  pinMode(PIN_LED, OUTPUT);
  init_buzz();
  short_led();
}

void loop()
{
  if (digitalRead(PIN_BUTTON) == HIGH)
  {
    delay(400);
    if (digitalRead(PIN_BUTTON) == HIGH)
    {
      //long press
      publish("buzz");
    }
    else
    {
      //short press
      publish("chord");
    }
    short_buzz();
    short_led();
  }
  //
  client.loop();
  // delay(1000);
}