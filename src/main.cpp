#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>\n#include <HTTPClient.h>\n#include <WiFiClientSecure.h>\n#include <ArduinoJson.h>

// ===================== WIFI =====================
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";\n\n// Supabase cloud database. Chỉ dùng ANON key, không dùng service_role trên ESP32.\nconst char* SUPABASE_URL = "https://YOUR_PROJECT.supabase.co";\nconst char* SUPABASE_ANON_KEY = "YOUR_SUPABASE_ANON_KEY";

// ===================== PINOUT =====================
constexpr uint8_t SOIL_A_PIN = 34;
constexpr uint8_t SOIL_B_PIN = 35;
constexpr uint8_t DHT_PIN    = 4;
constexpr uint8_t MQ2_PIN    = 36;
constexpr uint8_t RAIN_PIN   = 39;

constexpr uint8_t PUMP_RELAY_PIN = 26;
constexpr uint8_t FAN_RELAY_PIN  = 27;
constexpr uint8_t BUZZER_PIN     = 25;
constexpr uint8_t STATUS_LED_PIN = 2;

constexpr uint8_t DHT_TYPE = DHT22;

// Đổi thành false nếu relay của bạn kích mức LOW.
constexpr bool RELAY_ACTIVE_HIGH = true;

// ===================== THRESHOLDS =====================
constexpr int SOIL_DRY_THRESHOLD = 35;   // %
constexpr float HIGH_TEMP = 32.0;        // °C
constexpr int SMOKE_THRESHOLD = 1800;    // ADC, cần hiệu chỉnh thực tế
constexpr int RAIN_THRESHOLD = 1800;     // ADC, cần hiệu chỉnh thực tế\n\nint soilDryThreshold = SOIL_DRY_THRESHOLD;\nfloat highTemperature = HIGH_TEMP;\nint smokeThreshold = SMOKE_THRESHOLD;\nint rainThreshold = RAIN_THRESHOLD;\nbool autoMode = true;\nbool forcePump = false;\nbool forceFan = false;

DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
WebServer server(80);

struct SensorData {
  int soilA = 0;
  int soilB = 0;
  float temperature = NAN;
  float humidity = NAN;
  int smoke = 0;
  int rain = 0;
  bool pump = false;
  bool fan = false;
  bool alarm = false;
  bool raining = false;
};

SensorData data;
unsigned long lastRead = 0;
unsigned long lastLcd = 0;\nunsigned long lastCloudRead = 0;\nunsigned long lastCloudUpload = 0;

// ===================== HELPERS =====================
void setRelay(uint8_t pin, bool on) {
  digitalWrite(pin, RELAY_ACTIVE_HIGH ? (on ? HIGH : LOW)
                                      : (on ? LOW : HIGH));
}

int soilPercent(int raw) {
  // Hiệu chỉnh hai giá trị này theo cảm biến thực tế.
  constexpr int DRY_RAW = 3200;
  constexpr int WET_RAW = 1300;
  int pct = map(raw, DRY_RAW, WET_RAW, 0, 100);
  return constrain(pct, 0, 100);
}

String jsonBool(bool v) {
  return v ? "true" : "false";
}

String numberOrNull(float v) {
  return isnan(v) ? "null" : String(v, 1);
}

void readSensors() {
  int soilARaw = analogRead(SOIL_A_PIN);
  int soilBRaw = analogRead(SOIL_B_PIN);

  data.soilA = soilPercent(soilARaw);
  data.soilB = soilPercent(soilBRaw);
  data.temperature = dht.readTemperature();
  data.humidity = dht.readHumidity();
  data.smoke = analogRead(MQ2_PIN);
  data.rain = analogRead(RAIN_PIN);

  data.raining = data.rain < rainThreshold;

  // Tự động tưới khi đất khô.
  bool dry = data.soilA < SOIL_DRY_THRESHOLD ||
             data.soilB < SOIL_DRY_THRESHOLD;

  data.pump = dry && !data.raining;

  // Tự động bật quạt khi nhiệt độ cao.
  data.fan = autoMode ? (!isnan(data.temperature) && data.temperature >= highTemperature) : forceFan;

  // Cảnh báo khói hoặc nhiệt độ bất thường.
  data.alarm = data.smoke >= smokeThreshold;

  setRelay(PUMP_RELAY_PIN, data.pump);
  setRelay(FAN_RELAY_PIN, data.fan);
  digitalWrite(BUZZER_PIN, data.alarm ? HIGH : LOW);
  digitalWrite(STATUS_LED_PIN, data.alarm ? HIGH : LOW);
}

void syncSettingsFromCloud() {\n  if (WiFi.status() != WL_CONNECTED || String(SUPABASE_URL).indexOf("YOUR_PROJECT") >= 0) return;\n  WiFiClientSecure client; client.setInsecure();\n  HTTPClient http;\n  String url = String(SUPABASE_URL) + "/rest/v1/greenhouse_settings?select=*&id=eq.1";\n  if (!http.begin(client, url)) return;\n  http.addHeader("apikey", SUPABASE_ANON_KEY);\n  http.addHeader("Authorization", String("Bearer ") + SUPABASE_ANON_KEY);\n  int code = http.GET();\n  if (code == 200) {\n    DynamicJsonDocument doc(2048);\n    if (deserializeJson(doc, http.getString()) == DeserializationError::Ok && doc.size() > 0) {\n      JsonObject s = doc[0];\n      soilDryThreshold = s["soil_dry_threshold"] | SOIL_DRY_THRESHOLD;\n      highTemperature = s["high_temperature"] | HIGH_TEMP;\n      smokeThreshold = s["smoke_threshold"] | SMOKE_THRESHOLD;\n      rainThreshold = s["rain_threshold"] | RAIN_THRESHOLD;\n      autoMode = s["auto_mode"] | true;\n      forcePump = s["force_pump"] | false;\n      forceFan = s["force_fan"] | false;\n    }\n  }\n  http.end();\n}\n\nvoid uploadReadingToCloud() {\n  if (WiFi.status() != WL_CONNECTED || String(SUPABASE_URL).indexOf("YOUR_PROJECT") >= 0) return;\n  WiFiClientSecure client; client.setInsecure();\n  HTTPClient http;\n  String url = String(SUPABASE_URL) + "/rest/v1/greenhouse_readings";\n  if (!http.begin(client, url)) return;\n  http.addHeader("apikey", SUPABASE_ANON_KEY);\n  http.addHeader("Authorization", String("Bearer ") + SUPABASE_ANON_KEY);\n  http.addHeader("Content-Type", "application/json");\n  http.addHeader("Prefer", "return=minimal");\n  String body = "{\"soil_a\":" + String(data.soilA) + ",\"soil_b\":" + String(data.soilB) + ",\"temperature\":" + numberOrNull(data.temperature) + ",\"humidity\":" + numberOrNull(data.humidity) + ",\"smoke\":" + String(data.smoke) + ",\"rain\":" + String(data.rain) + ",\"raining\":" + jsonBool(data.raining) + ",\"pump\":" + jsonBool(data.pump) + ",\"fan\":" + jsonBool(data.fan) + ",\"alarm\":" + jsonBool(data.alarm) + "}";\n  int code = http.POST(body);\n  Serial.printf("Cloud upload HTTP=%d\\n", code);\n  http.end();\n}\n\nvoid updateLcd() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("A:");
  lcd.print(data.soilA);
  lcd.print("% B:");
  lcd.print(data.soilB);
  lcd.print("%");

  lcd.setCursor(0, 1);
  if (isnan(data.temperature)) {
    lcd.print("DHT error");
  } else {
    lcd.print("T:");
    lcd.print(data.temperature, 1);
    lcd.print("C H:");
    lcd.print(data.humidity, 0);
    lcd.print("%");
  }
}

String dashboardHtml() {
  return R"HTML(
<!doctype html>
<html lang="vi">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>NSL Greenhouse</title>
<style>
body{font-family:system-ui,sans-serif;background:#101522;color:#eee;margin:0;padding:24px}
h1{margin-top:0}.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(180px,1fr));gap:14px}
.card{background:#1a2232;border:1px solid #303b52;border-radius:16px;padding:18px}
.value{font-size:30px;font-weight:700;margin-top:8px}
.ok{color:#62d98b}.warn{color:#ffcc66}.danger{color:#ff7070}
small{color:#aeb8ca}
</style>
</head>
<body>
<h1>🌱 Nhà kính NSL</h1>
<p><small>ESP32 • giám sát thời gian thực</small></p>
<div class="grid">
<div class="card">🌱 Cây A<div id="a" class="value">--%</div></div>
<div class="card">🌱 Cây B<div id="b" class="value">--%</div></div>
<div class="card">🌡️ Nhiệt độ<div id="t" class="value">--</div></div>
<div class="card">💧 Độ ẩm KK<div id="h" class="value">--</div></div>
<div class="card">💨 Khói MQ-2<div id="s" class="value">--</div></div>
<div class="card">🌧️ Mưa<div id="r" class="value">--</div></div>
<div class="card">💦 Bơm<div id="p" class="value">--</div></div>
<div class="card">🌀 Quạt<div id="f" class="value">--</div></div>
<div class="card">🚨 Cảnh báo<div id="al" class="value">--</div></div>
</div>
<script>
async function update(){
  try{
    const d=await (await fetch('/api/data')).json();
    a.textContent=d.soilA+'%'; b.textContent=d.soilB+'%';
    t.textContent=d.temperature===null?'Lỗi':d.temperature+' °C';
    h.textContent=d.humidity===null?'Lỗi':d.humidity+' %';
    s.textContent=d.smoke; r.textContent=d.raining?'ĐANG MƯA':'KHÔ';
    p.textContent=d.pump?'BẬT':'TẮT'; f.textContent=d.fan?'BẬT':'TẮT';
    al.textContent=d.alarm?'NGUY HIỂM':'BÌNH THƯỜNG';
    al.className='value '+(d.alarm?'danger':'ok');
  }catch(e){document.body.style.opacity=.6}
}
update(); setInterval(update,2000);
</script>
</body>
</html>
)HTML";
}

void handleData() {
  String json = "{";
  json += "\"soilA\":" + String(data.soilA) + ",";
  json += "\"soilB\":" + String(data.soilB) + ",";
  json += "\"temperature\":" + numberOrNull(data.temperature) + ",";
  json += "\"humidity\":" + numberOrNull(data.humidity) + ",";
  json += "\"smoke\":" + String(data.smoke) + ",";
  json += "\"rain\":" + String(data.rain) + ",";
  json += "\"raining\":" + jsonBool(data.raining) + ",";
  json += "\"pump\":" + jsonBool(data.pump) + ",";
  json += "\"fan\":" + jsonBool(data.fan) + ",";
  json += "\"alarm\":" + jsonBool(data.alarm);
  json += "}";
  server.send(200, "application/json", json);
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting WiFi");
  unsigned long started = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - started < 15000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Dashboard: http://");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi failed. Running sensor/control locally.");
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(PUMP_RELAY_PIN, OUTPUT);
  pinMode(FAN_RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);

  setRelay(PUMP_RELAY_PIN, false);
  setRelay(FAN_RELAY_PIN, false);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(STATUS_LED_PIN, LOW);

  analogReadResolution(12);

  dht.begin();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("NSL Greenhouse");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");

  connectWiFi();

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html; charset=utf-8", dashboardHtml());
  });
  server.on("/api/data", HTTP_GET, handleData);
  server.begin();

  readSensors();
  updateLcd();
}

void loop() {
  server.handleClient();

  if (millis() - lastRead >= 2000) {
    lastRead = millis();
    readSensors();

    Serial.printf("Soil A=%d%% | Soil B=%d%% | T=%.1f C | H=%.1f%% | MQ2=%d | Rain=%d | Pump=%s | Fan=%s | Alarm=%s\n",
      data.soilA, data.soilB, data.temperature, data.humidity,
      data.smoke, data.rain,
      data.pump ? "ON" : "OFF",
      data.fan ? "ON" : "OFF",
      data.alarm ? "ON" : "OFF");
  }

  if (millis() - lastCloudRead >= 10000) {\n    lastCloudRead = millis();\n    syncSettingsFromCloud();\n    readSensors();\n  }\n\n  if (millis() - lastCloudUpload >= 10000) {\n    lastCloudUpload = millis();\n    uploadReadingToCloud();\n  }\n\n  if (millis() - lastLcd >= 5000) {
    lastLcd = millis();
    updateLcd();
  }
}
