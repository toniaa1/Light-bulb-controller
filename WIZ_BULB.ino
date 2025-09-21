#define BLYNK_TEMPLATE_ID "TMPxxxxxx"
#define BLYNK_TEMPLATE_NAME "Device"
#define BLYNK_AUTH_TOKEN "aQhReBpT5WbqFph1JllQ2RSPAUga5qtG"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiUdp.h>
#include <BlynkSimpleEsp32.h>
int tempr;
int intens;
int buttonPin=21;
int buttonState = HIGH;    
int lastButtonState = HIGH; 
unsigned long lastDebounceTime = 0; 
unsigned long debounceDelay = 10;
int currentHourIndex = 0; 
int hr[]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24};
int nr_hrs=24;
int hour=1;

const int sensorPin = 34;
int sensorValue = 0;
const char* ssid = "pppaaa";
const char* password = "12345678";
const char* bulbIP = "192.168.208.86"; // Replace with your bulb's IP address
const int bulbPort = 38899;
WiFiUDP udp;

int gauge1 = 0; // Variable for Gauge 1 (dimming)
int gauge2 = 0; // Variable for Gauge 2 (temperature)

BLYNK_WRITE(V0) { // Handle Gauge 1 on Virtual Pin V0
    gauge1 = param.asInt(); // Read integer value from Blynk
    Serial.println("Gauge 1 updated:");
    Serial.println(gauge1);
    sendCommand(gauge1, gauge2, true); // Send updated values
}

BLYNK_WRITE(V1) { // Handle Gauge 2 on Virtual Pin V1
    gauge2 = param.asInt(); // Read integer value from Blynk
    Serial.println("Gauge 2 updated:");
    Serial.println(gauge2);
    sendCommand(gauge1, gauge2, true); // Send updated values
}

void setup() {
    pinMode(sensorPin, INPUT);
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi!");

    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
}

bool sendCommand(const char* command) {
    if (!udp.beginPacket(bulbIP, bulbPort)) {
        Serial.println("Failed to start UDP packet.");
        return false;
    }
    udp.write((const uint8_t*)command, strlen(command));
    if (!udp.endPacket()) {
        Serial.println("Failed to send UDP packet.");
        return false;
    }
    Serial.println("Command sent: ");
    Serial.println(command);
    return true;
}

void listenForResponse() {
    int packetSize = udp.parsePacket();
    if (packetSize) {
        char buffer[512];
        int len = udp.read(buffer, 512);
        if (len > 0) {
            buffer[len] = '\0'; // Null-terminate the string
            Serial.println("Received response: ");
            Serial.println(buffer);
        }
    }
}

void sendCommand(int dimming, int temperature, bool lit) {
    char command[256];
    snprintf(command, sizeof(command),
             "{\"method\": \"setPilot\", \"params\": {\"state\": %s, \"dimming\": %d, \"temp\": %d}}",
             lit ? "true" : "false", dimming, temperature);
    sendCommand(command);
    listenForResponse();
}

void ensureWiFiConnected() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi lost. Reconnecting...");
        WiFi.disconnect();
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
            delay(1000);
            Serial.println("Reconnecting to WiFi...");
        }
        Serial.println("Reconnected to WiFi!");
    }
}

void hrcalc()
{
  int reading = digitalRead(buttonPin);
    
  // Check if the button state has changed
  if (reading != lastButtonState) {
    lastDebounceTime = millis(); // Reset debounce timer
  }

  // If the state is stable and debounce delay has passed
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading == LOW && buttonState == HIGH) { // Button pressed
      currentHourIndex = (currentHourIndex + 1) % nr_hrs; // Increment hour index and wrap around
      Serial.print("Hour changed to: ");
      Serial.println(hr[currentHourIndex]); // Print the current hour
    }
    buttonState = reading; // Update button state
  }

  lastButtonState = reading;
  hour=hr[currentHourIndex];
}


void transform(int preferred_temp, int preferred_intens)
{
sensorValue = analogRead(sensorPin);



double x=(double)sensorValue;
  x=(double)x*100/4095;

  intens=100-x;


  if (hour>=6 && hour<8)
    tempr=4000;
    else if (hour>=8 && hour<12)
    tempr=6000;
    else if (hour>=12 && hour<14)
    tempr=6500;
    else if (hour>=14 && hour<17)
    tempr=5000;
    else if (hour>=17 && hour<20)
    tempr=4000;
    else if (hour>=20 && hour<22)
    tempr=3000;
    else
    tempr=2300;


 if (hour>=6 && hour<8)
        if (intens<=15)
          intens=20;
    else if (hour>=8 && hour<12)
          if (intens<=25)
              intens=30;
    else if (hour>=12 && hour<14)
        if (intens<=35)
            intens=40;
    else if (hour>=14 && hour<17)
          if (intens<=25)
              intens=30;
   
}

void loop() {
    ensureWiFiConnected();
    Blynk.run();

    // Read and process the sensor value
    sensorValue = analogRead(sensorPin);
    sensorValue = 4095 - sensorValue; // Invert sensor value

    // Send the sensor value to Blynk (e.g., on Virtual Pin V2)
    Blynk.virtualWrite(V2, sensorValue);

    Serial.print("Sensor Value: ");
    Serial.println(sensorValue);

    delay(1000); 
}
