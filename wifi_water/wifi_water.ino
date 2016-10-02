#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define WATER_PIN 5
#define TEMP_PIN 4

const char* host = "casa-calida-watercontrol";
const char* ssid = "home4123";
const char* password = "kiketommy123";
unsigned long MAX_WATER_TIME = 600000UL; //(10 * 60 * 1000); 10min

String html1 = "<!DOCTYPE html>\r\n<html>\r\n\
<head>\r\n<meta content=\"text/html; charset=ISO-8859-1\" http-equiv=\"content-type\">\r\n\
<title>CasaCalida - Water Control</title>\r\n\
</head>\r\n\
<body>\r\n\
<p>Current outside temperature: ";
String html2 = "&deg;C</p><form action=\"";
String html3 = "\">\r\n<input value=\"ON/OFF\" style=\"";
String html4 = " width:5em;height:3em; font-size: 16px;\" type=\"submit\">\
</form>\r\n<br/>\r\n<p>UTC Time: ";
String html5 = "</p>\r\n</body>\r\n</html>";

String updateIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";

ESP8266WebServer server(80);

OneWire oneWire(TEMP_PIN);
DallasTemperature sensors(&oneWire);
DeviceAddress outsideThermometer;

int val = 1;
String Temp = "";
float temp_c;
bool temp_active = false;

unsigned int localPort = 2390;
IPAddress timeServerIP;
const char* ntpServerName = "0.at.pool.ntp.org";
const int NTP_PACKET_SIZE = 48;
byte packetBuffer[ NTP_PACKET_SIZE];
WiFiUDP udp;
unsigned long epoch = 0;
unsigned long millisStart;

unsigned long waterStart;

void setup() {
  digitalWrite(WATER_PIN, val);
  pinMode(WATER_PIN, OUTPUT);
  sensors.begin();

  Serial.begin(115200);
  Serial.println("");
  Serial.println("Starting Wifi");
  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  MDNS.begin(host);

  Serial.println("");

  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices on bus.");

  Serial.print("Parasite power is: "); 
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

  if (!sensors.getAddress(outsideThermometer, 0)) {
    Serial.println("Unable to find address for Device 0"); 
  } else {
    temp_active = true;
    sensors.setResolution(outsideThermometer, 9);
  }

  server.on("/", show_index);
  server.on("/on", handle_on);
  server.on("/off", handle_off);
  server.on("/api/", return_data);
  server.on("/reset", [](){
    server.send(200, "text/plain", "Resetting...");
    delay(50);
    ESP.reset();
  });

  server.on("/update", HTTP_GET, [](){
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", updateIndex);
  });
  server.on("/update", HTTP_POST, [](){
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError())?"FAIL":"OK");
    ESP.restart();
  },[](){
    HTTPUpload& upload = server.upload();
    if(upload.status == UPLOAD_FILE_START){
      Serial.setDebugOutput(true);
      WiFiUDP::stopAll();
      Serial.printf("Update: %s\n", upload.filename.c_str());
      uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
      if(!Update.begin(maxSketchSpace)){//start with max available size
        Update.printError(Serial);
      }
    } else if(upload.status == UPLOAD_FILE_WRITE){
      if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
        Update.printError(Serial);
      }
    } else if(upload.status == UPLOAD_FILE_END){
      if(Update.end(true)){ //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
      Serial.setDebugOutput(false);
    }
    yield();
  });

  server.begin();
  MDNS.addService("http", "tcp", 80);
  Serial.println("Server started");

  udp.begin(localPort);
  WiFi.hostByName(ntpServerName, timeServerIP); 

  int cb;
  do {
    sendNTPpacket(timeServerIP);
    
    for (int i=0; i<5 && !cb; i++)
    {
      delay(50);
      cb = udp.parsePacket();
    }
  } while (!cb);
  
  millisStart = millis();
  udp.read(packetBuffer, NTP_PACKET_SIZE);
  
  unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
  unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
  unsigned long secsSince1900 = highWord << 16 | lowWord;
  const unsigned long seventyYears = 2208988800UL;
  epoch = secsSince1900 - seventyYears;

  Serial.print("The UTC time is ");
  Serial.println(getTime(millis()));
}

void loop() {
  server.handleClient();
  checkMaxOn();
  delay(1);
}

void show_index()
{
  if (temp_active) {
    getSensorData();
  }
  Temp = html1 + (temp_active ? String(temp_c) : "-") + html2 + String((val) ? "/on" : "/off");
  Temp += html3 + String((val) ? "BACKGROUND-COLOR: DarkGray;" : "BACKGROUND-COLOR: Chartreuse;") + html4 + getTime(millis()) + html5;
  server.send(200, "text/html", Temp);
}

void handle_on()
{
  waterStart = millis();
  val = 0;
  digitalWrite(WATER_PIN, val);
  server.sendHeader("Location","/");
  server.send(303);
}
 
void handle_off()
{
  val = 1;
  digitalWrite(WATER_PIN, val);
  server.sendHeader("Location","/");
  server.send(303);
}

void return_data()
{
  Temp  = "{\"id\":1,\"type\":\"casa-calida-sprinkler\",\"name\":\"Sprinkler Control\",\"active\":" + String((val + 1) % 2);
  Temp += ",\"lastRun\":\"" + (waterStart ? getTime(waterStart) : "") + "\"" + (temp_active ? ", \"temperature\": " + String(temp_c) : "") + "}";
  server.send(200, "application/json", Temp);
}

unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  memset(packetBuffer, 0, NTP_PACKET_SIZE);

  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  udp.beginPacket(address, 123);
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

String getTime(unsigned long timeToFormat)
{
  unsigned long secondsDiff = (timeToFormat - millisStart) / 1000;
  unsigned long currentTime = epoch + secondsDiff;
  String hour = String((currentTime  % 86400L) / 3600);
  String minutes = "";
  if ( ((currentTime % 3600) / 60) < 10 ) {
    minutes += '0';
  }
  minutes += (currentTime % 3600) / 60;
  String seconds = "";
  if ( (currentTime % 60) < 10 ) {
    seconds += '0';
  }
  seconds += currentTime % 60;
  return hour + ':' + minutes + ':' + seconds;
}

void checkMaxOn()
{
  if (val == 0 && millis() - waterStart > MAX_WATER_TIME) {
    val = 1;
    digitalWrite(WATER_PIN, val);
    Serial.println("Auto water off.");
  }
}

void getSensorData() {
  sensors.requestTemperatures();
  float temp = sensors.getTempC(outsideThermometer);
    
  if (isnan(temp)) {
    Serial.println("Failed to read from DS sensor!");
  } else {
    temp_c = temp;
  }
}

