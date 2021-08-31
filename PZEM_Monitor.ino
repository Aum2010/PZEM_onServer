#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <PZEM004Tv30.h>

#include "string.h"

#if !defined(PZEM_RX_PIN) && !defined(PZEM_TX_PIN)
#define PZEM_RX_PIN 16
#define PZEM_TX_PIN 17
#endif

#if !defined(PZEM_SERIAL)
#define PZEM_SERIAL Serial2
#endif


#if defined(ESP32)

PZEM004Tv30 pzem(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN);
#elif defined(ESP8266)

#else

PZEM004Tv30 pzem(PZEM_SERIAL);
#endif

const char* ssid     = "ESP8266-Access-Point";
const char* password = "123456789";

AsyncWebServer server(80);


float voltage = 0;
float current = 0;
float power = 0;
float energy = 0;
float frequency = 0; 
float pf = 0;

char buff[200];


const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>

<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 2.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1rem;
      vertical-align:middle;
      padding-bottom: 2px;
    }
  </style>
</head>
<body>
  <p>
    <span class="dht-labels">Voltage</span> 
    <span id="vol">%VOLTAGE%</span>
    <sup class="units">volt</sup>
  </p>
  <p>
    <span class="dht-labels">Current</span>
    <span id="cur">%CURRENT%</span>
    <sup class="units">ampare</sup>
  </p>
  <p>
    <span class="dht-labels">Freq</span>
    <span id="freq">%FREQ%</span>
    <sup class="units">Hz</sup>
  </p>
    <p>
    <span class="dht-labels">Power</span>
    <span id="pow">%POWER%</span>
    <sup class="units">W</sup>
  </p>
    <p>
    <span class="dht-labels">PowerFactor</span>
    <span id="pf">%PF%</span>
    <sup class="units"></sup>
  </p>
    <p>
    <span class="dht-labels">Energy</span>
    <span id="energy">%ENERGY%</span>
    <sup class="units">kWh</sup>
  </p>
  
</body>

<script>

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("vol").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/voltage", true);
  xhttp.send();
}, 1000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("cur").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/current", true);
  xhttp.send();
}, 1000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("freq").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/frequency", true);
  xhttp.send();
}, 1000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("pow").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/pow", true);
  xhttp.send();
}, 1000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("pf").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/pf", true);
  xhttp.send();
}, 1000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("energy").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/energy", true);
  xhttp.send();
}, 1000 ) ;

</script>
</html>
)rawliteral";

String processor(const String& var){
  //Serial.println(var);
  if(var == "VOLTAGE"){
    return String(voltage);
  }
  else if(var == "CURRENT"){
    return String(current);
  }
  else if(var == "POWER"){
    return String(power);
  }
  else if(var == "ENERGY"){
    return String(energy);
  }
  else if(var == "FREQ"){
    return String(frequency);
  }
  else if(var == "PF"){
    return String(pf);
  }
  return String();
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());

  sprintf(buff , "{\"Voltage\":\"0.00\"}");

  server.on("/pzem", HTTP_GET, [](AsyncWebServerRequest *request) {
//    request->send(200, "application/json", "{\"message\":\"Welcome\"}");
      request->send(200, "application/json", buff);
  });

  server.on("/auto", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html,processor);
  });

  server.on("/voltage", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(voltage).c_str());
  });

  server.on("/current", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(current).c_str());
  });

  server.on("/energy", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(energy).c_str());
  });

  server.on("/frequency", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(frequency).c_str());
  });

  server.on("/pf", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(pf).c_str());
  });

  server.on("/power", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(power).c_str());
  });

  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
    voltage = pzem.voltage();
    current = pzem.current();
    power = pzem.power();
    energy = pzem.energy();
    frequency = pzem.frequency();
    pf = pzem.pf();
  
      if(isnan(voltage)){
        Serial.println("Error reading voltage");
    } else if (isnan(current)) {
        Serial.println("Error reading current");
    } else if (isnan(power)) {
        Serial.println("Error reading power");
    } else if (isnan(energy)) {
        Serial.println("Error reading energy");
    } else if (isnan(frequency)) {
        Serial.println("Error reading frequency");
    } else if (isnan(pf)) {
        Serial.println("Error reading power factor");
    } else {

        // Print the values to the Serial console
        Serial.print("Voltage: ");      Serial.print(voltage);      Serial.println("V");
        Serial.print("Current: ");      Serial.print(current);      Serial.println("A");
        Serial.print("Power: ");        Serial.print(power);        Serial.println("W");
        Serial.print("Energy: ");       Serial.print(energy,3);     Serial.println("kWh");
        Serial.print("Frequency: ");    Serial.print(frequency, 1); Serial.println("Hz");
        Serial.print("PF: ");           Serial.println(pf);

        sprintf(buff , "{\"voltage\":%f,\"current\":%f,\"power\":%f,\"energy\":%f,\"frequency\":%f,\"powerfac\":%f}",voltage,current,power,energy,frequency,pf);

    }

}
