#include <avr/sleep.h>    // Sleep Modes
#include <avr/power.h>    // Power management
#include <avr/wdt.h>      // Watchdog timer

#include <dht.h>
#include <SoftwareSerial.h>

#define DHT22PIN 0
#define ADPIN 3
#define POWERPIN 4
#define ESPPIN  2
const int Rx = 3;
const int Tx = 1;
SoftwareSerial mySerial(Rx, Tx);

float h = 0;
float t = 0;
float h_prev = 0;
float t_prev = 0;
float battery = 0;
boolean active = false;
unsigned long startMillis = 0;
unsigned long maxOn = 20000;

dht DHT;

ISR (WDT_vect) 
{
  //mySerial.println("INTERRUPT!");
  //wdt_disable();  // disable watchdog
}

void resetWatchdog ()
{
  // clear various "reset" flags
  MCUSR = 0;     
  // allow changes, disable reset, clear existing interrupt
  WDTCR = bit (WDCE) | bit (WDE) | bit (WDIF);
  // set interrupt mode and an interval (WDE must be changed from 1 to 0 here)
  WDTCR = bit (WDIE) | bit (WDP3) | bit (WDP0);    // set WDIE, and 8 seconds delay
  // pat the dog
  wdt_reset();  
}

void setup() {
  resetWatchdog ();
  
  pinMode(Rx, INPUT);
  pinMode(Tx, OUTPUT);
  pinMode(ESPPIN, INPUT);
  pinMode(POWERPIN, OUTPUT);
  mySerial.begin( 9600 );
  //mySerial.println("Starting");
}

void loop() {
  if(!active) {
    int chk = DHT.read22(DHT22PIN);
    switch (chk)
    {
      case DHTLIB_OK:
          //Everything fine
          break;
      case DHTLIB_ERROR_CHECKSUM:
          mySerial.print("Checksum error,\r");
          break;
      case DHTLIB_ERROR_TIMEOUT:
          mySerial.print("Time out error,\r");
          break;
      case DHTLIB_ERROR_CONNECT:
          mySerial.print("Connect error,\r");
          break;
      case DHTLIB_ERROR_ACK_L:
          mySerial.print("Ack Low error,\r");
          break;
      case DHTLIB_ERROR_ACK_H:
          mySerial.print("Ack High error,\r");
          break;
      default:
          mySerial.print("Unknown error,\r");
          break;
    }
    if(chk == DHTLIB_OK) {
      h = DHT.humidity;
      t = DHT.temperature;
      mySerial.print("Humidity:");
      mySerial.println(h);
      mySerial.print("Temperature:");
      mySerial.println(t);
      battery = 3.3/1023.0*float(analogRead(ADPIN));
      mySerial.print("Battery:");
      mySerial.println(battery);
      if(abs(h-h_prev) >= 0.5 || abs(t-t_prev) >= 0.5) {
        //mySerial.println("Turning Wifi On");
        turnOn();
        unsigned long currentMillis = millis();
        startMillis = currentMillis;
        delay(500);
      } else {
        //mySerial.println("Going to sleep");
        goToSleep ();
      }
    } else {
      //mySerial.println("Going to sleep");
      goToSleep ();
    }
  } else {
    unsigned long currentMillis = millis();
    if(currentMillis - startMillis >= maxOn) {
      digitalWrite(POWERPIN, HIGH);
      //mySerial.println("Turning Wifi off beacuse of max time");
      turnOff();
    } else {
      boolean val = digitalRead(ESPPIN);
      if (val == HIGH) {
        delay(3000);
        val = digitalRead(ESPPIN);
        if (val == HIGH) {
          h_prev = h;
          t_prev = t;
          //mySerial.println("Turning Wifi off as requested");
          turnOff();
        }
      }
    }
    delay(500);
  }
}

void goToSleep ()
{
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);
  ADCSRA = 0;            // turn off ADC
  power_all_disable ();  // power off ADC, Timer 0 and 1, serial interface
  noInterrupts ();       // timed sequence coming up
  resetWatchdog ();      // get watchdog ready
  sleep_enable ();       // ready to sleep
  interrupts ();         // interrupts are required now
  sleep_cpu ();          // sleep                
  sleep_disable ();      // precaution
  power_all_enable ();   // power everything back on
}


void turnOn() {
  digitalWrite(POWERPIN, HIGH);
  active = true;
}

void turnOff() {
  digitalWrite(POWERPIN, LOW);
  active = false;
}
