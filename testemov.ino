#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

// -------- WIFI --------
const char* ssid = "PAPAGOIABAZ";
const char* password = "euamojesus";

// -------- SERVER --------
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// -------- PINOS --------
const int ENApin = 14;
const int IN1pin = 27;
const int IN2pin = 26;

const int IN3pin = 25;
const int IN4pin = 33;
const int ENBpin = 32;

const int armaPin = 4;   // AJUSTA se precisar

// -------- PWM --------
const int pwmFreq = 20000;
const int pwmRes  = 8;
const int pwmChA  = 0;
const int pwmChB  = 1;

// -------- FAILSAFE --------
unsigned long lastCmdTime = 0;
const unsigned long FAILSAFE = 200;

// -------- MOTOR --------
void stopAll() {
  digitalWrite(IN1pin, LOW);
  digitalWrite(IN2pin, LOW);
  digitalWrite(IN3pin, LOW);
  digitalWrite(IN4pin, LOW);
  ledcWrite(pwmChA, 0);
  ledcWrite(pwmChB, 0);
}

void drive(int left, int right) {
  lastCmdTime = millis();

  // Motor A
  digitalWrite(IN1pin, left >= 0);
  digitalWrite(IN2pin, left < 0);
  ledcWrite(pwmChA, abs(left));

  // Motor B
  digitalWrite(IN3pin, right >= 0);
  digitalWrite(IN4pin, right < 0);
  ledcWrite(pwmChB, abs(right));
}

// -------- WEBSOCKET --------
void onWsEvent(AsyncWebSocket *server,
               AsyncWebSocketClient *client,
               AwsEventType type,
               void *arg,
               uint8_t *data,
               size_t len) {

  if (type == WS_EVT_DATA) {
    lastCmdTime = millis();
    String msg;
    for (size_t i = 0; i < len; i++) msg += (char)data[i];

    if (msg == "STOP") {
      stopAll();
      return;
    }

    if (msg == "A1") digitalWrite(armaPin, HIGH);
    else if (msg == "A0") digitalWrite(armaPin, LOW);
    else {
      int sep = msg.indexOf(',');
      if (sep > 0) {
        int left  = msg.substring(0, sep).toInt();
        int right = msg.substring(sep + 1).toInt();
        drive(left, right);
      }
    }
  }
}

// -------- SETUP --------
void setup() {
  Serial.begin(115200);

  pinMode(IN1pin, OUTPUT);
  pinMode(IN2pin, OUTPUT);
  pinMode(IN3pin, OUTPUT);
  pinMode(IN4pin, OUTPUT);
  pinMode(armaPin, OUTPUT);
  digitalWrite(armaPin, LOW);

  ledcSetup(pwmChA, pwmFreq, pwmRes);
  ledcSetup(pwmChB, pwmFreq, pwmRes);
  ledcAttachPin(ENApin, pwmChA);
  ledcAttachPin(ENBpin, pwmChB);

  stopAll();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(300);

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send(200, "text/html", R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, user-scalable=no">
<style>
body { background:#111; color:white; text-align:center; font-family:Arial; }
#joy {
  width:60vw; height:60vw;
  max-width:300px; max-height:300px;
  background:#333; border-radius:50%;
  margin:20px auto; position:relative;
}
#stick {
  width:30%; height:30%;
  background:#00c853; border-radius:50%;
  position:absolute; top:35%; left:35%;
}
button { width:40vw; height:8vh; font-size:5vw; margin:5px; }
</style>
</head>
<body>

<h1>ROBO</h1>

<div id="joy">
  <div id="stick"></div>
</div>

<button onclick="ws.send('A1')">🔫 ARMA ON</button>
<button onclick="ws.send('A0')">🛑 ARMA OFF</button>

<script>
let ws = new WebSocket(`ws://${location.hostname}/ws`);
let joy = document.getElementById("joy");
let stick = document.getElementById("stick");
let center = joy.offsetWidth / 2;

joy.ontouchmove = e => {
  let t = e.touches[0];
  let r = joy.getBoundingClientRect();
  let x = t.clientX - r.left - center;
  let y = t.clientY - r.top - center;

  let max = center;
  x = Math.max(-max, Math.min(max, x));
  y = Math.max(-max, Math.min(max, y));

  stick.style.left = (x + center - stick.offsetWidth/2) + "px";
  stick.style.top  = (y + center - stick.offsetHeight/2) + "px";

  let speed = 255;
  let left  = Math.round((-y + x) / max * speed);
  let right = Math.round((-y - x) / max * speed);

  ws.send(left + "," + right);
};

joy.ontouchend = () => {
  stick.style.left = "35%";
  stick.style.top = "35%";
  ws.send("STOP");
};
</script>

</body>
</html>
)rawliteral");
  });

  server.begin();
}

// -------- LOOP --------
void loop() {
  if (millis() - lastCmdTime > FAILSAFE) {
    stopAll();
    lastCmdTime = millis();
  }
}
