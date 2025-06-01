#include <WiFi.h>
#include <WebSocketsServer.h>
#include <WebServer.h>

// WiFi ayarları
const char* ssid = "Galaxy A05 9842";
const char* password = "vljxfz52";

// Web ve WebSocket sunucuları
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// HTML sayfası - geliştirilmiş
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 Joystick Controller</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        .container { max-width: 800px; margin: 0 auto; }
        .data-box { background-color: #f0f0f0; padding: 15px; margin-bottom: 10px; border-radius: 5px; }
        pre { background-color: #eee; padding: 10px; overflow-x: auto; }
        .gauge { width: 100%; height: 30px; background-color: #ddd; margin-top: 5px; margin-bottom: 15px; border-radius: 4px; position: relative; }
        .gauge-fill { height: 100%; background-color: #4CAF50; width: 0%; transition: width 0.2s; border-radius: 4px; }
        .gauge-center { position: absolute; width: 2px; height: 30px; background-color: #666; left: 50%; top: 0; z-index: 1; }
        .command-display { font-size: 24px; font-weight: bold; text-align: center; padding: 20px; background-color: #e1f5fe; border-radius: 10px; margin-bottom: 20px; }
        .status-box { display: inline-block; padding: 5px 10px; border-radius: 5px; margin-right: 10px; background-color: #e0e0e0; }
        .status-active { background-color: #4CAF50; color: white; }
        .connection-status { padding: 10px; margin-bottom: 10px; border-radius: 5px; text-align: center; font-weight: bold; }
        .connected { background-color: #4CAF50; color: white; }
        .disconnected { background-color: #f44336; color: white; }
        .calibration-info { background-color: #fff3cd; padding: 10px; border-radius: 5px; margin-bottom: 10px; }
        .noise-warning { background-color: #f8d7da; padding: 10px; border-radius: 5px; margin-bottom: 10px; color: #721c24; }
    </style>
</head>
<body>
    <div class="container">
        <h1>ESP32 Joystick Controller (Dual Button)</h1>
        
        <div class="connection-status disconnected" id="connectionStatus">Disconnected</div>
        
        <div class="command-display" id="commandDisplay">CENTER</div>
        
        <div class="calibration-info">
            <strong>Calibration Info:</strong> Center: ~1775, Dead Zone: ±100<br>
            <strong>Noise Filter:</strong> Active - Outlier detection enabled
        </div>
        
        <div class="noise-warning" id="noiseWarning" style="display: none;">
            <strong>⚠ Noise Detected:</strong> Potentiometer interference filtered
        </div>
        
        <div class="data-box">
            <h2>Joystick Status</h2>
            <div>
                <span class="status-box" id="leftStatus">LEFT</span>
                <span class="status-box" id="rightStatus">RIGHT</span>
                <span class="status-box" id="upStatus">UP</span>
                <span class="status-box" id="downStatus">DOWN</span>
                <span class="status-box" id="centerStatus" class="status-active">CENTER</span>
            </div>
        </div>
        
        <div class="data-box">
            <h2>Joystick Values</h2>
            <div>
                <h3>X Axis: <span id="valueX">0</span> | Normalized: <span id="normalizedX">0.000</span></h3>
                <div class="gauge">
                    <div class="gauge-center"></div>
                    <div id="gaugeX" class="gauge-fill"></div>
                </div>
            </div>
            <div>
                <h3>Y Axis: <span id="valueY">0</span> | Normalized: <span id="normalizedY">0.000</span></h3>
                <div class="gauge">
                    <div class="gauge-center"></div>
                    <div id="gaugeY" class="gauge-fill"></div>
                </div>
            </div>
            <div>
                <h3>Potentiometer: <span id="valuePot">0</span></h3>
                <div class="gauge">
                    <div id="gaugePot" class="gauge-fill"></div>
                </div>
            </div>
        </div>
        
        <div class="data-box">
            <h2>Buttons</h2>
            <div>Joystick Button: <span id="jsBtnText">NOT PRESSED</span></div>
            <div>External Button 1: <span id="extBtnText">NOT PRESSED</span></div>
            <div>External Button 2: <span id="extBtn2Text">NOT PRESSED</span></div>
        </div>
        
        <div class="data-box">
            <h2>Connection Info</h2>
            <div>Connected Clients: <span id="clientCount">0</span></div>
            <div>Last Heartbeat: <span id="lastHeartbeat">Never</span></div>
            <div>Noise Events: <span id="noiseCount">0</span></div>
        </div>
        
        <div class="data-box">
            <h2>JSON Data:</h2>
            <pre id="rawData">Waiting for data...</pre>
        </div>
        
        <div class="data-box">
            <h2>Unreal Engine Connection</h2>
            <p>WebSocket address: <code>ws://<span id="wsAddress">ESP32_IP</span>:81</code></p>
        </div>
    </div>

    <script>
        let lastHeartbeatTime = null;
        let connectionCheckInterval;
        let noiseEventCount = 0;

        document.addEventListener('DOMContentLoaded', function() {
            document.getElementById('wsAddress').innerText = window.location.hostname;
            
            function updateConnectionStatus(connected) {
                const statusEl = document.getElementById('connectionStatus');
                if (connected) {
                    statusEl.className = 'connection-status connected';
                    statusEl.innerText = 'Connected';
                } else {
                    statusEl.className = 'connection-status disconnected';
                    statusEl.innerText = 'Disconnected';
                }
            }
            
            function showNoiseWarning() {
                const warningEl = document.getElementById('noiseWarning');
                warningEl.style.display = 'block';
                noiseEventCount++;
                document.getElementById('noiseCount').innerText = noiseEventCount;
                setTimeout(() => {
                    warningEl.style.display = 'none';
                }, 3000);
            }
            
            function connectWebSocket() {
                const ws = new WebSocket(ws://${window.location.hostname}:81);
                
                ws.onopen = function() {
                    console.log("WebSocket connected");
                    updateConnectionStatus(true);
                    
                    connectionCheckInterval = setInterval(() => {
                        if (lastHeartbeatTime) {
                            const timeSinceHeartbeat = Date.now() - lastHeartbeatTime;
                            if (timeSinceHeartbeat > 10000) {
                                updateConnectionStatus(false);
                            }
                        }
                    }, 1000);
                };
                
                ws.onclose = function() {
                    console.log("WebSocket disconnected");
                    updateConnectionStatus(false);
                    clearInterval(connectionCheckInterval);
                    setTimeout(connectWebSocket, 2000);
                };
                
                ws.onerror = function(error) {
                    console.error("WebSocket error:", error);
                    updateConnectionStatus(false);
                };
                
                ws.onmessage = function(event) {
                    try {
                        const data = JSON.parse(event.data);
                        
                        if (data.heartbeat) {
                            lastHeartbeatTime = Date.now();
                            document.getElementById('lastHeartbeat').innerText = new Date().toLocaleTimeString();
                            updateConnectionStatus(true);
                            return;
                        }
                        
                        if (data.noiseDetected) {
                            showNoiseWarning();
                        }
                        
                        console.log("Data received:", data);
                        lastHeartbeatTime = Date.now();
                        updateConnectionStatus(true);
                        
                        document.getElementById('commandDisplay').innerText = data.cmd;
                        document.getElementById('valueX').innerText = data.x;
                        document.getElementById('valueY').innerText = data.y;
                        document.getElementById('valuePot').innerText = data.pot || 0;
                        
                        if (data.normalizedX !== undefined) {
                            document.getElementById('normalizedX').innerText = data.normalizedX.toFixed(3);
                        }
                        if (data.normalizedY !== undefined) {
                            document.getElementById('normalizedY').innerText = data.normalizedY.toFixed(3);
                        }
                        
                        const allStatus = document.querySelectorAll('.status-box');
                        allStatus.forEach(el => el.classList.remove('status-active'));
                        
                        if (data.xStatus === "LEFT") {
                            document.getElementById('leftStatus').classList.add('status-active');
                        }
                        if (data.xStatus === "RIGHT") {
                            document.getElementById('rightStatus').classList.add('status-active');
                        }
                        if (data.yStatus === "UP") {
                            document.getElementById('upStatus').classList.add('status-active');
                        }
                        if (data.yStatus === "DOWN") {
                            document.getElementById('downStatus').classList.add('status-active');
                        }
                        if (data.xStatus === "CENTER" && data.yStatus === "CENTER") {
                            document.getElementById('centerStatus').classList.add('status-active');
                        }
                        
                        if (data.normalizedX !== undefined) {
                            const xPosition = ((data.normalizedX + 1) / 2) * 100;
                            document.getElementById('gaugeX').style.width = xPosition + '%';
                        }
                        
                        if (data.normalizedY !== undefined) {
                            const yPosition = ((data.normalizedY + 1) / 2) * 100;
                            document.getElementById('gaugeY').style.width = yPosition + '%';
                        }
                        
                        if (data.pot !== undefined) {
                            const potPosition = (data.pot / 4095) * 100;
                            document.getElementById('gaugePot').style.width = potPosition + '%';
                        }
                        
                        document.getElementById('jsBtnText').innerText = data.jsBtn ? "PRESSED" : "NOT PRESSED";
                        document.getElementById('extBtnText').innerText = data.extBtn ? "PRESSED" : "NOT PRESSED";
                        document.getElementById('extBtn2Text').innerText = data.extBtn2 ? "PRESSED" : "NOT PRESSED";
                        
                        document.getElementById('clientCount').innerText = data.connectedClients || 0;
                        
                        document.getElementById('rawData').innerText = JSON.stringify(data, null, 2);
                        
                    } catch (e) {
                        console.error("JSON parsing error:", e);
                    }
                };
            }
            
            connectWebSocket();
        });
    </script>
</body>
</html>
)rawliteral";

// Pin tanımlamaları - ADC1 kanalları
#define VRX_PIN  32      // Joystick X ekseni
#define VRY_PIN  33      // Joystick Y ekseni
#define SW_PIN   27      // Joystick'in buton pini
#define BUTTON_PIN 13    // İlk harici buton pini
#define BUTTON2_PIN 14   // İkinci harici buton pini
#define POT_PIN 35       // Potansiyometre sinyal pini

// ESP32 kalibre edilmiş eşik değerleri
#define LEFT_THRESHOLD  800
#define RIGHT_THRESHOLD 2800
#define UP_THRESHOLD    800
#define DOWN_THRESHOLD  2800

// Gerçek orta nokta değeri
#define CENTER_VALUE    1775

// Dead zone (merkez etrafında hareket olmayan alan)
#define DEAD_ZONE       100

// Kalibreasyon değerleri
#define X_MIN           100
#define X_MAX           3200
#define Y_MIN           100
#define Y_MAX           4095

// Keep-alive ayarları
#define HEARTBEAT_INTERVAL 3000
#define CLIENT_TIMEOUT 15000

// Gürültü filtreleme ayarları
#define NOISE_THRESHOLD 500  // Ani değişiklik eşiği
#define POT_INTERFERENCE_LEVEL 5500// Potansiyometre gürültü eşiği düşürüldü
#define POT_MAX_VALUE 5500 // Potansiyometre maksimum değeri sınırlandırıldı
#define POT_MIN_VALUE 100   // Potansiyometre minimum değeri

// Değişkenler
int valueX = 0;
int valueY = 0;
int potValue = 0;
int joystickButtonState = 0;
int lastJoystickButtonState = 0;
int buttonState = 0;
int lastButtonState = 0;
int button2State = 0;
int lastButton2State = 0;

// Gürültü filtreleme değişkenleri
int lastValidX = CENTER_VALUE;
int lastValidY = CENTER_VALUE;
bool noiseDetected = false;

// Keep-alive değişkenleri
unsigned long lastHeartbeat = 0;
unsigned long lastClientCheck = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== ESP32 Joystick Controller Starting (Dual Button) ===");
  
  // ADC ayarları - gürültü azaltma
  analogReadResolution(12); // 12-bit (0-4095)
  
  // Her pin için ayrı attenuation ayarı
  analogSetPinAttenuation(VRX_PIN, ADC_11db);
  analogSetPinAttenuation(VRY_PIN, ADC_11db);
  analogSetPinAttenuation(POT_PIN, ADC_11db);
  
  // ADC genişlik ayarı
  analogSetWidth(12);
  
  // Pin modlarını ayarla
  pinMode(VRX_PIN, INPUT);
  pinMode(VRY_PIN, INPUT);
  pinMode(SW_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(POT_PIN, INPUT);
  
  Serial.println("Pins configured with dual button support");
  Serial.printf("Button 1 Pin: %d, Button 2 Pin: %d\n", BUTTON_PIN, BUTTON2_PIN);
  Serial.printf("Calibration Values: Center=%d, Dead Zone=%d\n", CENTER_VALUE, DEAD_ZONE);
  Serial.printf("Noise Filter: Threshold=%d, Pot Interference Level=%d\n", 
                NOISE_THRESHOLD, POT_INTERFERENCE_LEVEL);
  
  // WiFi bağlantısı
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  
  int connectionAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && connectionAttempts < 20) {
    delay(500);
    Serial.print(".");
    connectionAttempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("WiFi Connected! IP address: ");
    Serial.println(WiFi.localIP());
    
    // Web sunucusu
    server.on("/", HTTP_GET, []() {
      server.send(200, "text/html", index_html);
    });
    server.begin();
    Serial.println("Web server started");
    
    // WebSocket sunucusu
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    Serial.println("WebSocket server started");
  } else {
    Serial.println("\nWiFi connection failed! Running without network...");
  }
  
  Serial.println("=== ESP32 Joystick Controller Ready (Dual Button) ===\n");
}

void loop() {
  // Web ve WebSocket sunucularını işle
  if (WiFi.status() == WL_CONNECTED) {
    server.handleClient();
    webSocket.loop();
    sendHeartbeat();
    checkClientConnections();
  } else {
    reconnectWiFi();
  }
  
  // Gelişmiş analog okuma - gürültü filtrelemeli
  noiseDetected = false;
  
  // Önce potansiyometreyi oku
  potValue = readAnalogFiltered(POT_PIN, 10);
  delay(5); // Voltaj stabilizasyonu
  
  // Sonra joystick pinlerini oku
  int rawX = readAnalogFiltered(VRX_PIN, 8);
  delay(3);
  int rawY = readAnalogFiltered(VRY_PIN, 8);
  
  // Outlier detection - anormal değerleri filtrele
  if (abs(rawX - lastValidX) > NOISE_THRESHOLD && potValue > POT_INTERFERENCE_LEVEL) {
    valueX = lastValidX;
    noiseDetected = true;
    Serial.println("X outlier detected and filtered");
  } else {
    valueX = rawX;
    lastValidX = rawX;
  }
  
  if (abs(rawY - lastValidY) > NOISE_THRESHOLD && potValue > POT_INTERFERENCE_LEVEL) {
    valueY = lastValidY;
    noiseDetected = true;
    Serial.println("Y outlier detected and filtered");
  } else {
    valueY = rawY;
    lastValidY = rawY;
  }
  
  // Buton durumları
  joystickButtonState = digitalRead(SW_PIN);
  buttonState = digitalRead(BUTTON_PIN);
  button2State = digitalRead(BUTTON2_PIN);
  
  // Debug monitoring
  debugAnalogNoise();
  
  // Komut belirleme
  String xStatus = "CENTER";
  String yStatus = "CENTER";
  
  // X ekseni durumu
  if (valueX < (CENTER_VALUE - DEAD_ZONE)) {
    if (valueX < LEFT_THRESHOLD) {
      xStatus = "LEFT";
    }
  }
  else if (valueX > (CENTER_VALUE + DEAD_ZONE)) {
    if (valueX > RIGHT_THRESHOLD) {
      xStatus = "RIGHT";
    }
  }
  
  // Y ekseni durumu
  if (valueY < (CENTER_VALUE - DEAD_ZONE)) {
    if (valueY < UP_THRESHOLD) {
      yStatus = "UP";
    }
  }
  else if (valueY > (CENTER_VALUE + DEAD_ZONE)) {
    if (valueY > DOWN_THRESHOLD) {
      yStatus = "DOWN";
    }
  }
  
  // Kombine komut
  String cmdText = "CENTER";
  
  if (xStatus != "CENTER" || yStatus != "CENTER") {
    cmdText = "";
    if (xStatus != "CENTER") {
      cmdText += xStatus;
    }
    if (yStatus != "CENTER") {
      if (cmdText.length() > 0) cmdText += " + ";
      cmdText += yStatus;
    }
  }
  
  // Buton kontrolleri
  if (joystickButtonState == LOW && lastJoystickButtonState == HIGH) {
    if (cmdText == "CENTER") {
      cmdText = "JOYSTICK CLICK";
    } else {
      cmdText += " + JOYSTICK CLICK";
    }
  }
  
  if (buttonState == LOW && lastButtonState == HIGH) {
    if (cmdText == "CENTER") {
      cmdText = "BUTTON1 PRESS";
    } else {
      cmdText += " + BUTTON1 PRESS";
    }
  }
  
  if (button2State == LOW && lastButton2State == HIGH) {
    if (cmdText == "CENTER") {
      cmdText = "BUTTON2 PRESS";
    } else {
      cmdText += " + BUTTON2 PRESS";
    }
  }
  
  lastJoystickButtonState = joystickButtonState;
  lastButtonState = buttonState;
  lastButton2State = button2State;
  
  // WebSocket verisi gönder
  if (WiFi.status() == WL_CONNECTED && webSocket.connectedClients() > 0) {
    String json = "{";
    json += "\"x\":" + String(valueX) + ",";
    json += "\"y\":" + String(valueY) + ",";
    json += "\"pot\":" + String(potValue) + ",";
    json += "\"jsBtn\":" + String(joystickButtonState == LOW ? 1 : 0) + ",";
    json += "\"extBtn\":" + String(buttonState == LOW ? 1 : 0) + ",";
    json += "\"extBtn2\":" + String(button2State == LOW ? 1 : 0) + ",";
    json += "\"xStatus\":\"" + xStatus + "\",";
    json += "\"yStatus\":\"" + yStatus + "\",";
    json += "\"cmd\":\"" + cmdText + "\",";
    json += "\"timestamp\":" + String(millis()) + ",";
    json += "\"connectedClients\":" + String(webSocket.connectedClients()) + ",";
    json += "\"noiseDetected\":" + String(noiseDetected ? "true" : "false") + ",";
    
    // Normalize edilmiş değerler
    float normalizedX = map(valueX, X_MIN, X_MAX, -1000, 1000) / 1000.0;
    float normalizedY = map(valueY, Y_MIN, Y_MAX, -1000, 1000) / 1000.0;
    normalizedX = constrain(normalizedX, -1.0, 1.0);
    normalizedY = constrain(normalizedY, -1.0, 1.0);
    
    json += "\"normalizedX\":" + String(normalizedX, 3) + ",";
    json += "\"normalizedY\":" + String(normalizedY, 3);
    json += "}";
    
    webSocket.broadcastTXT(json);
  }
  
  delay(50);
}

// Gelişmiş analog okuma fonksiyonu
int readAnalogFiltered(int pin, int samples) {
  int readings[samples];
  int total = 0;
  
  // Örnekleri topla
  for (int i = 0; i < samples; i++) {
    readings[i] = analogRead(pin);
    delayMicroseconds(1000); // Gecikme artırıldı
    total += readings[i];
  }
  
  // Basit ortalama
  int average = total / samples;
  
  // Potansiyometre için özel işlem
  if (pin == POT_PIN) {
    // Potansiyometre değerini sınırla
    average = constrain(average, POT_MIN_VALUE, POT_MAX_VALUE);
    // Yumuşatma uygula
    static int lastPotValue = 0;
    average = (average * 0.7) + (lastPotValue * 0.3);
    lastPotValue = average;
  }
  
  // Median filtre (en büyük ve küçük değerleri çıkar)
  if (samples >= 5) {
    int min_val = readings[0];
    int max_val = readings[0];
    
    for (int i = 1; i < samples; i++) {
      if (readings[i] < min_val) min_val = readings[i];
      if (readings[i] > max_val) max_val = readings[i];
    }
    
    // Ekstrem değerleri çıkararak yeniden hesapla
    total = total - min_val - max_val;
    average = total / (samples - 2);
  }
  
  return average;
}

// Debug monitoring fonksiyonu
void debugAnalogNoise() {
  static unsigned long lastDebugTime = 0;
  static int maxX = 0, minX = 4095;
  static int maxY = 0, minY = 4095;
  static int potLast = 0;
  static int noiseEventCount = 0;
  
  if (noiseDetected) {
    noiseEventCount++;
  }
  
  if (millis() - lastDebugTime > 5000) { // 5 saniyede bir
    
    // Min/Max değerleri güncelle
    if (valueX > maxX) maxX = valueX;
    if (valueX < minX) minX = valueX;
    if (valueY > maxY) maxY = valueY;
    if (valueY < minY) minY = valueY;
    
    Serial.println("=== NOISE MONITORING (DUAL BUTTON) ===");
    Serial.printf("POT: %4d (Delta: %3d)\n", potValue, abs(potValue - potLast));
    Serial.printf("X  : %4d (Range: %3d)\n", valueX, maxX - minX);
    Serial.printf("Y  : %4d (Range: %3d)\n", valueY, maxY - minY);
    Serial.printf("BTN1: %d, BTN2: %d, JS_BTN: %d\n", buttonState, button2State, joystickButtonState);
    Serial.printf("Noise Events: %d\n", noiseEventCount);
    
    if (potValue > POT_INTERFERENCE_LEVEL) {
      Serial.println("*** HIGH POT - MONITORING FOR INTERFERENCE ***");
    }
    
    // Reset sayaçları
    static int resetCounter = 0;
    if (++resetCounter >= 3) { // 15 saniyede bir reset
      maxX = maxY = 0;
      minX = minY = 4095;
      noiseEventCount = 0;
      resetCounter = 0;
    }
    
    potLast = potValue;
    lastDebugTime = millis();
    Serial.println("======================================");
  }
}

void sendHeartbeat() {
  if (millis() - lastHeartbeat > HEARTBEAT_INTERVAL) {
    if (WiFi.status() == WL_CONNECTED && webSocket.connectedClients() > 0) {
      String heartbeat = "{";
      heartbeat += "\"heartbeat\":true,";
      heartbeat += "\"timestamp\":" + String(millis()) + ",";
      heartbeat += "\"connectedClients\":" + String(webSocket.connectedClients()) + ",";
      heartbeat += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
      heartbeat += "\"uptime\":" + String(millis() / 1000) + ",";
      heartbeat += "\"centerValue\":" + String(CENTER_VALUE) + ",";
      heartbeat += "\"deadZone\":" + String(DEAD_ZONE) + ",";
      heartbeat += "\"noiseFilterActive\":true,";
      heartbeat += "\"dualButtonMode\":true";
      heartbeat += "}";
      
      webSocket.broadcastTXT(heartbeat);
    }
    lastHeartbeat = millis();
  }
}

void checkClientConnections() {
  if (millis() - lastClientCheck > 30000) {
    if (webSocket.connectedClients() == 0) {
      Serial.println("No active WebSocket connections");
    }
    lastClientCheck = millis();
  }
}

void reconnectWiFi() {
  static unsigned long lastReconnectAttempt = 0;
  
  if (millis() - lastReconnectAttempt > 30000) {
    Serial.println("WiFi connection lost. Attempting to reconnect...");
    WiFi.disconnect();
    delay(1000);
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("");
      Serial.print("WiFi reconnected! IP: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("\nWiFi reconnection failed");
    }
    
    lastReconnectAttempt = millis();
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("WebSocket[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("WebSocket[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
        
        String welcome = "{";
        welcome += "\"connected\":true,";
        welcome += "\"clientId\":" + String(num) + ",";
        welcome += "\"timestamp\":" + String(millis()) + ",";
        welcome += "\"noiseFilterEnabled\":true,";
        welcome += "\"dualButtonMode\":true,";
        welcome += "\"calibration\":{";
        welcome += "\"centerValue\":" + String(CENTER_VALUE) + ",";
        welcome += "\"deadZone\":" + String(DEAD_ZONE) + ",";
        welcome += "\"xMin\":" + String(X_MIN) + ",";
        welcome += "\"xMax\":" + String(X_MAX) + ",";
        welcome += "\"yMin\":" + String(Y_MIN) + ",";
        welcome += "\"yMax\":" + String(Y_MAX);
        welcome += "}}";
        webSocket.sendTXT(num, welcome);
      }
      break;
    case WStype_TEXT:
      Serial.printf("WebSocket[%u] Received: %s\n", num, payload);
      
      if (strcmp((char*)payload, "ping") == 0) {
        webSocket.sendTXT(num, "pong");
      }
      break;
    case WStype_ERROR:
      Serial.printf("WebSocket[%u] Error occurred\n", num);
      break;
  }
}