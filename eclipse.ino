#include <SD.h>
#include <Wire.h>
#include <SPI.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <Adafruit_BMP183.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SoftwareSerial.h>

// BEGIN WIRE CONFIGS //

// Oled wire pins
#define sclk 12
#define mosi 11
#define dc   10
#define cs   9
#define rst  8

// Color definitions
#define  BLACK           0x0000
#define BLUE            0x001F
#define RED             0xF800
#define GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0
#define WHITE           0xFFFF

File dataFile;

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
Adafruit_SSD1351 tft = Adafruit_SSD1351(cs, dc, mosi, sclk, rst);

#define BMP183_CLK  4 //BMP183 commmunications pins: was CLK
#define BMP183_SDO  5
#define BMP183_SDI  6
#define BMP183_CS   7

#define DHTPIN 2
#define DHTTYPE DHT22

Adafruit_BMP183 bmp = Adafruit_BMP183(BMP183_CLK, BMP183_SDO, BMP183_SDI, BMP183_CS);

DHT dht(DHTPIN, DHTTYPE);
int led = 9;

#define chipSelect 53

void setup() {
  Serial.begin(9600); //start serial communication at 9600

  // Commence boot on oled
  tft.begin();
  tft.fillScreen(BLACK);
  tft.setTextSize(1);
  tft.setTextColor(GREEN);
  tft.setCursor(0, 0);
  tft.println("CVCC Eclipse v1.0");
  tft.println("---------------------");

  delay(1500);
  tft.println(">Initializing Sensors");

  Serial.println("Initializing the SD card");
  tft.println("Initializing SD card");

  if (!SD.begin(53)) // if statement to test for SD initialization
  {
    Serial.println("SD Initialization failed");
    tft.println("SD Initialization failed");

    return;
  }
  Serial.println("SD Initialization complete");
  tft.println("SD Init. done!");

  Serial.println();

  Serial.println("Initializing Accelerometer");
  tft.println("Initializing ADXL345");

  if (!accel.begin())
  {
    Serial.println("No Accelerometer detected");
    tft.println("No Accelerometer detected");

    return;
  }
  Serial.println("Accelerometer Initialization complete");
  tft.println("ADXL345 Init. done!");

  Serial.println();

  /* Set the range to whatever is appropriate for your project */
  accel.setRange(ADXL345_RANGE_16_G);
  // tftSetRange(ADXL345_RANGE_8_G);
  // tftSetRange(ADXL345_RANGE_4_G);
  // tftSetRange(ADXL345_RANGE_2_G);

  Serial.println("Initializing Pressure/Temperature Sensor");
  tft.println("Initializing BMP183");

  if (!bmp.begin())
  {
    Serial.println("No BMP183 detected");
    Serial.println();
    tft.println("No BMP183 detected!");
    tft.println("---------------------");
    return;
  }
  Serial.println("Pressure/Temperature Sensor detected");
  tft.println("BMP183 detected!");
  tft.println("---------------------");
  tft.println("Sensor Data:");
  tft.println("");

  Serial.println();

  pinMode(led, OUTPUT);
}

// Loop to record data to a .csv file every 6 seconds. Program by Nick, Cedric, and Alex
void loop() {

  dataFile = SD.open("HawkData.csv", FILE_WRITE); //open the file to write to it

  if (dataFile) {

    delay(1000);
    tft.begin();
    tft.fillScreen(BLACK);
    tft.setTextSize(1);
    tft.setTextColor(GREEN);
    tft.setCursor(0, 0);
    tft.println("Realtime sensor data:");
    tft.println("---------------------");

    /* Get a new sensor event */
    sensors_event_t event;
    accel.getEvent(&event);
    Serial.print("X: "); Serial.print(event.acceleration.x); Serial.print("  ");
    Serial.print("Y: "); Serial.print(event.acceleration.y); Serial.print("  ");
    Serial.print("Z: "); Serial.print(event.acceleration.z); Serial.print("  "); Serial.println("m/s^2 ");

    /* tft atmospheric pressue in Pascals */
    float pressure;
    pressure = bmp.getPressure();
    Serial.print("Pressure: ");
    Serial.print(pressure);
    Serial.print(" Pascals / ");
    Serial.print(bmp.getPressure() / 100);
    Serial.println(" millibar (hPa)");
    tft.print("Pressure: ");
    tft.print(pressure);
    tft.print(" Pascals / ");
    tft.print(bmp.getPressure() / 100);
    tft.println(" millibar (hPa)");

    // TODO check local sea level pressure for accurate altitude readings
    float seaLevelPressure = 1022.9;
    Serial.print("Sea level pressure: ");
    Serial.print(seaLevelPressure);
    Serial.println(" millibar/hPa");
    tft.print("Sea level pressure: ");
    tft.print(seaLevelPressure);
    tft.println(" millibar/hPa");

    float altitude;
    altitude = bmp.getAltitude(seaLevelPressure);
    Serial.print("Altitude: ");
    Serial.print(altitude * 3.28084);
    Serial.println(" ft");
    tft.print("Altitude: ");
    tft.print(altitude * 3.28084);
    tft.println(" ft");

    float insideTemperature;
    insideTemperature = bmp.getTemperature();
    Serial.print("Inside Temperature: ");
    Serial.print(insideTemperature * 1.8 + 32);
    Serial.println(" f");
    tft.println("Temperature: ");
    tft.print(insideTemperature * 1.8 + 32);
    tft.println();


    Serial.println("");


    String dataString = String(event.acceleration.x) + ", " + String(event.acceleration.y) + ", " + String(event.acceleration.z);

    dataFile.print(millis() / 1000);
    dataFile.print(",");
    dataFile.print(dataString); //x,y,z
    dataFile.print(",");
    dataFile.print(pressure);
    dataFile.print(",");
    dataFile.println(altitude * 3.28084);
    dataFile.print(",");
    dataFile.print(insideTemperature);
    dataFile.print(",");

    dataFile.close();
    Serial.println("");

    delay(5000);
    tft.println("");
    tft.fillScreen(BLACK);

  } else {

    Serial.println("Error in opening file");

  }

  delay(200);
  digitalWrite (led, HIGH);
  delay(200);
  digitalWrite (led, LOW);
  delay (200);
} // Delay for next capture currently 3000 miliseconds, can be changed.
