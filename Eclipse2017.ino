// This is an revision of Cedric and Nick's HAB Code from 2017. This revision is edited by Matt.
#include <SD.h>
#include <Wire.h>
#include <SPI.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_VC0706.h>
#include <SoftwareSerial.h>

File dataFile;

DHT dht(DHTPIN, DHTTYPE);
int led = 9;

#define chipSelect 53

#if ARDUINO >= 100

// On Mega: camera TX connected to pin 69 (A15), camera RX to pin 3:
SoftwareSerial cameraconnection = SoftwareSerial(69, 3);
#else
NewSoftSerial cameraconnection = NewSoftSerial(2, 3);
#endif

Adafruit_VC0706 cam = Adafruit_VC0706(&cameraconnection);

void setup() {

#if !defined (SOFTWARE_SPI)
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  if (chipSelect != 53) pinMode(53, OUTPUT)
#else
  if (chipSelect !=10) pinMode(10, OUTPUT)
#endif
#endif

  Serial.begin(9600);

  Serial.println("Initializing SD Card...");
  if (!SD.begin(53))
  {
    Serial.println("SD initialization failed.");
    return;
  }
  Serial.println("SD initialization completed.");

// Now to locate the camera...
if (cam.begin()) {
  Serial.println("Camera Found.");
} else {
  Serial.println("No Camera Found.");
  return;
}

// This is the camera's resolution (640x480)
cam.setImageSize(VC0706_640x480);

void loop() {


  if (! cam.takePicture())
    Serial.println("Failed to take picture.");
  else
    Serial.println("");
    Serial.println("Picture taken.");

    // Here is where we give the image a name.
    char filename[13];
    strcpy(filename, "IMAGE00.JPG");
    for (int i = 0; i < 100; i++) {
      filename[5] = '0' + i / 10;
      filename[6] = '0' + i % 10;

      // And if that file name already exists...
      if (! SD.exists(filename))  {
        break;
      }
    }

    File imgFile = SD.open(filename, FILE_WRITE);

      uint16_t jpglen = cam.frameLength();
      Serial.print("Storing ");
      Serial.print(jpglen, DEC);
      Serial.print(" byte image.");

      int32_t time = millis();
      pinMode(8, OUTPUT);
      byte wCount = 0; //
      while (jpglen > 0) {

        uint8_t *buffer;
        uint8_t bytesToRead = min(32, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
        buffer = cam.readPicture(bytesToRead);
        imgFile.write(buffer, bytesToRead);
        if(++wCount >= 64) {
          Serial.print('.');
          wCount = 0;
        }

        jpglen -= bytesToRead;
      }
      imgFile.close();

      time = millis() - time;
      Serial.println("Done.");
      Serial.print(time); Serial.println(" ms elapsed");
    }
