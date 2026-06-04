#include <WiFi.h>
#include <WebServer.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <LiquidCrystal.h>
#include <ESP32Servo.h>
#include "HX711.h"

// ================= WIFI =================
const char* ssid     = "mchau";
const char* password = "minhchauuu";

// ================= RTC =================
ThreeWire myWire(21, 22, 23);
RtcDS1302<ThreeWire> rtc(myWire);

// ================= WEB =================
WebServer server(80);

// ================= LCD =================
LiquidCrystal lcd(13, 12, 14, 27, 26, 25);

// ================= WATER =================
const int analogPin = 33;
const int relayPin  = 32;

const int mucDuoi5cm = 2000;
const int mucTren9cm = 3000;

// ================= SERVO =================
const int servoPin = 15;
Servo myServo;

// ================= HX711 =================
const int HX711_DT  = 18;
const int HX711_SCK = 19;

HX711 scale;
float calibration_factor = -211.0;

// ================= KHO =================
float khoThucAnDay = 300.0;
float binhNuocDay  = 1500.0;

float tongThucAnDaRa = 0.0;
float tongNuocDaRa   = 0.0;

float foodLeft  = 300.0;
float waterLeft = 1500.0;

float currentWeight = 0.0;

const float luuLuongBom = 80 / 6;

// ================= LOG =================
const int MAX_LOG = 30;

String feedTime[MAX_LOG];
float feedGram[MAX_LOG];
int feedCount = 0;

String waterTime[MAX_LOG];
float waterMl[MAX_LOG];
int waterCount = 0;

// ================= TRANG THAI =================
bool feedingActive = false;
float feedStartWeight = 0.0;

bool pumpActive = false;
unsigned long pumpStartMillis = 0;
String pumpStartTime = "";
float waterLeftAtPumpStart = 0.0;

int analogValue = 0;

unsigned long lastSensorMillis = 0;
unsigned long lastLcdMillis = 0;
unsigned long lastSerialMillis = 0;

// ================= HTML =================
const char MAIN_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>May cho an ESP32</title>

<style>
body{
    margin:0;
    padding:16px;
    font-family:Arial;
    background:linear-gradient(135deg,#141E30,#243B55);
    color:white;
}
.card{
    max-width:760px;
    margin:auto;
    background:rgba(255,255,255,0.12);
    padding:22px;
    border-radius:24px;
    box-shadow:0 10px 30px rgba(0,0,0,0.4);
}
h1,h2{text-align:center;}
.grid{
    display:grid;
    grid-template-columns:1fr 1fr;
    gap:12px;
}
.box{
    background:rgba(255,255,255,0.15);
    padding:14px;
    border-radius:14px;
    text-align:center;
}
.value{
    font-size:26px;
    font-weight:bold;
}
button{
    width:100%;
    padding:13px;
    margin-top:10px;
    border:none;
    border-radius:12px;
    font-size:16px;
    background:#00C896;
    color:white;
    font-weight:bold;
}
button.red{background:#E53935;}
button.blue{background:#2196F3;}
table{
    width:100%;
    border-collapse:collapse;
    margin-top:10px;
}
td,th{
    border-bottom:1px solid rgba(255,255,255,0.2);
    padding:8px;
    text-align:center;
}
.status{
    text-align:center;
    color:#FFE082;
    margin-top:10px;
}
</style>
</head>

<body>
<div class="card">
    <h1>May cho an ESP32</h1>

    <div class="grid">
        <div class="box">
            <div>Gio RTC</div>
            <div class="value" id="time">--:--:--</div>
            <div id="date">--/--/----</div>
        </div>

        <div class="box">
            <div>Can hien tai</div>
            <div class="value"><span id="weight">0</span> g</div>
        </div>

        <div class="box">
            <div>Thuc an con lai</div>
            <div class="value"><span id="foodLeft">0</span> g</div>
        </div>

        <div class="box">
            <div>Nuoc con lai</div>
            <div class="value"><span id="waterLeft">0</span> ml</div>
        </div>
    </div>

    <button type="button" class="blue" onclick="syncRTC()">DONG BO GIO DIEN THOAI</button>
    <button type="button" onclick="resetKho()">RESET KHO THUC AN / NUOC</button>
    <button type="button" class="red" onclick="clearHistory()">XOA LICH SU</button>

    <div class="status" id="status">Dang ket noi...</div>

    <h2>Lich su cho an</h2>
    <table>
        <thead>
            <tr>
                <th>Thoi gian</th>
                <th>Luong thuc an</th>
            </tr>
        </thead>
        <tbody id="feedHistory"></tbody>
    </table>

    <h2>Lich su bom nuoc</h2>
    <table>
        <thead>
            <tr>
                <th>Thoi gian</th>
                <th>Luong nuoc</th>
            </tr>
        </thead>
        <tbody id="waterHistory"></tbody>
    </table>
</div>

<script>
function updateData(){
    fetch("/data?nocache=" + Date.now())
    .then(r => r.json())
    .then(data => {
        document.getElementById("time").innerText = data.time;
        document.getElementById("date").innerText = data.date;
        document.getElementById("weight").innerText = data.weight;
        document.getElementById("foodLeft").innerText = data.foodLeft;
        document.getElementById("waterLeft").innerText = data.waterLeft;
        document.getElementById("status").innerText = "ESP32 Online";
    })
    .catch(e => {
        document.getElementById("status").innerText = "Mat ket noi";
    });
}

function updateHistory(){
    fetch("/history?nocache=" + Date.now())
    .then(r => r.json())
    .then(data => {
        let f = "";
        data.feed.forEach(x => {
            f += "<tr><td>" + x.time + "</td><td>" + x.gram + " g</td></tr>";
        });
        document.getElementById("feedHistory").innerHTML = f;

        let w = "";
        data.water.forEach(x => {
            w += "<tr><td>" + x.time + "</td><td>" + x.ml + " ml</td></tr>";
        });
        document.getElementById("waterHistory").innerHTML = w;
    });
}

function syncRTC(){
    let now = new Date();

    let y  = now.getFullYear();
    let mo = String(now.getMonth() + 1).padStart(2,'0');
    let d  = String(now.getDate()).padStart(2,'0');
    let h  = String(now.getHours()).padStart(2,'0');
    let mi = String(now.getMinutes()).padStart(2,'0');
    let s  = String(now.getSeconds()).padStart(2,'0');

    let formatted = y + "-" + mo + "-" + d + " " + h + ":" + mi + ":" + s;

    fetch("/set?datetime=" + encodeURIComponent(formatted))
    .then(r => r.text())
    .then(t => {
        document.getElementById("status").innerText = t;
        updateData();
    });
}

function resetKho(){
    fetch("/resetkho?nocache=" + Date.now())
    .then(r => r.text())
    .then(t => {
        document.getElementById("status").innerText = t;
        updateData();
    });
}

function clearHistory(){
    fetch("/clearhistory?nocache=" + Date.now())
    .then(r => r.text())
    .then(t => {
        document.getElementById("status").innerText = t;
        updateHistory();
    });
}

setInterval(updateData, 1000);
setInterval(updateHistory, 3000);

updateData();
updateHistory();
</script>
</body>
</html>
)rawliteral";

// ================= RTC =================
String getDateRTC()
{
    RtcDateTime now = rtc.GetDateTime();
    char buffer[20];
    sprintf(buffer, "%02d/%02d/%04d", now.Day(), now.Month(), now.Year());
    return String(buffer);
}

String getTimeRTC()
{
    RtcDateTime now = rtc.GetDateTime();
    char buffer[20];
    sprintf(buffer, "%02d:%02d:%02d", now.Hour(), now.Minute(), now.Second());
    return String(buffer);
}

String getDateTimeRTC()
{
    return getDateRTC() + " " + getTimeRTC();
}

// ================= LOG =================
void addFeedLog(float gram)
{
    if(gram < 2.0) return;

    if(feedCount >= MAX_LOG)
    {
        for(int i = 1; i < MAX_LOG; i++)
        {
            feedTime[i - 1] = feedTime[i];
            feedGram[i - 1] = feedGram[i];
        }

        feedCount = MAX_LOG - 1;
    }

    feedTime[feedCount] = getDateTimeRTC();
    feedGram[feedCount] = gram;
    feedCount++;
}

void addWaterLog(float ml)
{
    if(ml < 2.0) return;

    if(waterCount >= MAX_LOG)
    {
        for(int i = 1; i < MAX_LOG; i++)
        {
            waterTime[i - 1] = waterTime[i];
            waterMl[i - 1] = waterMl[i];
        }

        waterCount = MAX_LOG - 1;
    }

    waterTime[waterCount] = pumpStartTime;
    waterMl[waterCount] = ml;
    waterCount++;
}

// ================= WEB =================
void sendNoCache()
{
    server.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "0");
}

void handleRoot()
{
    sendNoCache();
    server.send_P(200, "text/html", MAIN_page);
}

void handleData()
{
    sendNoCache();

    String json = "{";
    json += "\"date\":\"" + getDateRTC() + "\",";
    json += "\"time\":\"" + getTimeRTC() + "\",";
    json += "\"weight\":" + String(currentWeight, 1) + ",";
    json += "\"foodLeft\":" + String(foodLeft, 0) + ",";
    json += "\"waterLeft\":" + String(waterLeft, 0);
    json += "}";

    server.send(200, "application/json", json);
}

void handleHistory()
{
    sendNoCache();

    String json = "{";

    json += "\"feed\":[";

    for(int i = feedCount - 1; i >= 0; i--)
    {
        json += "{";
        json += "\"time\":\"" + feedTime[i] + "\",";
        json += "\"gram\":" + String(feedGram[i], 1);
        json += "}";

        if(i > 0) json += ",";
    }

    json += "],";

    json += "\"water\":[";

    for(int i = waterCount - 1; i >= 0; i--)
    {
        json += "{";
        json += "\"time\":\"" + waterTime[i] + "\",";
        json += "\"ml\":" + String(waterMl[i], 1);
        json += "}";

        if(i > 0) json += ",";
    }

    json += "]";

    json += "}";

    server.send(200, "application/json", json);
}

void handleSet()
{
    sendNoCache();

    if(!server.hasArg("datetime"))
    {
        server.send(400, "text/plain", "Khong co du lieu");
        return;
    }

    String data = server.arg("datetime");

    int year, month, day, hour, minute, second;

    int result = sscanf(
        data.c_str(),
        "%d-%d-%d %d:%d:%d",
        &year,
        &month,
        &day,
        &hour,
        &minute,
        &second
    );

    if(result == 6)
    {
        RtcDateTime newTime(year, month, day, hour, minute, second);
        rtc.SetDateTime(newTime);
        server.send(200, "text/plain", "Da dong bo RTC");
    }
    else
    {
        server.send(400, "text/plain", "Sai dinh dang");
    }
}

void handleResetKho()
{
    sendNoCache();

    tongThucAnDaRa = 0;
    tongNuocDaRa = 0;

    foodLeft = khoThucAnDay;
    waterLeft = binhNuocDay;

    feedingActive = false;
    pumpActive = false;

    digitalWrite(relayPin, LOW);
    myServo.write(0);

    server.send(200, "text/plain", "Da reset kho");
}

void handleClearHistory()
{
    sendNoCache();

    feedCount = 0;
    waterCount = 0;

    server.send(200, "text/plain", "Da xoa lich su");
}

// ================= SETUP =================
void setup()
{
    Serial.begin(115200);

    pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin, LOW);

    myServo.attach(servoPin);
    myServo.write(0);

    lcd.begin(16, 2);
    lcd.clear();
    lcd.print("System Start");

    scale.begin(HX711_DT, HX711_SCK);

    delay(1000);

    scale.set_scale(calibration_factor);
    scale.tare(5);

    rtc.Begin();
    rtc.SetIsWriteProtected(false);
    rtc.SetIsRunning(true);

    WiFi.begin(ssid, password);

    lcd.clear();
    lcd.print("WiFi...");

    while(WiFi.status() != WL_CONNECTED)
    {
        delay(300);
        Serial.print(".");
    }

    Serial.println();
    Serial.print("IP ESP32: ");
    Serial.println(WiFi.localIP());

    lcd.clear();
    lcd.print("IP:");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());

    server.on("/", handleRoot);
    server.on("/data", handleData);
    server.on("/history", handleHistory);
    server.on("/set", handleSet);
    server.on("/resetkho", handleResetKho);
    server.on("/clearhistory", handleClearHistory);

    server.begin();

    delay(2000);
    lcd.clear();

    lastSensorMillis = millis();
    lastLcdMillis = millis();
    lastSerialMillis = millis();
}

// ================= LOOP =================
void loop()
{
    server.handleClient();
    delay(1);

    unsigned long nowMillis = millis();

    if(nowMillis - lastSensorMillis >= 1000)
    {
        float dt = (nowMillis - lastSensorMillis) / 1000.0;
        lastSensorMillis = nowMillis;

        // ================= DOC ADC NUOC =================
        analogValue = analogRead(analogPin);

        bool oldPumpActive = pumpActive;

        if(waterLeft <= 0)
        {
            digitalWrite(relayPin, LOW);
            pumpActive = false;
            waterLeft = 0;
        }
        else
        {
            if(analogValue <= mucDuoi5cm)
            {
                digitalWrite(relayPin, HIGH);
                pumpActive = true;
            }
            else if(analogValue >= mucTren9cm)
            {
                digitalWrite(relayPin, LOW);
                pumpActive = false;
            }
        }

        if(pumpActive && !oldPumpActive)
        {
            pumpStartMillis = nowMillis;
            pumpStartTime = getDateTimeRTC();
            waterLeftAtPumpStart = waterLeft;
        }

        if(pumpActive)
        {
            float waterUsedThisTick = dt * luuLuongBom;

            if(waterUsedThisTick > waterLeft)
            {
                waterUsedThisTick = waterLeft;
            }

            waterLeft -= waterUsedThisTick;

            if(waterLeft <= 0)
            {
                waterLeft = 0;
                digitalWrite(relayPin, LOW);
                pumpActive = false;
            }
        }

        if(!pumpActive && oldPumpActive)
        {
            float pumpMl = waterLeftAtPumpStart - waterLeft;

            if(pumpMl < 0)
            {
                pumpMl = 0;
            }

            if(pumpMl > waterLeftAtPumpStart)
            {
                pumpMl = waterLeftAtPumpStart;
            }

            tongNuocDaRa += pumpMl;
            addWaterLog(pumpMl);
        }

        // ================= DOC CAN =================
        if(scale.is_ready())
        {
            currentWeight = scale.get_units(1);

            if(currentWeight < 0)
            {
                currentWeight = 0;
            }
        }

        bool oldFeedingActive = feedingActive;

        if(foodLeft <= 0)
        {
            foodLeft = 0;
            myServo.write(0);
            feedingActive = false;
        }
        else
        {
            if(currentWeight < 100.0)
            {
                myServo.write(180);
                feedingActive = true;
            }
            else
            {
                myServo.write(0);
                feedingActive = false;
            }
        }

        if(feedingActive && !oldFeedingActive)
        {
            feedStartWeight = currentWeight;
        }

        if(feedingActive)
        {
            float feedNow = currentWeight - feedStartWeight;

            if(feedNow < 0)
            {
                feedNow = 0;
            }

            float maxCanFeed = khoThucAnDay - tongThucAnDaRa;

            if(maxCanFeed < 0)
            {
                maxCanFeed = 0;
            }

            if(feedNow > maxCanFeed)
            {
                feedNow = maxCanFeed;
            }

            foodLeft = khoThucAnDay - tongThucAnDaRa - feedNow;

            if(foodLeft <= 0)
            {
                foodLeft = 0;
                myServo.write(0);
                feedingActive = false;
            }
        }

        if(!feedingActive && oldFeedingActive)
        {
            float feedAmount = currentWeight - feedStartWeight;

            if(feedAmount < 0)
            {
                feedAmount = 0;
            }

            float maxCanFeed = khoThucAnDay - tongThucAnDaRa;

            if(maxCanFeed < 0)
            {
                maxCanFeed = 0;
            }

            if(feedAmount > maxCanFeed)
            {
                feedAmount = maxCanFeed;
            }

            tongThucAnDaRa += feedAmount;

            foodLeft = khoThucAnDay - tongThucAnDaRa;

            if(foodLeft <= 0)
            {
                foodLeft = 0;
            }

            addFeedLog(feedAmount);
        }
    }

    // ================= LCD =================
    if(nowMillis - lastLcdMillis >= 1000)
    {
        lastLcdMillis = nowMillis;

        lcd.setCursor(0, 0);
        lcd.print("Food:");
        lcd.print(foodLeft, 0);
        lcd.print("g W:");
        lcd.print(currentWeight, 0);
        lcd.print("g   ");

        lcd.setCursor(0, 1);
        lcd.print("Water:");
        lcd.print(waterLeft, 0);
        lcd.print("ml ");

        if(WiFi.status() == WL_CONNECTED)
        {
            lcd.print("OK ");
        }
        else
        {
            lcd.print("OFF");
        }
    }

    // ================= SERIAL DEBUG =================
    if(nowMillis - lastSerialMillis >= 1000)
    {
        lastSerialMillis = nowMillis;

        Serial.print("ADC nuoc: ");
        Serial.print(analogValue);

        Serial.print(" | Relay: ");
        Serial.print(pumpActive ? "ON" : "OFF");

        Serial.print(" | WaterLeft: ");
        Serial.print(waterLeft, 1);
        Serial.print(" ml");

        Serial.print(" | Weight: ");
        Serial.print(currentWeight, 1);
        Serial.print(" g");

        Serial.print(" | FoodLeft: ");
        Serial.print(foodLeft, 1);
        Serial.print(" g");

        Serial.print(" | WiFi: ");
        Serial.println(WiFi.status() == WL_CONNECTED ? "OK" : "OFF");
    }
}