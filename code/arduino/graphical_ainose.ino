#include <sample_ainose_inferencing.h>
#include <Multichannel_Gas_GMXXX.h>
#include <Wire.h>
#include "Grove_Temperature_And_Humidity_Sensor.h"
#include "TFT_eSPI.h"
#include "dog_nose.c"
#include <SparkFunBQ27441.h>  // Battery monitoring

GAS_GMXXX<TwoWire> gas;
#define DHTTYPE DHT22
#define DHTPIN 0
DHT dht(DHTPIN, DHTTYPE);

TFT_eSPI tft = TFT_eSPI();  // LCD object
const unsigned int BATTERY_CAPACITY = 650; // mAh

void setupBQ27441() {
  if (!lipo.begin()) {
    Serial.println("Error: Unable to communicate with BQ27441.");
    tft.setTextColor(TFT_RED);
    tft.setCursor(10, 10);
    tft.print("SOC Fail");
    while (1);
  }
  lipo.setCapacity(BATTERY_CAPACITY);
  Serial.println("Battery Monitor Initialized.");
}


void setup() {
  Serial.begin(115200);
  Wire.begin();

  dht.begin();
  gas.begin(Wire, 0x08);
  setupBQ27441();


  // Initialize display
  tft.begin();
  tft.setRotation(3); // Landscape mode
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.drawRect(5, 5, 248, 230, TFT_WHITE);  // Border for layout

  tft.setFreeFont(&FreeSansBoldOblique9pt7b);
  tft.setCursor(32, 65);
  tft.println("ARTIFICIAL");
  tft.setCursor(85, 110);
  tft.println("NOSE");
  tft.pushImage(100, 140, 70, 70, dog_nose);

  delay(5000);
}

void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT22 sensor!");
    delay(1000);
    return;
  }

  float no2 = gas.getGM102B();
  float ethanol = gas.getGM302B();
  float voc = gas.getGM502B();
  float co = gas.getGM702B();

  float features[] = { humidity, temperature, no2, ethanol, voc, co };

  signal_t signal;
  numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);

  ei_impulse_result_t result;
  EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);

  const char* detected_smell = "";
  float max_value = 0.0f;

  for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
    if (result.classification[ix].value > max_value) {
      max_value = result.classification[ix].value;
      detected_smell = result.classification[ix].label;
    }
  }

  Serial.print("Detected smell: ");
  Serial.print(detected_smell);
  Serial.print(" with accuracy: ");
  Serial.print(max_value * 100, 2);
  Serial.println("%");

  // ==== Display on LCD ====
  tft.fillRect(6, 6, 246, 228, TFT_BLACK);  // Clear inside of border

  // Border
  tft.drawRect(5, 5, 248, 230, TFT_WHITE);

   // Display SOC in top-left corner
  unsigned int soc = lipo.soc();  // Read battery percentage
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(10, 25);
  tft.print(String(soc) + "%");
  

  // SMELL label
  tft.setTextSize(2);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setCursor(75,40); // Approx center
  tft.println("SMELL");

  // Detected smell value
  tft.setTextSize(1);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  int labelWidth = strlen(detected_smell) * 12; // 6 px * size(2)
  tft.setCursor(120, 70);
  tft.println(detected_smell);

  // Accuracy label
  tft.setTextSize(2);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(30,120);
  tft.println("ACCURACY");

  // Accuracy value
  char accStr[10];
  dtostrf(max_value * 100, 4, 2, accStr);
  strcat(accStr, " %");
  int accWidth = strlen(accStr) * 12;
  tft.setTextSize(1);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.setCursor(100, 155);
  tft.println(accStr);

  // Accuracy bar (within 248 px border)
  int barMaxWidth = 160;
  int barX = 60;  // Centered
  int barY = 190;
  int barHeight = 10;
  int barWidth = (int)(max_value * barMaxWidth);
  tft.fillRect(barX, barY, barWidth, barHeight, TFT_GREEN);
  tft.drawRect(barX, barY, barMaxWidth, barHeight, TFT_WHITE);

  delay(2000);
}
