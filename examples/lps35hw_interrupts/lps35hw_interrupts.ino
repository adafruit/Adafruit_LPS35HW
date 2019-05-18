#include <Adafruit_LPS35HW.h>

Adafruit_LPS35HW lps35hw = Adafruit_LPS35HW();

void setup() {
  Serial.begin(115200);
  // Wait until serial port is opened
  while (!Serial) { delay(1); }

  Serial.println("Adafruit LPS35HW Pressure threshold Test");

  if (!lps35hw.begin_I2C()) {
    Serial.println("Couldn't find LPS35HW chip");
    while (1);
  }
  Serial.println("Found LPS35HW chip");
  Serial.println();

  // zero pressure to the current measurement so that
  // because low thresholds only work in relative mode
  lps35hw.zeroPressure();
  
  // set the pressure threshold
  Serial.println("Setting threshold to 10 hPa");
  lps35hw.setThresholdPressure(10);

  lps35hw.enableHighThreshold();

  // uncomment to enable the low threshold.
  // can behave strangely if used at the same time
  // as enableHighThreshold since they share a threshold
//  lps35hw.enableLowThreshold();

  // by default the INT pin will be active high.
  // Connect an LED cathode with current limiting resistor to GND
  // and the LED anode to INT
  lps35hw.enableInterrupts();

  // Uncomment the line below to configure the INT pin as open drain.
  // Connect an LED cathode with current limiting resistor to INT
  // and the LED anode to an appropriate voltage source like 3VO
//  lps35hw.enableInterrupts(true);
  
  // uncomment to disable interrupts
  //  lps35hw.disableInterrupts();

}

void loop() {
  Serial.print("Pressure: ");
  Serial.print(lps35hw.readPressure());
  Serial.println(" hPa");
  
  if (lps35hw.highThresholdExceeded()){
    Serial.println("High threshold exceeded");
  }
//   uncomment this delay to take time for the interrupts to 
//   trigger again if checking both
//   delay(100);
//  
//  if (lps35hw.lowThresholdExceeded()){
//    Serial.println("Low threshold exceeded");
//  }

  Serial.println();
  delay(1000);
}