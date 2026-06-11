#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "DHT.h"


const char* ssid     = "TP Link C64";
const char* password = "Netgear@shihab1209";

#define DHTPIN D2     
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);

ESP8266WebServer server(80);

String csvDataLog = "";
int totalRowsLogged = 0;
float currentTemp = 0.0;
float currentHumid = 0.0;

//UNIFIED PREMIUM INDUSTRIAL WEB INTERFACE

void handleRootRoute() {
  String html = "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>ThermaSense Node Hardware Control</title>";
  html += "<style>";
  html += ":root { --bg-apple: #f5f5f7; --card-bg: rgba(255, 255, 255, 0.85); --text-primary: #1d1d1f; --text-secondary: #86868b; --accent-blue: #0071e3; --radius-premium: 22px; }";
  html += "body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; background-color: var(--bg-apple); color: var(--text-primary); margin: 0; padding: 30px; display: flex; justify-content: center; }";
  html += ".apple-container { max-width: 900px; width: 100%; position: relative; }";
  
  //CLOCK CORNER PANEL STYLING
  html += ".clock-widget { position: absolute; top: 0; right: 0; text-align: right; background: var(--card-bg); padding: 15px 20px; border-radius: 14px; box-shadow: 0 4px 20px rgba(0,0,0,0.02); border: 1px solid rgba(255,255,255,0.5); }";
  html += "#timeDisplay { font-size: 20px; font-weight: 700; font-variant-numeric: tabular-nums; }";
  html += "#dateDisplay { font-size: 12px; color: var(--text-secondary); font-weight: 500; margin-top: 2px; text-transform: uppercase; letter-spacing: 0.5px; }";
  html += ".toggle-btn { background: none; border: 1px solid #d2d2d7; color: var(--accent-blue); font-size: 10px; font-weight: 700; padding: 4px 8px; border-radius: 6px; cursor: pointer; margin-top: 6px; text-transform: uppercase; transition: all 0.2s; }";
  html += ".toggle-btn:hover { background: var(--accent-blue); color: white; border-color: var(--accent-blue); }";

  html += "header { text-align: left; margin-bottom: 35px; border-left: 4px solid var(--accent-blue); padding-left: 20px; min-height: 70px; }";
  html += "header h1 { font-size: 32px; font-weight: 700; letter-spacing: -0.8px; margin: 5px 0; }";
  html += ".badge { background: #e8e8ed; color: var(--accent-blue); padding: 5px 12px; border-radius: 20px; font-size: 11px; font-weight: 700; text-transform: uppercase; }";
  html += ".subtitle { color: var(--text-secondary); font-size: 15px; margin: 0; }";
  
  html += ".grid-layout { display: grid; grid-template-columns: 1fr 1.2fr; gap: 30px; }";
  html += ".apple-card { background: var(--card-bg); backdrop-filter: blur(30px); border-radius: var(--radius-premium); padding: 30px; box-shadow: 0 10px 40px rgba(0, 0, 0, 0.03); border: 1px solid rgba(255, 255, 255, 0.5); display: flex; flex-direction: column; justify-content: space-between; }";
  html += ".card-desc { color: var(--text-secondary); font-size: 13px; margin-bottom: 20px; line-height: 1.4; }";
  
  html += ".metrics-display { display: grid; grid-template-columns: 1fr 1fr; gap: 15px; margin-bottom: 20px; }";
  html += ".m-card { background: #fff; padding: 18px; border-radius: 12px; border: 1px solid #e8e8ed; }";
  html += ".m-card .m-label { display: block; color: var(--text-secondary); font-size: 11px; font-weight: 600; margin-bottom: 3px; }";
  html += ".m-card .m-val { font-size: 28px; font-weight: 700; }";
  
  html += ".log-preview { background: #fff; border-radius: 12px; padding: 15px; border: 1px solid #e8e8ed; font-family: monospace; font-size: 13px; max-height: 140px; overflow-y: auto; white-space: pre-wrap; margin-bottom: 20px; text-align: left; }";
  html += ".apple-btn { background: var(--accent-blue); color: white; border: none; width: 100%; padding: 14px; border-radius: 10px; font-size: 15px; font-weight: 600; cursor: pointer; text-decoration: none; text-align: center; box-sizing: border-box; display: block; transition: background 0.2s; }";
  html += ".apple-btn:hover { background: #0077ed; }";
  html += ".counter { display: inline-block; margin-top: 12px; font-size: 12px; color: var(--text-secondary); font-weight: 600; text-align: center; width: 100%; }";
  html += "</style></head><body>";
  
  html += "<div class='apple-container'>";
  
  //REAL-TIME SYSTEM CLOCK
  html += "  <div class='clock-widget'>";
  html += "    <div id='timeDisplay'>00:00:00</div>";
  html += "    <div id='dateDisplay'>LOADING DATE...</div>";
  html += "    <button class='toggle-btn' onclick='toggleFormat()'>Switch Format</button>";
  html += "  </div>";

  html += "  <header>";
  html += "    <span class='badge'>Hardware Capture Node</span>";
  html += "    <h1>ThermaSense Edge Unit</h1>";
  html += "    <p class='subtitle'>Localized Microclimate Logger Module</p>";
  html += "  </header>";
  
  html += "  <div class='grid-layout'>";
  
  //LEFT PANEL: REAL TIME FEED (Now updates live via AJAX)
  html += "    <div class='apple-card'>";
  html += "      <div>";
  html += "        <h3 style='margin-top:0;'>Live Telemetry</h3>";
  html += "        <p class='card-desc'>Current empirical values gathered from the localized sensor array hardware (Auto-updating).</p>";
  html += "        <div class='metrics-display'>";
  html += "          <div class='m-card'><span class='m-label'>INDOOR TEMP</span><span class='m-val' style='color:#0071e3;' id='liveTemp'>" + String(currentTemp, 1) + "<span style='font-size:16px; font-weight:500;'>°C</span></span></div>";
  html += "          <div class='m-card'><span class='m-label'>INDOOR HUMIDITY</span><span class='m-val' style='color:#34c759;' id='liveHumid'>" + String(currentHumid, 1) + "<span style='font-size:16px; font-weight:500;'>%</span></span></div>";
  html += "        </div>";
  html += "      </div>";
  html += "      <div style='background: #e8e8ed; border-radius: 10px; padding: 10px; text-align: center; font-size: 12px; color: var(--accent-blue); font-weight: 600;'>🟢 System Live Stream Active</div>";
  html += "    </div>";
  
  //RIGHT PANEL: PIPELINE DATA EXTRACTION
  html += "    <div class='apple-card'>";
  html += "      <div>";
  html += "        <h3 style='margin-top:0;'>Dataset Exporter</h3>";
  html += "        <p class='card-desc'>Raw structured telemetry text file ready for deployment to the analytical AI core.</p>";
  html += "        <div class='log-preview' id='logPreview'>" + (csvDataLog == "" ? "Awaiting initialization..." : csvDataLog) + "</div>";
  html += "      </div>";
  html += "      <div>";
  html += "        <a href='/download' class='apple-btn'>Download telemetry.txt</a>";
  html += "        <span class='counter' id='rowCountLabel'>Active Rows Registered: " + String(totalRowsLogged) + "</span>";
  html += "      </div>";
  html += "    </div>";
  
  html += "  </div>"; 
  html += "</div>";   
  
  // AJAX REAL-TIME STREAMING & CLOCK ENGINE
  html += "<script>";
  html += "let is24Hour = false;";
  
  // Clock Function
  html += "function updateClock() {";
  html += "  const now = new Date();";
  html += "  let hours = now.getHours();";
  html += "  const minutes = String(now.getMinutes()).padStart(2, '0');";
  html += "  const seconds = String(now.getSeconds()).padStart(2, '0');";
  html += "  let ampm = '';";
  html += "  if (!is24Hour) {";
  html += "    ampm = hours >= 12 ? ' PM' : ' AM';";
  html += "    hours = hours % 12;";
  html += "    hours = hours ? hours : 12;";
  html += "  }";
  html += "  hours = String(hours).padStart(2, '0');";
  html += "  document.getElementById('timeDisplay').innerText = hours + ':' + minutes + ':' + seconds + ampm;";
  html += "  const days = ['Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday'];";
  html += "  const months = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'];";
  html += "  document.getElementById('dateDisplay').innerText = days[now.getDay()] + ', ' + months[now.getMonth()] + ' ' + now.getDate() + ', ' + now.getFullYear();";
  html += "}";
  html += "function toggleFormat() { is24Hour = !is24Hour; updateClock(); }";
  html += "setInterval(updateClock, 1000); updateClock();";

  // AJAX Live Data Fetching Function (Runs quietly in the background)
  html += "async function fetchLiveData() {";
  html += "  try {";
  html += "    const response = await fetch('/liveData');";
  html += "    const data = await response.json();";
  html += "    document.getElementById('liveTemp').innerHTML = data.temp.toFixed(1) + '<span style=\"font-size:16px; font-weight:500;\">°C</span>';";
  html += "    document.getElementById('liveHumid').innerHTML = data.humid.toFixed(1) + '<span style=\"font-size:16px; font-weight:500;\">%</span>';";
  html += "    document.getElementById('rowCountLabel').innerText = 'Active Rows Registered: ' + data.rows;";
  html += "    if(data.log !== '') { document.getElementById('logPreview').innerText = data.log; }";
  html += "  } catch (err) { console.error('Data stream disconnect:', err); }";
  html += "}";
  html += "setInterval(fetchLiveData, 3000);"; // Polls background endpoints every 3 seconds instantly
  html += "</script>";
  
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

//JSON DATA STREAM ENDPOINT

void handleLiveDataRoute() {
  String json = "{";
  json += "\"temp\":" + String(currentTemp, 1) + ",";
  json += "\"humid\":" + String(currentHumid, 1) + ",";
  json += "\"rows\":" + String(totalRowsLogged) + ",";
  json += "\"log\":\"" + csvDataLog + "\"";
  // Replacing newline breaks with clean JS formatting values to protect standard JSON parser blocks
  json.replace("\n", "\\n"); 
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleDownloadRoute() {
  server.sendHeader("Content-Disposition", "attachment; filename=telemetry.txt");
  server.send(200, "text/plain", "indoor_temp,indoor_humidity\n" + csvDataLog);
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  
  Serial.println("\n📡 Connection Complete");
  Serial.print("🔗 Edge Gateway URL: http://"); Serial.println(WiFi.localIP());

  server.on("/", handleRootRoute);
  server.on("/liveData", handleLiveDataRoute); // Target endpoint for background AJAX engine strings
  server.on("/download", handleDownloadRoute);
  server.begin();
}

void loop() {
  server.handleClient();

  static unsigned long lastSampleTime = 0;
  if (millis() - lastSampleTime >= 5000) { 
    lastSampleTime = millis();

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h)) {
      currentTemp = t;
      currentHumid = h;
      csvDataLog += String(t, 1) + "," + String(h, 1) + "\n";
      totalRowsLogged++;
    }
  }
}