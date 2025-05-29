#include <TFT_eSPI.h>
#include <Multichannel_Gas_GMXXX.h>
#include <Wire.h>
#include "Grove_Temperature_And_Humidity_Sensor.h"

// Initialize Gas Sensor
GAS_GMXXX<TwoWire> gas;

// Uncomment the sensor type you're using
#define DHTTYPE DHT22  // Change if using a different DHT model
#define DHTPIN 0       // Pin for DHT sensor
DHT dht(DHTPIN, DHTTYPE);

// Location configuration (set before deployment)
const int LOCATION = 1;  // 1 = office, 0 = kitchen

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Initialize DHT sensor
  dht.begin();

  // Initialize Multichannel Gas Sensor
  gas.begin(Wire, 0x08);  // Using hardware I2C with address 0x08

  Serial.println("Sensors Initialized Successfully");
}

void loop() {
  float temp_hum_val[2] = { 0 };

  // Read temperature & humidity
  if (!dht.readTempAndHumidity(temp_hum_val)) {
    float humidity = temp_hum_val[0];
    float temperature = temp_hum_val[1];

    // Print environmental data
    Serial.print(humidity, 1);  // 1 decimal place
    Serial.print(',');
    Serial.print(temperature, 1);
    Serial.print(',');
  } else {
    Serial.println("0.0,0.0,");  // Send zeros if read fails
  }

  // Read gas sensor data
  int NO2_val = gas.getGM102B();      // NO2 sensor
  int Ethanol_val = gas.getGM302B();  // Ethanol (C2H5OH) sensor
  int VOC_val = gas.getGM502B();      // VOC sensor
  int CO_val = gas.getGM702B();       // CO sensor

  // Print gas sensor data
  Serial.print(NO2_val); Serial.print(',');
  Serial.print(Ethanol_val); Serial.print(',');
  Serial.print(VOC_val); Serial.print(',');
  Serial.print(CO_val); Serial.print(',');

  // Print location and label placeholder
  Serial.print(LOCATION); Serial.print(',');
  Serial.println("label");  // Placeholder for Python to replace

  // Update every 2 seconds
  delay(2000);
}