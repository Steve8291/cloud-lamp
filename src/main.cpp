#include <Arduino.h>
#include <FastLED.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h> 
#include <LittleFS.h>


#define NUM_LEDS 60        // Number of LEDs in your strip
#define DATA_PIN 16         // Data pin connected to WS2812B
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define BRIGHTNESS 200     // Set overall brightness (0-255)

CRGB leds[NUM_LEDS];

// touch code
const int touchPin = 4; 
const int LED1 = 16;
const int threshold = 20;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 1000;
int touchValue;
int lampState = LOW;
int buttonState;
static uint8_t Shue = 0;
int lampMode = 1; // 1: thunderstorm, 2: breathing, 3: rainbow, 4: meteor


// Change the ssid to your preferred WiFi network name
// Password not needed if you want the AP (Access Point) to be open
const char* ssid = "Arduino-ESP32";
const char* password = "88888888";

// Configure an IP address for the Access Point
IPAddress ap_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress  subnet(255, 255, 255, 0);


AsyncWebServer server(80); // AsyncWebServer object on port 80
AsyncWebSocket ws("/ws"); // WebSocket server at /ws

void notifyClients() {
  String jsonMessage = "{\"id\":" + String(lampMode) + "}";
  ws.textAll(jsonMessage);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  JsonDocument receivedJson; // Json object to hold received data from clients
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
  data[len] = 0;
  String message = (char*)data;
  Serial.println(message);

  deserializeJson(receivedJson, message);

  lampMode = receivedJson["id"];

  notifyClients();
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    switch (type) {
    case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        notifyClients(); // Send initial states to newly connected client
        break;
    case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
    case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);
        break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
        break;
    }
}


void initWebSocket() {
    ws.onEvent(onEvent);
    server.addHandler(&ws);

    // Serve static UI files (index.html, script.js, style.css) from LittleFS.
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    server.begin();
}

// Initialize Wi-Fi Access Point and Web Server
void initAP() {
    Serial.print("Setting AP (Access Point)…");
    WiFi.mode(WIFI_AP);
    // Remove the password parameter, if you want the AP (Access Point) to be open
    WiFi.softAP(ssid, password);
    if (!WiFi.softAPConfig(ap_ip, gateway, subnet)) {
        Serial.println("AP configuration failed");
    } else {
        // Print ESP IP Address
        IPAddress IP = WiFi.softAPIP();
        Serial.println("AP configuration successful");
        Serial.print("AP IP address: ");
        Serial.println(IP);
    }
}

  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
  data[len] = 0;
  String message = (char*)data;
  Serial.println(message);

  deserializeJson(receivedJson, message);

  lampMode = receivedJson["id"];

  notifyClients();
  }
void breathing() {

  uint8_t bri = beatsin8(15, 0, 255); 
  fill_solid(leds, NUM_LEDS, CHSV(32, 200, bri)); 
  FastLED.show();
}

void rainbowFlow(uint8_t hue) {
  
  fill_rainbow(leds, NUM_LEDS, hue, 7);
  Shue++;
  fadeToBlackBy(leds, NUM_LEDS, 50);
  FastLED.show();
}

void meteor() {
  static int pos = 0;
  fadeToBlackBy(leds, NUM_LEDS, 60);
  leds[pos] = CRGB::Cyan; 
  pos++;
  if (pos >= NUM_LEDS) pos = 0;
  FastLED.show();
  delay(30); 
}

void thunderstorm() {
  // Randomly decide to create a flash
  if (random(100) < 10) { // 10% chance per cycle to start a flash

    // Randomize number of flashes
    int flashes = random(2, 5); 
    
    for (int i = 0; i < flashes; i++) {
      // Randomize which part of the strip lights up
      int startLed = random(0, NUM_LEDS / 2);
      int endLed = random(startLed, NUM_LEDS);
      
      // Choose white or slight blue color
      CRGB color = (random(0, 5) > 3) ? CRGB(100, 100, 255) : CRGB::White;
      
      // Light up the segment
      fill_solid(&leds[startLed], endLed - startLed, color);
      FastLED.show();
      
      delay(random(10, 50)); // Fast flash duration
      
      // Turn off
      fill_solid(&leds[startLed], endLed - startLed, CRGB::Black);
      FastLED.show();
      
      delay(random(10, 100)); // Dark time between flashes
    }
  }
  // Keep base scene very dark blue or off
  fadeToBlackBy(leds, NUM_LEDS, 50);
  FastLED.show();

}

void updateLEDs(int touchValue){
  Serial.print("Value: ");
  Serial.println(touchValue);
  if (touchValue >= 100 && touchValue <= 180){

    if (lampState == LOW){
      lampState = HIGH;
    } else {
      lampState = LOW;
    }
  
    Serial.print(" - lampState: ");
    Serial.println(lampState);
    lastDebounceTime = millis();

  }
}

bool debounceReady() {
  if ((millis() - lastDebounceTime) > debounceDelay) {
    return true;
  } else {
    return false;
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000); // give me time to bring up serial monitor
  // initialize the LED pin as an output:
  pinMode (LED1, OUTPUT);

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  if (!LittleFS.begin(true)) {
      Serial.println("LittleFS mount failed");
  }

  initAP();
  initWebSocket();
}

void loop() {
  ws.cleanupClients();

  touchValue = touchRead(touchPin);
  if ( debounceReady()){
  updateLEDs(touchValue);
  }

  
  if (lampState == HIGH){

    switch (lampMode) {
            case 1:
                thunderstorm();
                break;
            case 2:
                breathing();
                break;
            case 3:
                rainbowFlow(Shue);
                break;
            case 4:
                meteor();
                break;
    }
    //Randomly decide to create a flash
    // if (random(100) < 10) { // 10% chance per cycle to start a flash
    //   thunderstorm();
    //   delay(100);
    // breathing();
    //   delay(500);
      //rainbowFlow(Shue);
    //   delay(500);
    //   meteor();
    // }

    // //Keep base scene very dark blue or off
    // fadeToBlackBy(leds, NUM_LEDS, 50);
    // FastLED.show();
    }else {
      fadeToBlackBy(leds, NUM_LEDS, 50);
      FastLED.show();
    }
 delay(50);
}

