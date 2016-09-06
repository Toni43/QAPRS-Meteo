/*
  Для передачи APRS пакетов использована библиотека Lukasz Nidecki SQ5RWU
  Наружная температура и влажность измеряются датчиком DHT22. Внутренняя 
  температура и давление - BMP180.
  На выводе 4 кнопка для принудительной передачи пакета.
  Pin 2 - вход данных DHT22
  Pin 13 - светодиод
  Pin 7 - PTT
  Pin 4 - кнопка для принудительной передачи пакета
*/

#include <Arduino.h>
#include <ArduinoQAPRS.h>
#include <DHT.h>
#include <Adafruit_BMP085.h>


#define TX_INTERVAL_METEO 600000  // интервал для передачи метео пакетов в миллисекуднах
#define TX_INTERVAL_STATUS 900000  // интервал для передачи статус пакетов в миллисекуднах
#define LED_BLINK_INTERVAL 1000 //период мигания светодиода в миллисекуднах
#define LEDPIN 13
#define BUTTONPIN 4 //кнопка для принудительной передачи пакета

DHT dht;
Adafruit_BMP085 bmp;

char * packet_buffer  = "                                                                                    \n ";
char from_addr[] = "RA4NHY";      // Позывной
char dest_addr[] = "APRSWX";     // Адрес
char relays[] = "WIDE2 2";       // Путь

//установка координат метеодатчика
const char Latitude[] = "5831.97N"; //координаты в формате DDMM.MM
const char Longitude[] = "04926.92E";  //координаты в формате DDDMM.MM

unsigned long previousMillis_txmeteo = 0;
unsigned long previousMillis_txstatus = 0;
unsigned long previousMillis_led = 0;
bool firsttx=true;
int ledState = LOW; 

int buttonState = 0;         // variable for reading the pushbutton status

void setup()
{
  pinMode(LEDPIN, OUTPUT);
  pinMode(BUTTONPIN, INPUT);
  
  digitalWrite(LEDPIN, HIGH);
  
  dht.setup(2); // data pin DHT22
  bmp.begin();
  
  Serial.begin(9600);            // Скорость для вывода отладочных сообщений

  QAPRS.init(6, 7);
  Serial.println("Ready");
  delay(3000);
  digitalWrite(LEDPIN, LOW);
}

void loop()
{

    buttonState = digitalRead(BUTTONPIN);
    
  if ( (millis() - previousMillis_txmeteo >= TX_INTERVAL_METEO) | (firsttx == true) | (buttonState == LOW))
    {
     firsttx=false;
     previousMillis_txmeteo=millis();
     
     digitalWrite(LEDPIN, LOW);
     
     delay(dht.getMinimumSamplingPeriod());
     float humidity = dht.getHumidity();
     float temperature = dht.toFahrenheit(dht.getTemperature());
     float internalTemp = bmp.readTemperature();

     unsigned long int pressure = (unsigned long int)bmp.readPressure();
     pressure/=10;

     snprintf(packet_buffer, 100, "=%s/%s_.../...g...t%03ir...p...P...h%02ib%05i Internal temperature=%i C", Latitude, Longitude, (int)temperature, (int)humidity, (int)pressure,  (int)internalTemp);


    Serial.println(packet_buffer); 
    
    QAPRS.send(from_addr, '2', dest_addr, '0', relays, packet_buffer);    // SSID-2
    }

    if (millis() - previousMillis_txstatus >= TX_INTERVAL_STATUS)
      {
        previousMillis_txstatus=millis();
        snprintf(packet_buffer, 100, "> QAPRS meteo http://R4N.SU/forum" );

        Serial.println(packet_buffer); 
    
        QAPRS.send(from_addr, '2', dest_addr, '0', relays, packet_buffer);    // SSID-2
      }

      
   if (millis() - previousMillis_led >= LED_BLINK_INTERVAL)
   {
    previousMillis_led=millis();

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) 
       {
       ledState = HIGH;
       } else
        {
        ledState = LOW;
        }

    // set the LED with the ledState of the variable:
    digitalWrite(LEDPIN, ledState);
    }
}


