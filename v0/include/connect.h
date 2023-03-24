#ifndef PIXEL_WIFI_CONNECT
#define PIXEL_WIFI_CONNECT

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// - WIfi
ESP8266WebServer server(80);

const char* ssid = STASSID;
const char* password = STAPSK;

String MODE = "rain";
unsigned short bright = 100;
unsigned short hue = 0;
short invert = 0;

const char * html = R"rawliteral(
<!DOCTYPE HTML><html><head><meta name="viewport" content="width=device-width, initial-scale=1">
<style>
body {
  color: white;
  background-color: black;
}
container {
  height: 100vh;
  display: flex;
  flex-direction: column;
  justify-content: center;
  align-items: center;
}
container div {
  margin-bottom: 20px;
}
#dimmer {
  width: 80vw;
  max-width: 400px;
}
#hue {
  width: 80vw;
  max-width: 400px;
}
</style>
</head>

<body>
<container>
<div>
    <h2>PIXEL LAMP</h2>
</div>
<div>
  <label>MODE</label>
  <select name="mode" id="mode" onchange="mode_change(event)"></select>
  <button onclick="nextmode()">Next</button>
  <!-- <input type="range" min="0" max="255" value="0" id="mode"  > -->
</div>
<div>
    <label>DIM </label>
    <input id="dimmer" type="range" min="0" max="100" value="50" step="5" oninput="dimmer(event)" />
</div>

<div>
  <label>Color </label>
  <input id="hue" type="range" min="0" max="255" value="0" step="5" oninput="hue(event)" />
</div>

<div>
  <label>Invert </label>
  <input id="invert" type="checkbox" onchange="invert(event)" />
</div>
</container>
<script>

function create(tag,options){
  let d = document.createElement(tag)
  for(let k in options)   d[k] = options[k]
  return d;
}
const modes = ['full','half','square','triangle','cross','lines','hatch','hatch2','arrow','line','scroll_lines','waves','stripes']
const mode = document.getElementById('mode')
modes.forEach(m=> mode.appendChild(create('option',{value: m, textContent: m})) )

function mode_change(ev){
  fetch(`/mode?val=${ev.target.value}`)
    .then(res=>res.text())
    .then(txt=>console.log(txt))
}

function nextmode(){
    let opt = document.getElementById('mode')
    let s = opt.selectedIndex +1
    if(s>=opt.options.length)  s = 0;
    opt.selectedIndex = s
    fetch(`/mode?val=${opt.value}`)
      .then(res=>res.text())
      .then(txt=>console.log(txt))
}

function dimmer(ev){
    let val = ev.target.value;
    fetch(`/bright?val=${val}`)
      .then(res=>res.text())
      .then(txt=>console.log(txt))
}

function hue(ev){
    let val = ev.target.value
    fetch(`/hue?val=${val}`)
      .then(res=>res.text())
      .then(txt=>console.log(txt))
}
function invert(ev){
    let val = ev.target.checked ? 1 : 0;
    fetch(`/invert?val=${val}`)
      .then(res=>res.text())
      .then(txt=>console.log(txt))
}
</script>
</body></html>
)rawliteral";


void handleRoot() {
  server.send(200, "text/html", html);
}

void handleNotFound(){
  server.send(404, "text/plain", " / 505 - NOT FOUND BRO \n\n");  
}

void handleVar(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(200, "text/plain", message);
}


void handleMode(){
  
  for(uint8_t i=0; i<server.args(); i++){
    if(server.argName(i).equals("val"))
        MODE = server.arg(i); 
  }
  String message = "MODE=" + String(MODE);
  server.send(200, "text/plain", message);
}

void handleBright(){
  for(uint8_t i=0; i<server.args(); i++){
    if(server.argName(i).equals("val"))
        bright = server.arg(i).toInt(); 
  }
  String message = "bright=" + String(bright);
  server.send(200, "text/plain", message);
}

void handleHue(){
  for(uint8_t i=0; i<server.args(); i++){
    if(server.argName(i).equals("val"))
      hue = server.arg(i).toInt(); 
  }
  String message = "hue=" + String(hue);
  server.send(200, "text/plain", message);
}

void handleInv(){
  for(uint8_t i=0; i<server.args(); i++){
    if(server.argName(i).equals("val"))
      invert = server.arg(i).toInt(); 
  }
  String message = "inv=" + String(invert);
  server.send(200, "text/plain", message);
}


// *

void connect_wifi(){

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(5000);
    ESP.restart();
  }
  
  Serial.println("OYTA");

  ArduinoOTA.setHostname("LAMP-ESP");
  // ArduinoOTA.setPassword("admin");
  ArduinoOTA.begin();
  
  Serial.println("server");

  server.on("/", handleRoot);
  server.on("/mode",handleMode);
  server.on("/bright",handleBright);
  server.on("/hue",handleHue);
  server.on("/invert",handleInv);

  server.on("/var",handleVar);

  server.onNotFound(handleNotFound);

  server.begin();
  
}

void check_ota(){
  ArduinoOTA.handle();
}

void check_server(){
    server.handleClient();
}

#endif