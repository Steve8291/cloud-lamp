var gateway = `ws://${window.location.host}/ws`;
var websocket;

window.addEventListener("load", onLoad);

function onLoad(event) {
    initWebSocket();
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage; 
}

function onOpen(event) {
    console.log('Connection opened');
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

var activeButton = 1;

function setActiveButton(ledId) {
    if (ledId < 1 || ledId > 4) {
        return;
    }

    for (var i = 1; i <= 4; i++) {
        document.getElementById("button" + i).classList.remove('active');
    }

    activeButton = ledId;
    document.getElementById("button" + activeButton).classList.add('active');
}

function onMessage(event) {
    var message = JSON.parse(event.data);
    if (message.id == 1 || message.id == 2 || message.id == 3 || message.id == 4) {
        setActiveButton(message.id);
    }
}

function toggleLED(ledId) {
    setActiveButton(ledId);

    // Send the command
    websocket.send(JSON.stringify({ id: ledId }));
}