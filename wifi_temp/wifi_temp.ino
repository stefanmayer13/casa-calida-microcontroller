#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <DHT.h>

#define DHTTYPE DHT22
#define DHTPIN  2

const char* ssid     = "home4123";
const char* password = "kiketommy123";

ESP8266WebServer server(80);
DHT dht(DHTPIN, DHTTYPE, 11);

float humidity, temp_c;
String webString="";

unsigned long previousMillis = 0;
unsigned long interval = 2000UL;

String updateIndex = "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  dht.begin();

  WiFi.begin(ssid, password);
  Serial.print("\n\r \n\rWorking to connect");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("DHT Temperature Reading Server");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", [](){
    getdata();
    webString="<!DOCTYPE><html><head><title>CasaCalida - Temperature</title></head><body><p>Temperature: "+String((int)temp_c)+"&deg;C<br/>Humidity: "+String((int)humidity)+"%</body></html>";
    server.send(200, "text/html", webString);
  });

  server.on("/api/", [](){
    getdata();
    webString="{\"id\":2,\"type\":\"casa-calida-temperature\",\"name\":\"Temperature and Humidity Sensor\",\"temperature\":" + String((int)temp_c) + ",\"humidity\":" + String((int)humidity) + "}";
    server.send(200, "application/json", webString);
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
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

void getdata() {
  unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;   
 
    humidity = dht.readHumidity();
    temp_c = dht.readTemperature();

    if (isnan(humidity) || isnan(temp_c)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
  }
}

