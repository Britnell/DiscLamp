#ifndef CONNECT_H
#define CONNECT_H

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
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

void wifi_setup(){
  Serial.begin(115200);
  Serial1.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(5000);
    ESP.restart();
  }
  
  print_ip();

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
    server.handleClient();
}

#endif
