#ifndef CONNECT_H
#define CONNECT_H

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <time.h>
#include "html.h"
// #include "pattern.h"
#include "secrets.h"


// - WIfi
ESP8266WebServer server(80);
const char* ssid = STASSID;
const char* password = STAPSK;

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


void handleSet(){
    String message = "";
    for(uint8_t i=0; i<server.args(); i++)
    {
        message = server.argName(i)  + server.arg(i) + "\n";
        Serial1.print(message);
    }
    server.send(200, "text/plain", "OK");
}


void print_ip(){
  Serial.print(" wifi connected : ");
  Serial.println(WiFi.localIP());
}

void send_time(){
  time_t now = time(nullptr);
  if(now < 100000) return; // NTP not synced yet
  struct tm* t = localtime(&now);
  char buf[20];
  snprintf(buf, sizeof(buf), "t%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
  Serial.printf(" TIME %02d:%02d:%02d  \n",t->tm_hour, t->tm_min, t->tm_sec);
  Serial1.println(buf);
}

void wifi_setup(){
  Serial.begin(115200);
  Serial1.begin(115200);
  Serial.println("SEtUP==");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(5000);
    ESP.restart();
  }

  print_ip();

  MDNS.begin("lamp");

  configTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.nist.gov");
  // wait up to 5s for NTP sync
  for(int i = 0; i < 50 && time(nullptr) < 100000; i++) delay(100);
  send_time();

  //   ArduinoOTA.setHostname("LAMP-ESP");
  //   ArduinoOTA.setPassword("admin");
  //   ArduinoOTA.begin();
    
  server.on("/", handleRoot);
  server.on("/set",handleSet);
  server.on("/var",handleVar);
  server.onNotFound(handleNotFound);

  server.begin();
}


void wifi_loop(){
    MDNS.update();
    server.handleClient();
    static unsigned long last_time_send = 0;
    unsigned long now_ms = millis();
    if(now_ms - last_time_send >= 10000){
        last_time_send = now_ms;
        send_time();
    }
}

#endif
