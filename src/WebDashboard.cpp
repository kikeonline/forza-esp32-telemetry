#include "WebDashboard.h"

#include <WiFi.h>

namespace {
const char DashboardHtml[] PROGMEM = R"HTML(
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Forza RPM</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
<link href="https://fonts.googleapis.com/css2?family=Doto:wght@500;700&family=Space+Grotesk:wght@300;400;500;700&family=Space+Mono:wght@400;700&display=swap" rel="stylesheet">
<style>
:root{color-scheme:dark;--black:#000;--surface:#111;--raised:#1a1a1a;--border:#222;--visible:#333;--disabled:#666;--secondary:#999;--primary:#e8e8e8;--display:#fff;--accent:#d71921;--success:#4a9e5c;--warning:#d4a843;--blue:#5b9bf6}
*{box-sizing:border-box}body{margin:0;background:var(--black);color:var(--primary);font-family:"Space Grotesk","DM Sans",system-ui,sans-serif}
main{max-width:1080px;margin:0 auto;padding:24px 18px 40px}.top{display:flex;align-items:flex-start;justify-content:space-between;gap:24px;margin-bottom:48px}
h1{margin:0;color:var(--display);font-family:"Doto","Space Mono",monospace;font-size:clamp(44px,9vw,88px);font-weight:700;line-height:1;letter-spacing:0}
.kicker,.label,label span,.meta{font-family:"Space Mono","JetBrains Mono",monospace;font-size:11px;line-height:1.2;letter-spacing:.08em;text-transform:uppercase;color:var(--secondary)}
.ip{text-align:right;color:var(--secondary);font-family:"Space Mono","JetBrains Mono",monospace;font-size:12px}.status-dot{display:inline-block;width:8px;height:8px;margin-right:8px;background:var(--disabled);border-radius:50%}.status-dot.on{background:var(--success)}.status-dot.wait{background:var(--warning)}
section{padding:32px 0;border-top:1px solid var(--border)}.hero{display:grid;grid-template-columns:minmax(0,1.35fr) minmax(280px,.65fr);gap:32px;align-items:end}
.rpm-read{display:flex;align-items:flex-start;gap:12px}.rpm-number{color:var(--display);font-family:"Space Mono","JetBrains Mono",monospace;font-size:clamp(58px,14vw,132px);line-height:.9;letter-spacing:0}.rpm-unit{margin-top:14px;color:var(--secondary);font-family:"Space Mono","JetBrains Mono",monospace;font-size:12px;text-transform:uppercase;letter-spacing:.08em}
.segments{display:grid;grid-template-columns:repeat(24,1fr);gap:2px;margin-top:28px}.seg{height:18px;background:var(--border)}.seg.on{background:var(--display)}
.metrics{display:grid;gap:0;border-top:1px solid var(--visible);border-bottom:1px solid var(--border)}.stat{display:grid;grid-template-columns:1fr auto;gap:16px;align-items:center;padding:14px 0;border-bottom:1px solid var(--border)}.stat:last-child{border-bottom:0}.value{font-family:"Space Mono","JetBrains Mono",monospace;color:var(--display);font-size:18px;text-align:right}
.settings-grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:16px 24px}.field{min-width:0}.field.full{grid-column:1/-1}label{display:grid;gap:10px}
input{width:100%;accent-color:var(--display)}input[type=number]{height:44px;background:transparent;border:1px solid var(--visible);border-radius:8px;color:var(--display);padding:10px 12px;font:400 15px "Space Mono","JetBrains Mono",monospace}input[type=range]{height:28px}
.row{display:grid;grid-template-columns:1fr 86px;gap:12px;align-items:center}.mode{display:grid;grid-template-columns:repeat(3,1fr);border:1px solid var(--visible);border-radius:8px;overflow:hidden}.mode input{position:absolute;opacity:0;pointer-events:none}.mode label{display:block;min-height:44px;padding:15px 10px;text-align:center;color:var(--secondary);font:400 11px/1.2 "Space Mono","JetBrains Mono",monospace;letter-spacing:.08em;text-transform:uppercase;border-right:1px solid var(--visible);cursor:pointer}.mode label:last-child{border-right:0}.mode input:checked+label{background:var(--display);color:var(--black)}
.actions{display:flex;gap:10px;flex-wrap:wrap;align-items:center;margin-top:24px}button{min-height:44px;border:1px solid var(--visible);border-radius:999px;background:transparent;color:var(--primary);padding:12px 22px;font:400 13px "Space Mono","JetBrains Mono",monospace;letter-spacing:.06em;text-transform:uppercase;cursor:pointer}button.primary{background:var(--display);border-color:var(--display);color:var(--black)}button:hover{border-color:var(--display);color:var(--display)}button.primary:hover{color:var(--black)}.message{font-family:"Space Mono","JetBrains Mono",monospace;font-size:12px;color:var(--secondary)}
@media(max-width:760px){main{padding:18px 14px 32px}.top{display:block;margin-bottom:36px}.ip{text-align:left;margin-top:14px}.hero{grid-template-columns:1fr;gap:28px}.settings-grid{grid-template-columns:1fr}.segments{grid-template-columns:repeat(12,1fr)}}
</style>
</head>
<body>
<main>
<header class="top"><div><div class="kicker">ESP32 telemetry</div><h1>Forza RPM</h1></div><div class="ip" id="ip">--</div></header>
<section class="hero">
<div>
<div class="label">Engine speed</div>
<div class="rpm-read"><div class="rpm-number" id="rpmNow">--</div><div class="rpm-unit">rpm<br><span id="rpmMax">-- max</span></div></div>
<div class="segments" id="rpmsegments"></div>
</div>
<div class="metrics">
<div class="stat"><span class="label">Race</span><span class="value" id="race">--</span></div>
<div class="stat"><span class="label">LED mode</span><span class="value" id="modeStatus">--</span></div>
<div class="stat"><span class="label">Packet</span><span class="value" id="packet">--</span></div>
<div class="stat"><span class="label">Wi-Fi</span><span class="value" id="wifi">--</span></div>
</div>
</section>
<section>
<form id="settings">
<div class="settings-grid">
<div class="field"><label><span>UDP port</span><input name="udpPort" type="number" min="1024" max="65535"></label></div>
<div class="field"><label><span>LED count</span><input name="ledCount" type="number" min="1" max="300"></label></div>
<div class="field full"><span class="label">LED RPM mode</span><div class="mode"><input id="modeFill" name="ledMode" type="radio" value="0"><label for="modeFill">Fill</label><input id="modeSolid" name="ledMode" type="radio" value="1"><label for="modeSolid">Solid</label><input id="modeCenter" name="ledMode" type="radio" value="2"><label for="modeCenter">Center</label></div></div>
<div class="field"><label><span>Brightness</span><div class="row"><input name="brightness" type="range" min="1" max="255"><input name="brightnessNumber" type="number" min="1" max="255"></div></label></div>
<div class="field"><label><span>Green starts %</span><div class="row"><input name="greenThreshold" type="range" min="1" max="95"><input name="greenThresholdNumber" type="number" min="1" max="95"></div></label></div>
<div class="field"><label><span>Yellow starts %</span><div class="row"><input name="yellowThreshold" type="range" min="1" max="98"><input name="yellowThresholdNumber" type="number" min="1" max="98"></div></label></div>
<div class="field"><label><span>Red starts %</span><div class="row"><input name="redThreshold" type="range" min="1" max="99"><input name="redThresholdNumber" type="number" min="1" max="99"></div></label></div>
<div class="field"><label><span>Red blink starts %</span><div class="row"><input name="redBlinkThreshold" type="range" min="1" max="100"><input name="redBlinkThresholdNumber" type="number" min="1" max="100"></div></label></div>
</div>
<div class="actions">
<button class="primary" type="submit">Save</button>
<button type="button" id="test">Test strip</button>
<button type="button" id="off">Turn off</button>
<button type="button" id="reset">Reset defaults</button>
<span class="message" id="message"></span>
</div>
</form>
</section>
</main>
<script>
const form=document.querySelector("#settings"),msg=document.querySelector("#message");
let currentSettings={greenThreshold:.3,yellowThreshold:.65,redThreshold:.8,ledMode:0};
const modeNames=["Fill","Solid","Center"];
const pair=(a,b)=>{const x=form.elements[a],y=form.elements[b];x.addEventListener("input",()=>y.value=x.value);y.addEventListener("input",()=>x.value=y.value)};
["brightness","greenThreshold","yellowThreshold","redThreshold","redBlinkThreshold"].forEach(n=>pair(n,n+"Number"));
const pct=v=>Math.round(v*100),$=s=>document.querySelector(s);
const segments=$("#rpmsegments");for(let i=0;i<24;i++){const el=document.createElement("span");el.className="seg";segments.appendChild(el)}
function rpmColor(p){if(p>=currentSettings.redThreshold)return"#d71921";if(p>=currentSettings.yellowThreshold)return"#d4a843";if(p>=currentSettings.greenThreshold)return"#4a9e5c";return"#5b9bf6"}
function paintSegments(p){const color=rpmColor(p),lit=Math.ceil(p*24);[...segments.children].forEach((el,i)=>{el.className=i<lit?"seg on":"seg";el.style.background=i<lit?color:""})}
async function loadSettings(){const s=await fetch("/api/settings").then(r=>r.json());currentSettings=s;for(const [k,v]of Object.entries(s)){if(k==="ledMode"){const selected=form.querySelector(`[name=ledMode][value="${v}"]`);if(selected)selected.checked=true;continue}const displayValue=k.includes("Threshold")?pct(v):v;if(form.elements[k])form.elements[k].value=displayValue;if(form.elements[k+"Number"])form.elements[k+"Number"].value=displayValue}$("#modeStatus").textContent=modeNames[s.ledMode]||"--"}
async function loadStatus(){try{const s=await fetch("/api/status").then(r=>r.json());$("#ip").innerHTML=`<span class="status-dot ${s.timedOut?"wait":"on"}"></span>${s.ip}:80`;$("#race").textContent=s.timedOut?"Waiting":(s.raceOn?"On":"Off");$("#rpmNow").textContent=Math.round(s.currentRpm);$("#rpmMax").textContent=Math.round(s.maxRpm)+" max";$("#packet").textContent=s.packetSize+" B";$("#wifi").textContent=s.rssi+" dBm";paintSegments(s.rpmPercent)}catch(e){}}
form.addEventListener("submit",async e=>{e.preventDefault();msg.textContent="[SAVING]";const body=new URLSearchParams(new FormData(form));const r=await fetch("/api/settings",{method:"POST",body});msg.textContent=r.ok?"[SAVED]":"[ERROR]";await loadSettings()});
document.querySelector("#reset").onclick=async()=>{await fetch("/api/reset",{method:"POST"});msg.textContent="[DEFAULTS]";await loadSettings()};
document.querySelector("#test").onclick=async()=>{await fetch("/api/led-test?action=startup",{method:"POST"});msg.textContent="[TEST SENT]"};
document.querySelector("#off").onclick=async()=>{await fetch("/api/led-test?action=off",{method:"POST"});msg.textContent="[STRIP OFF]"};
loadSettings();loadStatus();setInterval(loadStatus,1000);
</script>
</body>
</html>
)HTML";

float clampFloat(float value, float minValue, float maxValue) {
  if (!isfinite(value)) {
    return minValue;
  }
  return constrain(value, minValue, maxValue);
}
}  // namespace

WebDashboard::WebDashboard(DeviceSettings& settings,
                           TelemetryStatus& status,
                           SettingsStore& settingsStore,
                           RpmLeds& rpmLeds)
    : server_(80),
      settings_(settings),
      status_(status),
      settingsStore_(settingsStore),
      rpmLeds_(rpmLeds) {}

void WebDashboard::begin() {
  server_.on("/", HTTP_GET, [this]() { handleRoot(); });
  server_.on("/api/settings", HTTP_GET, [this]() { handleSettingsJson(); });
  server_.on("/api/settings", HTTP_POST, [this]() { handleSaveSettings(); });
  server_.on("/api/status", HTTP_GET, [this]() { handleStatusJson(); });
  server_.on("/api/reset", HTTP_POST, [this]() { handleResetSettings(); });
  server_.on("/api/led-test", HTTP_POST, [this]() { handleLedTest(); });
  server_.onNotFound([this]() { handleNotFound(); });
  server_.begin();
}

void WebDashboard::handleClient() {
  server_.handleClient();
}

bool WebDashboard::consumeSettingsChanged() {
  const bool changed = settingsChanged_;
  settingsChanged_ = false;
  return changed;
}

void WebDashboard::handleRoot() {
  server_.send_P(200, "text/html", DashboardHtml);
}

void WebDashboard::handleSettingsJson() {
  server_.send(200, "application/json", settingsJson());
}

void WebDashboard::handleStatusJson() {
  server_.send(200, "application/json", statusJson());
}

void WebDashboard::handleSaveSettings() {
  settings_.udpPort = argUInt16("udpPort", settings_.udpPort, 1024, 65535);
  settings_.ledCount = argUInt16("ledCount", settings_.ledCount, 1, 300);
  settings_.brightness = argUInt8("brightness", settings_.brightness, 1, 255);
  settings_.ledMode = argRpmLedMode("ledMode", settings_.ledMode);
  settings_.greenThreshold = argPercent("greenThreshold", settings_.greenThreshold);
  settings_.yellowThreshold = argPercent("yellowThreshold", settings_.yellowThreshold);
  settings_.redThreshold = argPercent("redThreshold", settings_.redThreshold);
  settings_.redBlinkThreshold = argPercent("redBlinkThreshold", settings_.redBlinkThreshold);

  settings_.yellowThreshold = max(settings_.yellowThreshold, settings_.greenThreshold + 0.01f);
  settings_.redThreshold = max(settings_.redThreshold, settings_.yellowThreshold + 0.01f);
  settings_.redBlinkThreshold = max(settings_.redBlinkThreshold, settings_.redThreshold);
  settings_.redBlinkThreshold = min(settings_.redBlinkThreshold, 1.0f);

  settingsStore_.save(settings_);
  settingsChanged_ = true;
  server_.send(200, "application/json", settingsJson());
}

void WebDashboard::handleResetSettings() {
  settingsStore_.reset();
  settings_ = DeviceSettings();
  settingsStore_.save(settings_);
  settingsChanged_ = true;
  server_.send(200, "application/json", settingsJson());
}

void WebDashboard::handleLedTest() {
  const String action = server_.arg("action");
  if (action == "startup") {
    rpmLeds_.startupTest(120);
  } else {
    rpmLeds_.off();
  }
  server_.send(200, "application/json", "{\"ok\":true}");
}

void WebDashboard::handleNotFound() {
  server_.send(404, "application/json", "{\"error\":\"not found\"}");
}

uint16_t WebDashboard::argUInt16(const char* name,
                                 uint16_t fallback,
                                 uint16_t minValue,
                                 uint16_t maxValue) {
  if (!server_.hasArg(name)) {
    return fallback;
  }
  const long value = server_.arg(name).toInt();
  return constrain(value, static_cast<long>(minValue), static_cast<long>(maxValue));
}

uint8_t WebDashboard::argUInt8(const char* name,
                               uint8_t fallback,
                               uint8_t minValue,
                               uint8_t maxValue) {
  if (!server_.hasArg(name)) {
    return fallback;
  }
  const long value = server_.arg(name).toInt();
  return static_cast<uint8_t>(
      constrain(value, static_cast<long>(minValue), static_cast<long>(maxValue)));
}

RpmLedMode WebDashboard::argRpmLedMode(const char* name, RpmLedMode fallback) {
  if (!server_.hasArg(name)) {
    return fallback;
  }
  const int value = server_.arg(name).toInt();
  if (value < static_cast<int>(RpmLedMode::Fill) ||
      value > static_cast<int>(RpmLedMode::Center)) {
    return fallback;
  }
  return static_cast<RpmLedMode>(value);
}

float WebDashboard::argPercent(const char* name, float fallback) {
  if (!server_.hasArg(name)) {
    return fallback;
  }
  return clampFloat(server_.arg(name).toFloat() / 100.0f, 0.0f, 1.0f);
}

String WebDashboard::settingsJson() const {
  String json = "{";
  json += "\"udpPort\":" + String(settings_.udpPort);
  json += ",\"ledCount\":" + String(settings_.ledCount);
  json += ",\"brightness\":" + String(settings_.brightness);
  json += ",\"ledMode\":" + String(static_cast<uint8_t>(settings_.ledMode));
  json += ",\"greenThreshold\":" + String(settings_.greenThreshold, 3);
  json += ",\"yellowThreshold\":" + String(settings_.yellowThreshold, 3);
  json += ",\"redThreshold\":" + String(settings_.redThreshold, 3);
  json += ",\"redBlinkThreshold\":" + String(settings_.redBlinkThreshold, 3);
  json += "}";
  return json;
}

String WebDashboard::statusJson() const {
  String json = "{";
  json += "\"ip\":\"" + WiFi.localIP().toString() + "\"";
  json += ",\"rssi\":" + String(WiFi.RSSI());
  json += ",\"uptimeMs\":" + String(millis());
  json += ",\"raceOn\":" + String(status_.raceOn ? "true" : "false");
  json += ",\"timedOut\":" + String(status_.timedOut ? "true" : "false");
  json += ",\"currentRpm\":" + String(status_.currentRpm, 1);
  json += ",\"maxRpm\":" + String(status_.maxRpm, 1);
  json += ",\"rpmPercent\":" + String(status_.rpmPercent, 3);
  json += ",\"packetSize\":" + String(status_.packetSize);
  json += ",\"lastPacketAgeMs\":" + String(status_.lastPacketAgeMs);
  json += ",\"layout\":\"" + String(status_.layoutName) + "\"";
  json += "}";
  return json;
}
