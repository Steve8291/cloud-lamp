// http://cloudlamp.local
// default ip 192.168.4.1

#include <Arduino.h>
#include <FastLED.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <LittleFS.h>


#define NUM_LEDS 60        // Number of LEDs in your strip
#define DATA_PIN 16         // Data pin connected to WS2812B
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define BRIGHTNESS 200     // Set overall brightness (0-255)




// touch code
const int BOOT_BUTTON_PIN = 0; // Built-in BOOT button on the ESP32
const int touchPin = 4;
const int threshold = 20;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 1000;
int touchValue;
int lampState = LOW;
int buttonState;

const int touchLowThreshold = 143; // Adjust this threshold based on your touch sensitivity needs
const int touchHighThreshold = 214; // Adjust this threshold based on your touch sensitivity needs
int lampMode = 2; // 1: thunderstorm, 2: breathing, 3: rainbow, 4: meteor


CRGB leds[NUM_LEDS];
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

void onWiFiDisconnect(WiFiEvent_t event, WiFiEventInfo_t info) {
    Serial.println("Disconnected from WiFi! Router may be down.");
    // The ESP32 will attempt to reconnect asynchronously without blocking your loop
    WiFi.reconnect(); 
}

void initWebSocket() {
    ws.onEvent(onEvent);
    server.addHandler(&ws);
    // Serve static UI files (index.html, script.js, style.css) from LittleFS.
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    server.begin();
}

void breathing() {
    static unsigned long lastBreathTime = 0;
    if (millis() - lastBreathTime >= 20) {
        lastBreathTime = millis();
        uint8_t bri = beatsin8(15, 0, 255);
        fill_solid(leds, NUM_LEDS, CHSV(32, 200, bri));
        FastLED.show();
    }
}

void rainbowFlow() {
    static unsigned long lastRainbowTime = 0;
    static uint8_t hue = 0;
    
    // Draw a frame every 50ms
    if (millis() - lastRainbowTime >= 50) {
        lastRainbowTime = millis();
        fill_rainbow(leds, NUM_LEDS, hue, 7);
        hue++;
        fadeToBlackBy(leds, NUM_LEDS, 50);
        FastLED.show();
    }
}

void meteor() {
    static unsigned long lastMeteorTime = 0;
    static int pos = 0;

    // Only draw a new frame every 30 milliseconds
    if (millis() - lastMeteorTime >= 30) {
        lastMeteorTime = millis();

        fadeToBlackBy(leds, NUM_LEDS, 60);
        leds[pos] = CRGB::Cyan;
        pos++;
        if (pos >= NUM_LEDS) pos = 0;
        
        FastLED.show();
    }
}

void thunderstorm() {
    static uint8_t state = 0;             // 0: Idle/Dark, 1: Flash On, 2: Flash Off
    static unsigned long lastActionTime = 0; 
    static unsigned long waitTime = 50;   // Time to wait before next action
    static int flashesRemaining = 0;
    static int startLed = 0;
    static int endLed = 0;
    static CRGB strikeColor;

    // The base fade effect happens continuously every 50ms
    static unsigned long lastFadeTime = 0;
    if (millis() - lastFadeTime >= 50) {
        lastFadeTime = millis();
        if (state == 0) { 
            fadeToBlackBy(leds, NUM_LEDS, 50);
            FastLED.show();
        }
    }

    // Check if it is time to move to the next phase of the lightning strike
    if (millis() - lastActionTime >= waitTime) {
        lastActionTime = millis();

        switch (state) {
            case 0: // IDLE: Waiting for a chance to strike
                if (random(100) < 5) { // 5% chance to start a storm
                    flashesRemaining = random(2, 5);
                    startLed = random(0, NUM_LEDS / 2);
                    endLed = random(startLed + 1, NUM_LEDS); // Ensure end is after start
                    strikeColor = (random(0, 5) > 3) ? CRGB(100, 100, 255) : CRGB::White;
                    
                    state = 1;      // Move to the "Flash On" state
                    waitTime = 0;   // Trigger immediately
                } else {
                    waitTime = 50;  // Check again in 50ms
                }
                break;

            case 1: // FLASH ON: Light up the segment
                fill_solid(&leds[startLed], endLed - startLed, strikeColor);
                FastLED.show();
                waitTime = random(10, 50); // Stay bright for 10-50ms
                state = 2;                 // Move to the "Flash Off" state
                break;

            case 2: // FLASH OFF: Go dark between flashes in the same strike
                fill_solid(&leds[startLed], endLed - startLed, CRGB::Black);
                FastLED.show();
                flashesRemaining--;

                if (flashesRemaining > 0) {
                    waitTime = random(10, 100); // Stay dark for 10-100ms
                    state = 1;                  // Go back to "Flash On"
                } else {
                    waitTime = random(100, 500); // Cooldown before next possible strike
                    state = 0;                   // Go back to "Idle"
                }
                break;
        }
    }
}


void updateLEDs(int touchValue) {
    static bool wasTouched = false;
    static unsigned long consecutiveTouchStart = 0;
    static bool isActivelyTouching = false;
    
    static float baseline = 255.0; 
    static unsigned long lastBaselineUpdate = 0; // NEW: Speed limit tracker for the baseline

    static unsigned long lastPrintTime = 0;
    if (millis() - lastPrintTime > 500) {
        Serial.print("Touch: ");
        Serial.print(touchValue);
        Serial.print(" | Baseline: ");
        Serial.println(baseline);
        lastPrintTime = millis();
    }

    // 1. HARD LOCKOUT: Force baseline to track perfectly while settling
    if (millis() - lastDebounceTime < debounceDelay) {
        baseline = touchValue; 
        return; 
    }

    // 2. DYNAMIC THRESHOLD: Lowered to 25. The ON state range is compressed, 
    // so touches don't cause as deep of a numerical drop.
    bool isCurrentlyTouched = (touchValue < (baseline - 25));

    // 3. ADAPT THE BASELINE (Speed-limited)
    if (!isCurrentlyTouched) {
        // NEW: Only allow the baseline to adapt once every 20 milliseconds. 
        // This prevents the baseline from instantly "swallowing" a light touch.
        if (millis() - lastBaselineUpdate > 20) {
            baseline = (baseline * 0.95) + (touchValue * 0.05);
            lastBaselineUpdate = millis();
        }
        
        isActivelyTouching = false;
        wasTouched = false;
    }

    // 4. TRUE DEBOUNCE
    if (isCurrentlyTouched) {
        if (!isActivelyTouching) {
            consecutiveTouchStart = millis();
            isActivelyTouching = true;
        } else if (millis() - consecutiveTouchStart > 50) { 
            if (!wasTouched) {
                lampState = (lampState == LOW) ? HIGH : LOW;

                Serial.print("Valid touch! New lampState: ");
                Serial.println(lampState);

                lastDebounceTime = millis(); 
                wasTouched = true;           
            }
        }
    }
}


void setup() {
    Serial.begin(115200);
    delay(2000); // give me time to bring up serial monitor

    pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);

    FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);

    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS mount failed");
    }


    WiFi.mode(WIFI_STA); // Set Wi-Fi to station mode (client mode)
    WiFiManager wm;
    wm.setConfigPortalTimeout(180); // 3 minutes timeout for config portal
    
    // If unable to autoConnect to network, spin up AP called "CloudLamp_Setup" with no password
    if(!wm.autoConnect("CloudLamp_Setup")) {
        Serial.println("Failed to connect and hit timeout");
        delay(3000);
        ESP.restart();
    }

    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    WiFi.onEvent(onWiFiDisconnect, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

    // If connected to WiFi, initialize mDNS with hostname "cloudlamp.local"
    if (!MDNS.begin("cloudlamp")) {
        Serial.println("mDNS responder failed");
    } else {
        Serial.println("mDNS started at http://cloudlamp.local");
        // Broadcast the HTTP and WS services so clients can find it easier
        MDNS.addService("http", "tcp", 80);
        MDNS.addService("ws", "tcp", 80);
    }

    initWebSocket();
    delay(2000); // Stabilize before starting capacitive touch
}

void loop() {
    ws.cleanupClients();

    // --- ON DEMAND CONFIG PORTAL TRIGGER ---
    if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
        delay(50); // Debounce
        if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
            Serial.println("BOOT button pressed! Entering Setup Mode...");
            
            // Visual feedback: Turn lamp solid Blue to indicate AP mode
            fill_solid(leds, NUM_LEDS, CRGB::Blue);
            FastLED.show();

            server.end(); // Stop the web server to free up resources for the config portal

            WiFiManager wm;
            // Launch the portal on-demand
            if (!wm.startConfigPortal("CloudLamp_Setup")) {
                Serial.println("Failed to connect or hit timeout");
                delay(3000);
                ESP.restart();
            } else {
                // If you enter new credentials and connect successfully:
                Serial.println("Connected to new network!");
                
                // Rebooting is the safest way to ensure the AsyncWebServer 
                // and mDNS bind correctly to your brand new IP address.
                ESP.restart(); 
            }
        }
    }


    touchValue = touchRead(touchPin);
    updateLEDs(touchValue);

    if (lampState == HIGH) {
        switch (lampMode) {
            case 1:
                thunderstorm();
                break;
            case 2:
                breathing();
                break;
            case 3:
                rainbowFlow();
                break;
            case 4:
                meteor();
                break;
        }
    } else {
        fadeToBlackBy(leds, NUM_LEDS, 50);
        FastLED.show();
    }
}

