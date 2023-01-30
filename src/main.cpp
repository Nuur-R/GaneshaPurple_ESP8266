#include <Arduino.h>

#include <SPI.h>
#include <DMD2.h>
#include <fonts/SystemFont5x7.h>
#include <fonts/Arial_Black_16.h>

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// Pin definitions
#define windPin 13 // Receive the data from sensor

// Constants definitions
const float pi = 3.14159265; // pi number
int period = 10000;          // Measurement period (miliseconds)
int delaytime = 10000;       // Time between samples (miliseconds)
int radio = 90;              // Distance from center windmill to outer cup (mm)
int jml_celah = 18;          // jumlah celah sensor

// Variable definitions
unsigned int Sample = 0;  // Sample number
unsigned int counter = 0; // B/W counter for sensor
unsigned int RPM = 0;     // Revolutions per minute
float speedwind = 0;      // Wind speed (m/s)


void addcount()
{
    counter++;
}
// Measure wind speed
void windvelocity()
{
    pinMode(windPin, INPUT_PULLUP);
    speedwind = 0;
    counter = 0;
    attachInterrupt(windPin, addcount, CHANGE);
    unsigned long millis();
    long startTime = millis();
    while (millis() < startTime + period)
    {
    }

    detachInterrupt(1);
}

void RPMcalc()
{
    RPM = ((counter / jml_celah) * 60) / (period / 1000); // Calculate revolutions per minute (RPM)
}

void WindSpeed()
{
    speedwind = ((2 * pi * radio * RPM) / 60) / 1000; // Calculate wind speed on m/s
}



void setup()
{
    // Set the pins
    pinMode(D4, INPUT);
    digitalWrite(2, HIGH);

    // sets the serial port to 9600
    Serial.begin(9600);

    // Splash screen
    Serial.println("ANEMOMETER");
    Serial.println("****");
    Serial.println("Based on depoinovasi anemometer sensor");
    Serial.print("Sampling period: ");
    Serial.print(period / 1000);
    Serial.print(" seconds every ");
    Serial.print(delaytime / 1000);
    Serial.println(" seconds.");
    Serial.println("* You could modify those values on code *");
    Serial.println();
}

void loop()
{
    Sample++;
    Serial.print(Sample);
    Serial.print(": Start measurement…");
    windvelocity();
    Serial.println(" finished.");
    Serial.print("Counter: ");
    Serial.print(counter);
    Serial.print("; RPM: ");
    RPMcalc();
    Serial.print(RPM);
    Serial.print("; Wind speed: ");
    WindSpeed();
    Serial.print(speedwind);
    Serial.print(" [m/s]");
    Serial.println();
    delay(2000);
}



// void setup() {
//     Serial.begin(115200);
//     Serial.println("Hello World!");

//     pinMode(A0, INPUT);
// }

// void loop() {
//     int mq2Value = analogRead(A0);
//     Serial.print("Smoke : ");
//     Serial.println(mq2Value);
//     delay(1000);
// }

// // Set Width to the number of displays wide you have
// const int WIDTH = 4;

// // You can change to a smaller font (two lines) by commenting this line,
// // and uncommenting the line after it:
// const uint8_t *FONT = Arial_Black_16;
// //const uint8_t *FONT = SystemFont5x7;

// const char *MESSAGE = "Hello Wolrd! - ";

// SPIDMD  dmd(WIDTH,1);  // DMD controls the entire display
// DMD_TextBox box(dmd);  // "box" provides a text box to automatically write to/scroll the display

// // the setup routine runs once when you press reset:
// void setup() {
//   Serial.begin(9600);
//   dmd.setBrightness(255);
//   dmd.selectFont(FONT);
//   dmd.begin();
// }

// // the loop routine runs over and over again forever:
// void loop() {
//   const char *next = MESSAGE;
//   while(*next) {
//     Serial.print(*next);
//     box.print(*next);
//     delay(200);
//     next++;
//   }
// }
