
# ☁️ Cloud Lamp 

A DIY ESP32 cloud lamp with a local Web UI and capacitive touch controls.


## ✨ Features 

* **Web UI :** Access `http://cloudlamp.local` to change modes.
* **Touch Control :** Tap Pin 4 to toggle ON/OFF. 
* **Easy Wi-Fi :** Auto-creates `CloudLamp_Setup` portal if Wi-Fi fails.
    - default ip 192.168.4.1

## 🌩️ Modes 

1. **Thunderstorm :** Random lightning flashes. 
2. **Breathing :** Smooth pulsing fade. 
3. **Rainbow Flow :** Seamless color cycle. 
4. **Meteor :** Cyan trailing effect. 

## 🛠️ Hardware Wiring 

* **Microcontroller:** ESP32
* **LED Strip:** WS2812B
* **LED Data :** `GPIO 16`
* **Touch Sensor :** `GPIO 4`
* **Reset Wi-Fi :** `GPIO 0` (BOOT Button)

## 💻 Software & Libraries 

This project uses the Arduino IDE. You will need to install the following libraries via the Library Manager:

* `FastLED`
* `ESPAsyncWebServer` (and `AsyncTCP`)
* `WiFiManager` (by tzapu)
* `ArduinoJson`

## 🚀 Quick Setup 

1. **Flash :** Upload the `.ino` sketch via Arduino IDE.
2. **LittleFS :** Upload the `data` folder using the ESP32 LittleFS Tool. 
3. **Connect :** Power on, join the `CloudLamp_Setup` Wi-Fi network, and enter your home router details. 
## 🎮 Usage 

* **Web Control :** Open your browser and go to `http://cloudlamp.local`. 
* **Wi-Fi Reset :** Hold the BOOT button to re-enter Wi-Fi setup mode. 
 
 example for video:
 ![Alt text describing the video](./path/to/your/video.mp4)