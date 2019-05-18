#include <Adafruit_LPS35HW.h>

Adafruit_LPS35HW lps35hw = Adafruit_LPS35HW();

void setup() {
  Serial.begin(115200);
  // Wait until serial port is opened
  while (!Serial) { delay(1); }

  Serial.println("Adafruit LPS35HW Relative Pressure Test");

  if (!lps35hw.begin_I2C()) {
    Serial.println("Couldn't find LPS35HW chip");
    while (1);
  }
  Serial.println("Found LPS35HW chip");
  Serial.println();
  Serial.print("Absolute Pressure: ");
  Serial.print(lps35hw.readPressure());
  Serial.println(" hPa");
  Serial.println();
  
  // zero pressure to the current measurement so that
  // pressure readings are relative to it

  lps35hw.zeroPressure();

}

void loop() {
  Serial.print("Pressure: ");
  Serial.print(lps35hw.readPressure());
  Serial.println(" hPa");

  Serial.println();
  delay(1000);
}