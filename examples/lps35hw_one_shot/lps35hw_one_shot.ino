#include <Adafruit_LPS35HW.h>

Adafruit_LPS35HW lps35hw = Adafruit_LPS35HW();

void setup() {
  Serial.begin(115200);
  // Wait until serial port is opened
  while (!Serial) { delay(1); }

  Serial.println("Adafruit LPS35HW One shot mode Test");

  if (!lps35hw.begin_I2C()) {
    Serial.println("Couldn't find LPS35HW chip");
    while (1);
  }
  Serial.println("Found LPS35HW chip");
  Serial.println();

  // set the data rate to one shot mode so we can take individual measurements
  lps35hw.setDataRate(LPS35HW_RATE_ONE_SHOT);
  
}

void loop() {
  
  Serial.println("------- First measurement ---------");
  // take an initial measurement
  lps35hw.takeMeasurement();
  for (int i=0; i < 3; i++){
    Serial.print("First Temperature: ");
    Serial.print(lps35hw.readTemperature());
    Serial.println(" C");
    
    Serial.print("First Pressure: ");
    Serial.print(lps35hw.readPressure());
    Serial.println(" hPa");
    Serial.println();

    delay(1000);

  }
  
  delay(1000);
  Serial.println("------- Second measurement ---------");
  // take an new measurement
  // the value will be the same because the one shot
  // data rate only updates the temperature and pressure
  // when takeMeasurement is called
  lps35hw.takeMeasurement();
  for (int i=0; i < 3; i++){
    Serial.print("Second Temperature: ");
    Serial.print(lps35hw.readTemperature());
    Serial.println(" C");
    
    Serial.print("Second Pressure: ");
    Serial.print(lps35hw.readPressure());
    Serial.println(" hPa");
    Serial.println();
    
    delay(1000);
  }
  delay(2000);
}