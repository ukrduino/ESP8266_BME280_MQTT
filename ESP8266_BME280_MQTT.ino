#include "Credentials.h"
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
Adafruit_BME280 bme; // I2C 
// Change sensor address in  I2C, 0x76 or 0x77, in Adafruit_BME280.h (http://playground.arduino.cc/Main/I2cScanner)


const char* mqtt_server = MQTT_SERVER_IP;
WiFiClient espClient;
unsigned long reconnectionPeriod = 60000; //miliseconds
unsigned long lastBrokerConnectionAttempt = 0;
unsigned long lastWifiConnectionAttempt = 0;
PubSubClient client(espClient);

long sensorRequestPeriod = 60000; //miliseconds
long lastSensorDataMsg = 0;
char msg[50];

void setup() {
	Serial.begin(115200);
	client.setServer(mqtt_server, 1883);
	client.setCallback(callback);
	setup_wifi();
	Wire.begin(D3, D4);
	delay(1000);
	if (!bme.begin()) {
		Serial.println("Could not find a valid BME280 sensor, check wiring!");
	}
}

void loop() {
	if (!client.connected()) {
		reconnectToBroker();
	}
	client.loop();
	sendTempHumidPressureToMqtt();
}

void reconnectToBroker() {
	long now = millis();
	if (now - lastBrokerConnectionAttempt > reconnectionPeriod) {
		lastBrokerConnectionAttempt = now;
		{
			if (WiFi.status() == WL_CONNECTED)
			{
				if (!client.connected()) {
					connectToBroker();
				}
			}
			else
			{
				reconnectWifi();
			}
		}
	}
}


//Connection to MQTT broker
void connectToBroker() {
	Serial.print("Attempting MQTT connection...");
	// Attempt to connect
	if (client.connect("WemosD1Mini_2")) {
		Serial.println("connected");
		// Once connected, publish an announcement...
		client.publish("WemosD1Mini_2/status", "WemosD1Mini_2 connected");
		// ... and resubscribe
		client.subscribe("WemosD1Mini_2/setSensorRequestPeriod");
	}
	else {
		Serial.print("failed, rc=");
		Serial.print(client.state());
		Serial.println(" try again in 5 seconds");
		// Wait 5 seconds before retrying
		delay(5000);
	}
}

void reconnectWifi() {
	long now = millis();
	if (now - lastWifiConnectionAttempt > reconnectionPeriod) {
		lastWifiConnectionAttempt = now;
		setup_wifi();
	}
}

void setup_wifi() {

	delay(500);
	// We start by connecting to a WiFi network
	int numberOfNetworks = WiFi.scanNetworks();

	for (int i = 0; i < numberOfNetworks; i++) {
		Serial.print("Network name: ");
		Serial.println(WiFi.SSID(i));
		if (WiFi.SSID(i).equals(SSID_1))
		{
			Serial.print("Connecting to ");
			Serial.println(SSID_1);
			WiFi.begin(SSID_1, PASSWORD_1);
			delay(1000);
			connectToBroker();
			return;
		}
		else if (WiFi.SSID(i).equals(SSID_2))
		{
			Serial.print("Connecting to ");
			Serial.println(SSID_2);
			WiFi.begin(SSID_2, PASSWORD_2);
			delay(1000);
			connectToBroker();
			return;
		}
		else if (WiFi.SSID(i).equals(SSID_3))
		{
			Serial.print("Connecting to ");
			Serial.println(SSID_3);
			WiFi.begin(SSID_3, PASSWORD_3);
			delay(1000);
			connectToBroker();
			return;
		}
		else
		{
			Serial.println("Can't connect to " + WiFi.SSID(i));
		}
	}
}

void callback(char* topic, byte* payload, unsigned int length) {
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");
	for (int i = 0; i < length; i++) {
		Serial.print((char)payload[i]);
	}
	Serial.println("-----");

	if (strcmp(topic, "WemosD1Mini_2/setSensorRequestPeriod") == 0) {
		String myString = String((char*)payload);
		sensorRequestPeriod = myString.toInt();
		Serial.println(sensorRequestPeriod);
	}
}


void sendTempHumidPressureToMqtt() {
	long now = millis();
	if (now - lastSensorDataMsg > sensorRequestPeriod) {
		lastSensorDataMsg = now;
		String temp = String(bme.readTemperature(), 0);
		Serial.print("Temperature = ");
		Serial.print(temp);
		Serial.println(" *C");
		String pressure = String(bme.readPressure() / 100.0F, 0);
		Serial.print("Pressure = ");
		Serial.print(pressure);
		Serial.println(" hPa");
		String humidity = String(bme.readHumidity(), 0);
		Serial.print("Humidity = ");
		Serial.print(humidity);
		Serial.println(" %");
		client.publish("WemosD1Mini_2/temp", temp.c_str());
		client.publish("WemosD1Mini_2/pressure", pressure.c_str());
		client.publish("WemosD1Mini_2/humidity", humidity.c_str());
	}
}
