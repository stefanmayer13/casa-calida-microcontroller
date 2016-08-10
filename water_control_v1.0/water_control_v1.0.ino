#include <avr/pgmspace.h>
#include <EEPROM.h>

// CONFIG BEGIN
//#define DEBUG
#define PORT 80

unsigned long MAX_WATER_TIME = 1200000UL; //(20 * 60 * 1000); 20min
unsigned long WIFI_RETRY_PERIOD = 15000UL; //(15 * 1000); 15sec

#define LED_PIN 13
#define WATER_PIN 8
// CONFIG END

#define WIFI_Serial Serial
#ifdef DEBUG
  #define Debug_Serial mySerial
  #include <SoftwareSerial.h>
  SoftwareSerial mySerial(10,11); // RX, TX
#endif

// Webserver
#define HTML_SENDE_MODE_PREPARE 0
#define HTML_SENDE_MODE_SEND 11

int HTMLContentLength;
int HTMLHeaderLength;
int HTMLTempLength;
byte HTMLSendeMode;

#define RECV_BUFFER_SIZE 100
char RecvBuffer[RECV_BUFFER_SIZE];

// WIFI
#define WIFI_ERROR_NONE 0
#define WIFI_ERROR 1

char Network[30];
char IP_Adress[20];
char WIFI_Host[24];
int WIFI_CWMODE;
#define CWMODE_STATION 1
#define CWMODE_ACCESSPOINT 2
int WIFISetupError = 1;

// Water
#define WATER_OFF HIGH
#define WATER_ON LOW
int waterMode = WATER_OFF;
unsigned long waterStartTime = 0;
unsigned long lastWifiRetry = 0;

// Clock
int year = 2016, month = 1, day = 1, hour = 0, minute = 0;
int lastYear = 0, lastMonth = 0, lastDay = 0, lastHour = 0, lastMinute = 0;
unsigned long milliseconds = 0UL;

// Schedule
int scheduleHour, scheduleMinute;
bool scheduleActive = false;
bool scheduleSkip = false;

// Functions
int setupWIFI();
void recvBufferClear();
void waterHandling();
bool clockHandling();
void setWater(int value);
void setLed(int value);
void handleRequest();
void webReply(int WIFI_Cannel);
void HTMLPageHeader();
void showHTMLPage();
void JSONHeader();
void returnJSONData();
void HTMLSend(char * c);
void HTMLSendInt(int v);
void HTMLSendPROGMEM(const __FlashStringHelper* data);

void setup() {
  setLed(LOW);
  pinMode(LED_PIN,OUTPUT);
  setWater(WATER_OFF);
  pinMode(WATER_PIN,OUTPUT);
  WIFI_Serial.begin(115200);
  WIFI_Serial.setTimeout(5000);
  #ifdef DEBUG
    Debug_Serial.begin(9600);
  #endif
  delay(3000); // Waiting for ESP8266
  setLed(HIGH);
}

void loop() {
  bool timeChanged = clockHandling();
  if (timeChanged) {
    waterHandling();
  }
  
  // WIFI setup
  if (WIFISetupError && millis() - lastWifiRetry > WIFI_RETRY_PERIOD) {
    WIFISetupError = setupWIFI();
    if (WIFISetupError) {
      #ifdef DEBUG
        Debug_Serial.println(F("WIFI Setup Error"));
        Debug_Serial .println(F("AT+RST"));
      #endif
      WIFI_Serial.println(F("AT+RST"));
      setLed(HIGH);
      WIFI_Serial.setTimeout(20);
    } else {
       setLed(LOW);
       WIFI_Serial.setTimeout(20);
    }
    lastWifiRetry = millis();
  }

  if (WIFI_Serial.findUntil("+IPD,", "\r")) {
    handleRequest();
  }
}

void handleRequest() {
  int WIFI_Cannel, WIFI_Packet_Length;
  char *buffer_pointer;
  byte len;
  bool json;
  
  WIFI_Cannel = WIFI_Serial.parseInt();
  WIFI_Serial.findUntil(",", "\r");
  WIFI_Packet_Length = WIFI_Serial.parseInt();

  if (WIFI_Serial.findUntil("GET /", "\r")) {

    WIFI_Serial.readBytesUntil(13, RecvBuffer, RECV_BUFFER_SIZE);

    if (WIFI_Packet_Length > 0) {
      buffer_pointer = RecvBuffer;

      if (strncmp(buffer_pointer, "api/", 4) == 0) {
        buffer_pointer += 4;
        json = true;
      } else {
        json = false;
      }
  
      if (strncmp(buffer_pointer, "?WATER=", 7) == 0) {
        buffer_pointer += 7;
        delay(50);
        if (strncmp(buffer_pointer, "1", 1) == 0) {
          setWater(WATER_ON);
        } else if (strncmp(buffer_pointer, "0", 1) == 0) {
          setWater(WATER_OFF);
        }
        delay(50);
        buffer_pointer += 1;
      } else if (strncmp(buffer_pointer, "daily?time=", 11) == 0) {
        buffer_pointer += 11;
        String  hourStr = "";
        String  minuteStr = "";
        hourStr += buffer_pointer[0];
        hourStr += buffer_pointer[1];
        buffer_pointer+=5;
        minuteStr += buffer_pointer[0];
        minuteStr += buffer_pointer[1];
        buffer_pointer+=2;
        if (scheduleHour != hourStr.toInt() && scheduleMinute !== minuteStr.toInt()) {
          scheduleHour = hourStr.toInt();
          scheduleMinute = minuteStr.toInt();
          EEPROM.write(0, scheduleHour);
          EEPROM.write(1, scheduleMinute);
        }
        scheduleActive = true;
      } else if (strncmp(buffer_pointer, "nodaily", 7) == 0) {
        buffer_pointer += 7;
        scheduleActive = false;
      } else if (strncmp(buffer_pointer, "?skipdaily=", 11) == 0) {
        buffer_pointer += 11;
        if (strncmp(buffer_pointer, "1", 1) == 0) {
          scheduleSkip = true;
        } else if (strncmp(buffer_pointer, "0", 1) == 0) {
          scheduleSkip = false;
        }
        buffer_pointer += 1;
      } else if (strncmp(buffer_pointer, "time?date=", 10) == 0) {
        buffer_pointer += 10;
        String yearStr = "";
        String  monthStr = "";
        String  dayStr = "";
        String  hourStr = "";
        String  minuteStr = "";
        for (int i=0; i < 4; i++) {
          yearStr += buffer_pointer[i];
        }
        buffer_pointer+=5;
        monthStr += buffer_pointer[0];
        monthStr += buffer_pointer[1];
        buffer_pointer+=3;
        dayStr += buffer_pointer[0];
        dayStr += buffer_pointer[1];
        buffer_pointer+=8;
        hourStr += buffer_pointer[0];
        hourStr += buffer_pointer[1];
        buffer_pointer+=5;
        minuteStr += buffer_pointer[0];
        minuteStr += buffer_pointer[1];
        buffer_pointer+=2;
        if (year == 0) {
          scheduleHour = EEPROM.read(0);
          scheduleMinute = EEPROM.read(1);
        }
        year = yearStr.toInt();
        month = monthStr.toInt();
        day = dayStr.toInt();
        hour = hourStr.toInt();
        minute = minuteStr.toInt();
        milliseconds = millis();
#ifdef WIFI_DEBUG
        Debug_Serial.print(F("Time set to"));
        Debug_Serial.println(String(year)+"-"+month+"-"+day+" "+hour+":"+minute);
#endif
      }

      WIFI_Host[0] = 0;
      if (WIFI_Serial.find("Host: ")) {
        len = WIFI_Serial.readBytesUntil(13, WIFI_Host, 23);
        WIFI_Host[len] = 0;
      }
      webReply(WIFI_Cannel, json);
    }
  }
  recvBufferClear();
}

//---------------------------------------------------------------------------
int setupWIFI() {
  byte len;

  recvBufferClear();
  
#ifdef DEBUG
  Debug_Serial .println(F("AT+CIPMUX=1"));
#endif
  WIFI_Serial.println(F("AT+CIPMUX=1"));
  delay(10);
  if(!WIFI_Serial.find("OK")){
    return WIFI_ERROR;
  }

  recvBufferClear();

#ifdef DEBUG
  Debug_Serial .println(F("AT+CIPSERVER=1"));
#endif
  WIFI_Serial.print(F("AT+CIPSERVER=1,"));
  WIFI_Serial.println(PORT);
  delay(10);
  
  if (!WIFI_Serial.find("OK")) {
    return WIFI_ERROR;
  }

  recvBufferClear();

#ifdef DEBUG
  Debug_Serial .println(F("AT+CIPSTO=0"));
#endif
  WIFI_Serial.println(F("AT+CIPSTO=0"));
  delay(10);
  if (!WIFI_Serial.find("OK")) {
    return WIFI_ERROR;
  }
  
  recvBufferClear();
  Network[0] = 0;

#ifdef DEBUG
  Debug_Serial .println(F("AT+CWMODE?"));
#endif
  WIFI_Serial.println(F("AT+CWMODE?"));
  delay(10);
  WIFI_CWMODE = 0;
  if (WIFI_Serial.find("AT+CWMODE?\r\r\n+CWMODE:")) {
    WIFI_CWMODE = WIFI_Serial.parseInt();
  }
  if (WIFI_CWMODE == 0) {
    return WIFI_ERROR;
  }

  if (WIFI_CWMODE == CWMODE_STATION) {
#ifdef DEBUG
    Debug_Serial .println(F("AT+CWJAP?"));
#endif
    WIFI_Serial.println(F("AT+CWJAP?"));
    delay(10);
    len = 0;
    Network[0] = 0;
    if (WIFI_Serial.find("AT+CWJAP?\r\r\n+CWJAP:\"")) {
      len = WIFI_Serial.readBytesUntil('\"', Network, 20);
      Network[len] = 0;
    }
    if (len > 0) {
#ifdef DEBUG
      Debug_Serial.println(F("Network:"));
      Debug_Serial.println(Network);
#endif
    }
    else {
      return WIFI_ERROR;
    }
  }
  if (WIFI_CWMODE == CWMODE_ACCESSPOINT) {
#ifdef DEBUG
    Debug_Serial .println(F("AT+CWSAP?"));
#endif
    WIFI_Serial.println(F("AT+CWSAP?"));
    delay(10);
    len = 0;
    Network[0] = 0;
   
    if (WIFI_Serial.find("AT+CWSAP?\r\r\n+CWSAP:\"")) {
      len = WIFI_Serial.readBytesUntil('\"', Network, 20);
      Network[len] = 0;
    }

    if (len > 0) {
#ifdef DEBUG
      Debug_Serial.println(F("Network:"));
      Debug_Serial.println(Network);
#endif
    }
    else {
      return WIFI_ERROR;
    }
  }

#ifdef WIFI_DEBUG
  Debug_Serial .println(F("AT+CIFSR"));
#endif
  WIFI_Serial.println(F("AT+CIFSR"));
  delay(1000);
  len = 0;
  IP_Adress[0] = 0;

  if (WIFI_Serial.find("AT+CIFSR\r\r\n") && WIFI_Serial.find("+CIFSR:STAIP,")) {
    len = WIFI_Serial.readBytesUntil('\r', IP_Adress, 20);
    IP_Adress[len] = 0;
  }

#ifdef WIFI_DEBUG
  if (len > 0) {
    Debug_Serial.println(F("IP"));
    Debug_Serial.println(IP_Adress);
  }
#endif
  if (len == 0) {
    return WIFI_ERROR;
  }

#ifdef WIFI_DEBUG
  Debug_Serial .println(F("Setup finished"));
#endif
  
  return WIFI_ERROR_NONE;
}

void waterHandling() {
  if (scheduleActive && scheduleHour == hour && scheduleMinute == minute) {
    if (!scheduleSkip) {
      setWater(WATER_ON);
    } else {
      scheduleSkip = false;
    }
  }
  if (waterMode == WATER_ON && (millis() - waterStartTime > MAX_WATER_TIME)) {
#ifdef DEBUG
    Debug_Serial.println(F("Water running too long, closing valve"));
#endif
    setWater(WATER_OFF);
  }
}

bool clockHandling(){
  if (millis() - milliseconds >= 60000UL) {
    milliseconds += 60000UL;

    minute++;
    if (minute >= 60) {
      minute = 0;
      hour++;
    }
    if (hour >= 24) {
      hour = 0;
      day++;
    }
    if ((day == 28 && month == 2 && year % 4 != 0) || (day == 29 && month == 2) || 
        (day == 30 && (month == 4 || month == 6 || month == 9 || month == 11)) || day >= 31) {
      month++;
      day = 1;
    }
    if (month >= 13) {
      year++;
      month = 1;
    }
    return true;
  }
  return false;
}

void setWater(int value) {
  if (value != waterMode && value == WATER_ON) {
    waterStartTime = millis();
  }
  waterMode = value;
  
  #ifdef DEBUG
    Debug_Serial.print(F("Setting water to "));
    Debug_Serial.println(waterMode);
  #endif

  if (waterMode == WATER_ON) {
    lastYear = year;
    lastMonth = month;
    lastDay = day;
    lastHour = hour;
    lastMinute = minute;
  }
  digitalWrite(WATER_PIN, waterMode);
}

void setLed(int value) {
  digitalWrite(LED_PIN, value);
}

void webReply(int WIFICannel, bool json) {
  HTMLSendeMode = HTML_SENDE_MODE_PREPARE;

  HTMLTempLength = 0;
  if (!json) {
    showHTMLPage();
  } else {
    returnJSONData();
  }
  HTMLContentLength = HTMLTempLength;

  HTMLTempLength = 0;
  if (!json) {
    HTMLPageHeader();
  } else {
    JSONHeader();
  }
  HTMLHeaderLength = HTMLTempLength;

  WIFI_Serial.print(F("AT+CIPSEND="));
  WIFI_Serial.print(WIFICannel);
  WIFI_Serial.print(F(","));
  WIFI_Serial.println(HTMLHeaderLength + HTMLContentLength);
  
  delay(20);

  recvBufferClear();

  HTMLSendeMode = HTML_SENDE_MODE_SEND;
  if (!json) {
    HTMLPageHeader();
    showHTMLPage();
  } else {
    JSONHeader();
    returnJSONData();
  }

  delay(10);
}

//---------------------------------------------------------------------------
void HTMLPageHeader() {
  HTMLSendPROGMEM(F("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n"));
  HTMLSendPROGMEM(F("Content-Length:"));
  HTMLSendInt(HTMLContentLength);
  HTMLSendPROGMEM(F("\r\n\r\n"));
}

//---------------------------------------------------------------------------

void showHTMLPage() {
  HTMLSendPROGMEM(F("<!DOCTYPE html><HTML><HEAD><title>Casa-Calida - Water Control</title><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></HEAD>"));
  HTMLSendPROGMEM(F("<BODY><h1>Welcome to Casa-Calida water control</h1>"));
  HTMLSendPROGMEM(F("<form action=\"/\" method=\"get\"><p>The sprinklers are currently <b>"));
  switch (waterMode)  {
    case WATER_ON:
      HTMLSendPROGMEM(F("ON"));
      break;
    case WATER_OFF:
      HTMLSendPROGMEM(F("OFF"));
      break;
  }

  if (lastYear > 0) {
    HTMLSendPROGMEM(F("</b>.</p><p>They were running the last time at "));
    String lastMonthFiller = lastMonth < 10 ? "0" : "";
    String lastDayFiller = lastDay < 10 ? "0" : "";
    String lastHourFiller = lastHour < 10 ? "0" : "";
    String lastMinuteFiller = lastMinute < 10 ? "0" : "";
    
    HTMLSendVar(String(lastYear)+"-"+lastMonthFiller+lastMonth+"-"+lastDayFiller+lastDay+" "+lastHourFiller+lastHour+":"+lastMinuteFiller+lastMinute);
  } else {
    HTMLSendPROGMEM(F("</b>"));
  }
  
  HTMLSendPROGMEM(F(".</p><button style=\"font-size: 1em\">Refresh</button></form><br/>"));
  HTMLSendPROGMEM(F("<button style=\"font-size: 2em\"><a style=\"text-decoration: none;color: black;\" href=\"/?WATER="));
  switch (waterMode)  {
    case WATER_ON:
      HTMLSendPROGMEM(F("0"));
      break;
    case WATER_OFF:
      HTMLSendPROGMEM(F("1"));
      break;
  }
  HTMLSendPROGMEM(F("\">Turn Water "));
  switch (waterMode)  {
    case WATER_ON:
      HTMLSendPROGMEM(F("OFF"));
      break;
    case WATER_OFF:
      HTMLSendPROGMEM(F("ON"));
      break;
  }
  HTMLSendPROGMEM(F("</a></button><br/><br/><br/>"));

  HTMLSendPROGMEM(F("<form action=\"/daily\" method=\"get\"><input type=\"time\" name=\"time\" value=\""));

  if(scheduleActive) {
    String scheduleHourFiller = scheduleHour < 10 ? "0" : "";
    String scheduleMinuteFiller = scheduleMinute < 10 ? "0" : "";
    HTMLSendVar(String(scheduleHourFiller)+scheduleHour+":"+scheduleMinuteFiller+scheduleMinute);
  }

  HTMLSendPROGMEM(F("\" /><button>Set Daily Sprinkler Time</button></form>"));

  if(scheduleSkip) {
    HTMLSendPROGMEM(F("<br/><br/><p>Skipping the next scheduled time! <button><a style=\"text-decoration: none;color: black;\" href=\"/?skipdaily=0\">Enable again</a></button></form>"));
  } else {
    HTMLSendPROGMEM(F("<br/><br/><button><a style=\"text-decoration: none;color: black;\" href=\"/?skipdaily=1\">Skip next</a></button></form>"));
  }

  if(scheduleActive) {
    HTMLSendPROGMEM(F("<br/><br/><form action=\"/nodaily\" method=\"get\"><button>Disable Daily Routine</button></form>"));
  }

  HTMLSendPROGMEM(F("<br/><br/><form action=\"/time\" method=\"get\"><input type=\"date\" name=\"date\" value=\""));

  String monthFiller = month < 10 ? "0" : "";
  String dayFiller = day < 10 ? "0" : "";
  
  HTMLSendVar(String(year)+"-"+monthFiller+month+"-"+dayFiller+day);
  
  HTMLSendPROGMEM(F("\" /><input type=\"time\" name=\"time\" value=\""));

  String hourFiller = hour < 10 ? "0" : "";
  String minuteFiller = minute < 10 ? "0" : "";
  HTMLSendVar(String(hourFiller)+hour+":"+minuteFiller+minute);

  HTMLSendPROGMEM(F("\" /><button>Set Time</button></form>"));
  HTMLSendPROGMEM(F("</body></html>"));
}

//---------------------------------------------------------------------------
void JSONHeader() {
  HTMLSendPROGMEM(F("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n"));
  HTMLSendPROGMEM(F("Content-Length:"));
  HTMLSendInt(HTMLContentLength);
  HTMLSendPROGMEM(F("\r\n\r\n"));
}

//---------------------------------------------------------------------------
void returnJSONData() {
  const __FlashStringHelper* one = F("1");
  const __FlashStringHelper* zero = F("0");
  HTMLSendPROGMEM(F("{\"id\":1,\"active\":"));
  switch (waterMode)  {
    case WATER_ON:
      HTMLSendPROGMEM(one);
      break;
    case WATER_OFF:
      HTMLSendPROGMEM(zero);
      break;
  }
  HTMLSendPROGMEM(F(",\"lastRun\":\""));
  String lastMonthFiller = lastMonth < 10 ? "0" : "";
  String lastDayFiller = lastDay < 10 ? "0" : "";
  String lastHourFiller = lastHour < 10 ? "0" : "";
  String lastMinuteFiller = lastMinute < 10 ? "0" : "";
  HTMLSendVar(String(lastYear)+"-"+lastMonthFiller+lastMonth+"-"+lastDayFiller+lastDay+" "+lastHourFiller+lastHour+":"+lastMinuteFiller+lastMinute);
  
  HTMLSendPROGMEM(F("\",\"dailyTime\":\""));
  String scheduleHourFiller = scheduleHour < 10 ? "0" : "";
  String scheduleMinuteFiller = scheduleMinute < 10 ? "0" : "";
  HTMLSendVar(String(scheduleHourFiller)+scheduleHour+":"+scheduleMinuteFiller+scheduleMinute);
    
  HTMLSendPROGMEM(F("\",\"scheduleActive\":"));
  HTMLSendPROGMEM(scheduleActive?one:zero);
  HTMLSendPROGMEM(F(",\"scheduleSkip\":"));
  HTMLSendPROGMEM(scheduleSkip?one:zero);

  HTMLSendPROGMEM(F(",\"time\":\""));
  String monthFiller = month < 10 ? "0" : "";
  String dayFiller = day < 10 ? "0" : "";
  String hourFiller = hour < 10 ? "0" : "";
  String minuteFiller = minute < 10 ? "0" : "";
  HTMLSendVar(String(year)+"-"+monthFiller+month+"-"+dayFiller+day+" "+hourFiller+hour+":"+minuteFiller+minute);

  HTMLSendPROGMEM(F("\",\"wifi_error_state\":"));
  HTMLSendInt(WIFISetupError);
  
  HTMLSendPROGMEM(F("}\n"));
}

//---------------------------------------------------------------------------
void HTMLSendInt(int value) {
  char tmpText[8];
  itoa(value, tmpText, 10);
  HTMLSend(tmpText);
}

//---------------------------------------------------------------------------
void HTMLSend(char * text) {
  HTMLTempLength += strlen(text);
  if (HTMLSendeMode == HTML_SENDE_MODE_SEND) {
    WIFI_Serial.print(text);
  }
}

//---------------------------------------------------------------------------
void HTMLSendPROGMEM(const __FlashStringHelper* p_text) {
  HTMLTempLength += strlen_P((const char*)p_text);
  if (HTMLSendeMode == HTML_SENDE_MODE_SEND) {
    WIFI_Serial.print(p_text);
  }
}

//---------------------------------------------------------------------------
void HTMLSendVar(String p_text) {
  HTMLTempLength += p_text.length();
  if (HTMLSendeMode == HTML_SENDE_MODE_SEND) {
    WIFI_Serial.print(p_text);
  }
}

//---------------------------------------------------------------------------
void recvBufferClear() {
  for (int i = 0; i < RECV_BUFFER_SIZE; i++) RecvBuffer[i] = 0;
  while (WIFI_Serial.available())WIFI_Serial.read();
}

