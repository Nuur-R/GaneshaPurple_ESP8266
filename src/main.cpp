#include <Arduino.h>

#include <SPI.h>
#include <DMD2.h>
#include <fonts/SystemFont5x7.h>
#include <fonts/Arial_Black_16.h>
#include "DHT.h"
#include <Adafruit_Sensor.h>
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

#include "ThingSpeak.h"

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager


#define mq2pin D2
#define DHTPIN D1
#define windSpeedPin A0
#define DHTTYPE DHT22

unsigned long myChannelNumber = 3;
const char * myWriteAPIKey = "CBHFE8I3YQ0S6A0G";
WiFiClient client;

DHT dht(DHTPIN, DHTTYPE);

int mq2Value = 0;
int kelembaban = 0;
int suhu = 0;
int indeksPanas = 0;
int windSpeed = 0;
int windSpeedMph = 0;
int altitude = 702;
int pressure = 0;
String gasStatus = "No Gas Detected";
String MESSAGE;

const int WIDTH = 4;
const uint8_t *FONT = Arial_Black_16;

SPIDMD  dmd(WIDTH,1);
DMD_TextBox box(dmd);
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

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// = = = = = = = = === = = =  =

Task dataUpdate(1000, [](){
    float sensorValue = analogRead(A0);
    float voltage = (sensorValue / 1023) * 5;

    windSpeed = mapfloat(voltage, 0.4, 2, 0, 32.4);
    windSpeedMph = ((windSpeed * 3600) / 1609.344);

    if (windSpeed <= 0)
    {
        windSpeed = windSpeed * -1;
    }
    if (windSpeedMph <= 0)
    {
        windSpeedMph = windSpeedMph * -1;
    }

    mq2Value = digitalRead(mq2pin);
    kelembaban = dht.readHumidity();
    suhu = dht.readTemperature();
    indeksPanas = dht.computeHeatIndex(suhu, kelembaban, false);
    pressure = 0;

    if (isnan(kelembaban) || isnan(suhu)) {
        Serial.println(F("Failed to read from DHT sensor!"));
        return;
    }
    if (mq2Value > 0)
    {
        gasStatus = "Udara Kotor";
    }
    else
    {
        gasStatus = "Udara Bersih";
    }

    MESSAGE = "Selamat Datang di PIKSI GANESHA Bandung || Mini Weather Station Ganesha Purple || Kualitas Udaraa : " + gasStatus + " || Temperatur : " + String(suhu) + " C || Kelembaban : " + String(kelembaban) + " % || Indeks Panas : " + String(indeksPanas) + " C || Kecepatan Angin : " + String(windSpeed) + " m/s      ";
    Serial.println(MESSAGE);
});
Task thingspeakUpdate(30000, [](){
        
        ThingSpeak.setField(1, windSpeedMph);
        ThingSpeak.setField(2, mq2Value);
        ThingSpeak.setField(3, pressure);
        ThingSpeak.setField(4, suhu);
        ThingSpeak.setField(5, kelembaban);
        ThingSpeak.setField(6, altitude);

        // set the status
        ThingSpeak.setStatus("myStatus OK");
        
        // write to the ThingSpeak channel
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
    
    WiFiManager wifiManager;
    wifiManager.autoConnect("Ganesha Purple V2");
    Serial.println("connected...yeey :)");
    ThingSpeak.begin(client);

    dht.begin();

    dmd.setBrightness(255);
    dmd.selectFont(FONT);
    dmd.begin();

    pinMode(mq2pin, INPUT);
    pinMode(DHTPIN, INPUT);
    pinMode(windSpeedPin, INPUT);
    
}

void loop()
{
    dataUpdate.update();
    thingspeakUpdate.update();
    const char *next = MESSAGE.c_str();
    while(*next) {
        Serial.print(*next);
        box.print(*next);
        delay(200);
        next++;
    }
}