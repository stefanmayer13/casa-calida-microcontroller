#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <SoftwareSerial.h>
#include <WifiAuth.h>

#define ATTINYPIN  0 //GPIO0
const int Rx = 2;
const int Tx = 1;
SoftwareSerial mySerial(Rx, Tx);

const char* host = "192.168.1.35";
const int httpPort = 8001;
String id = "4";

ESP8266WebServer server(80);

float humidity, temp_c;
String webString="";

boolean updateSent = false;
boolean updateInProgress = false;
boolean waitingForShutdown = false;

String updateIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";

void setup() {
  digitalWrite(ATTINYPIN, LOW);
  pinMode(ATTINYPIN, OUTPUT);
  
  // put your setup code here, to run once:
  Serial.begin(115200);
  mySerial.begin(9600);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WifiAuth::ssid, WifiAuth::password);
  Serial.print("\n\r \n\rWorking to connect");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("DHT Temperature Reading Sensors");
  Serial.print("Connected to ");
  Serial.println(WifiAuth::ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/update", HTTP_GET, [](){
    updateInProgress = true;
    digitalWrite(ATTINYPIN, LOW);
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", updateIndex);
  });
  server.on("/update", HTTP_POST, [](){
    updateInProgress = true;
    digitalWrite(ATTINYPIN, LOW);
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
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  while(mySerial.available() > 0)
  {
    String line = mySerial.readStringUntil('\r');
    Serial.print(line);
  }
  if(!waitingForShutdown) {
        
    Serial.print("connecting to ");
    Serial.println(host);
        
    WiFiClient client;
    if (!client.connect(host, httpPort)) {
      Serial.println("connection failed");
      return;
    }
    /*
    String url = "/?id=";
    url += id;
    url += "&type=casa-calida-temperature-battery";
    url += "&name=Temperature%20and%20Humidity%20Sensor%20(Battery)";
    url += "&temperature=";
    url += temp_c;
    url += "&humidity=";
    url += humidity;
    
    Serial.print("Requesting URL: ");
    Serial.println(url);
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" + 
                "Connection: close\r\n\r\n");
    updateSent = true;
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        updateSent = false;
        break;
      }
    }
    while(client.available()){
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
    */
    Serial.println("Ok to turn off");
    digitalWrite(ATTINYPIN, HIGH);
    waitingForShutdown = true;
  }
}
