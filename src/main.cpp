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

#define mq2pin D2
#define DHTPIN D1
#define windSpeedPin A0
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

int mq2Value = 0;
int kelembaban = 0;
int suhu = 0;
int indeksPanas = 0;
int windSpeed = 0;
int windSpeedMph = 0;
int altitude = 702;
int pressure = 0;
String gasStatus = "";

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

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


void TeksJalan(int y, uint8_t kecepatan) {
    String pesan = "Selamat datang di PIKSI GANESHA Bandung || Mini Weather Station Ganesha Purple || Suhu : " + String(suhu) + " C - Kelembaban : " + String(kelembaban) + " % - Kecepatan Angin : " + String(windSpeed) + " m/h - Gas : " + gasStatus ;
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
        gasStatus = "Terdeteksi";
    }
    else
    {
        gasStatus = "Tidak Terdeteksi";
    }

    Serial.println("Data Updated");
    Serial.println("mq2Value    : " + String(mq2Value)+" ppm");
    Serial.println("gasStatus   : " + gasStatus);
    Serial.println("suhu        : " + String(suhu)+" C");
    Serial.println("kelembaban  : " + String(kelembaban)+" %");
    Serial.println("indeksPanas : " + String(indeksPanas)+" C");
    Serial.println("windSpeed   : " + String(windSpeed)+" m/s");
    Serial.println("windSpeedMph: " + String(windSpeedMph)+" mph");
    Serial.println();
});

Task thingspeakUpdate(1800000, [](){ 
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
    pinMode(windSpeedPin, INPUT);

    Disp.start(); // Jalankan library DMDESP
    Disp.setBrightness(20); // Tingkat kecerahan
    Disp.setFont(FONT);
   
    WiFiManager wifiManager;
    wifiManager.autoConnect("Ganesha Purple V2");
    Serial.println("connected...yeey :)");
    ThingSpeak.begin(client);

    delay(3000);
    dht.begin();

     // Tentukan huruf
}

void loop()
{
    dataUpdate.update();
    thingspeakUpdate.update();
    Disp.loop();
    TeksJalan(0, 50);
}