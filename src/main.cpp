#include <Arduino.h>

#include <SPI.h>
#include <DMD2.h>
#include <DMDESP.h>
#include <fonts/SystemFont5x7.h>
#include <fonts/Arial_Black_16.h>
#include "DHT.h"
#include <Adafruit_Sensor.h>
#include <ESP8266WiFi.h> //https://github.com/esp8266/Arduino

#include "ThingSpeak.h"

// needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager


unsigned long myChannelNumber = 1;
const char * myWriteAPIKey = "K9YK5F2GQ5QME81I";
WiFiClient client;

#define mq2pin A0
#define DHTPIN D1
// #define windSpeedPin A0
#define DHTTYPE DHT22
int windSpeedPin = 4;

DHT dht(DHTPIN, DHTTYPE);

int mq2Value = 0;
int kelembaban = 0;
int suhu = 0;
int indeksPanas = 0;
int windSpeed = 0;

// windSpeed
const bool debugOutput = true;
const float number_reed = 4;

unsigned long  next_timestamp = 0;
volatile unsigned long i = 0;
float wind = 0;
float last_wind = 0;
int count = 0;
volatile unsigned long last_micros;
long debouncing_time = 5; //in millis
char charBuffer[32];

void ICACHE_RAM_ATTR Interrupt()
{
  if((long)(micros() - last_micros) >= debouncing_time * 1000) {
    i++;
    last_micros = micros();
  }
}

const uint8_t *FONT = Arial_Black_16;
//SETUP DMD
#define DISPLAYS_WIDE 4 // Kolom Panel
#define DISPLAYS_HIGH 1 // Baris Panel
DMDESP Disp(DISPLAYS_WIDE, DISPLAYS_HIGH);

class Task
{
public:
    Task(unsigned long interval, void (*callback)())
    {
        this->interval = interval;
        this->callback = callback;
        this->nextRun = millis() + interval;
    }

    void update()
    {
        if (millis() >= nextRun)
        {
            nextRun = millis() + interval;
            callback();
        }
    }

private:
    unsigned long interval;
    unsigned long nextRun;
    void (*callback)();
};


void TeksJalan(int y, uint8_t kecepatan) {
    String pesan = "Selamat datang di POLITEKNIK PIKSI GANESHA || Weather Station || Suhu : " + String(suhu) +(char)223+ "°C - Kelembaban : " + String(kelembaban) + " % - Kecepatan Angin : " + String(windSpeed) + " km/h - Kadar CO2 : " + mq2Value + " ppm";
    static uint32_t pM;
    static uint32_t x;
    int width = Disp.width();
    Disp.setFont(FONT);
    int fullScroll = Disp.textWidth(pesan.c_str()) + width;
    if((millis() - pM) > kecepatan) { 
        pM = millis();
        if (x < fullScroll) {
        ++x;
        } else {
        x = 0;
        return;
        }
        Disp.drawText(width - x, y, pesan.c_str());
    }  
}
// = = = = = = = = === = = =  =

Task dataUpdate(5000, [](){

    mq2Value = analogRead(mq2pin);
    kelembaban = dht.readHumidity();
    suhu = dht.readTemperature();
    indeksPanas = dht.computeHeatIndex(suhu, kelembaban, false);


    if (isnan(kelembaban) || isnan(suhu)) {
        Serial.println(F("Failed to read from DHT sensor!"));
        return;
    }
    if (millis() > next_timestamp )    
    { 
        detachInterrupt(windSpeedPin);
        count++; 
        float rps = i/number_reed; //computing rounds per second 
        if(i == 0)
        wind = 0.0;
        else
        wind = 1.761 / (1 + rps) + 3.013 * rps;// found here: https://www.amazon.de/gp/customer-reviews/R3C68WVOLJ7ZTO/ref=cm_cr_getr_d_rvw_ttl?ie=UTF8&ASIN=B0018LBFG8 (in German)
        if(last_wind - wind > 0.8 || last_wind - wind < -0.8 || count >= 10){
        if(debugOutput){
            Serial.print("Wind: ");
            Serial.print(wind);
            Serial.println(" km/h");
            windSpeed = wind;
        }
        String strBuffer;
        strBuffer =  String(wind);
        strBuffer.toCharArray(charBuffer,10);
        count = 0;
        }
        i = 0;
        last_wind = wind;
        next_timestamp  = millis()+1000; //intervall is 1s
        attachInterrupt(windSpeedPin,Interrupt,RISING);
    }
    yield();
    

    Serial.println("Data Updated");
    Serial.println("mq2Value    : " + String(mq2Value)+ " ppm");
    Serial.println("suhu        : " + String(suhu)+" C");
    Serial.println("kelembaban  : " + String(kelembaban)+" %");
    Serial.println("indeksPanas : " + String(indeksPanas)+" C");
    Serial.println("windSpeed   : " + String(windSpeed)+" km/s");
    Serial.println();
});

Task thingspeakUpdate(900000, [](){ 
    ThingSpeak.setField(1, suhu);
    ThingSpeak.setField(2, kelembaban);
    ThingSpeak.setField(3, mq2Value);
    ThingSpeak.setField(4, windSpeed);

    // set the status
    ThingSpeak.setStatus("myStatus OK");

    // write to the ThingSpeak channel
    ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if(x == 200){
        Serial.println("Channel update successful.");
    }
    else{
        Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
});

void setup()
{
    Serial.begin(115200);
    pinMode(mq2pin, INPUT);
    pinMode(DHTPIN, INPUT);
    pinMode(windSpeedPin, INPUT_PULLUP);
    

    Disp.start(); // Jalankan library DMDESP
    Disp.setBrightness(20); // Tingkat kecerahan
    Disp.setFont(FONT);
   
    WiFiManager wifiManager;
    wifiManager.autoConnect("Ganesha Purple V2");
    Serial.println("connected...yeey :)");
    ThingSpeak.begin(client);

    delay(3000);
    dht.begin();
    attachInterrupt(windSpeedPin,Interrupt,RISING);
}

void loop()
{
    dataUpdate.update();
    thingspeakUpdate.update();
    Disp.loop();
    TeksJalan(0, 50);
}