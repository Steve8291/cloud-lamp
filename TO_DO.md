# index.html
1) Change the `<title>` and `<h1>` to reflect your project

2) Adjust the buttons. You can use https://jsfiddle.net/ to check how it works. Note: I removed the "ledState" lines. Example:
```html
    <!-- Old Code -->
    <p>LED 1 State: <span id="ledState">OFF</span></p>
    <button class="button" id="button1" onclick="toggleLED(1)">Toggle LED 1</button>

    <!-- New Code -->
    <button class="button" id="button1" onclick="toggleLED(1)">Rainbow</button>
```

# style.css
1) Adjust the colors of your buttons. There is one button color for when it is clicked (active) and one for when it is not active.
2) Decide if you want to change the font.
3) Think about putting a background picture in. You will need to search how to do this. It will be something like this. The image will need to be in your data folder.
```css
  body {
    background-image: url('your-image.jpg');
    background-repeat: no-repeat;
    background-attachment: fixed;
    background-size: cover;
  }
```

# script.js
1) In the function `onMessage` you will want to remove all the lines that change the ledState text that you deleted from the .html code. Delete all the lines that look like this:
```js
    document.getElementById("ledState").innerHTML = message.status == 1 ? "ON" : "OFF";

```

# platform.ini
1) Add the required libraries for:
```c++
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h> 
#include <LittleFS.h>
```
