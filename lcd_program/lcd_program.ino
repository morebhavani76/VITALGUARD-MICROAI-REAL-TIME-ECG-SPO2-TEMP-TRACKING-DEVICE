// ====== Libraries ======
#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "BluetoothSerial.h"
#include <LiquidCrystal_I2C.h>

// ---------- Pins ----------
#define ONE_WIRE_BUS 4
#define ECG_PIN 34  // AD8232 output -> GPIO34

// ---------- Objects ----------
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
MAX30105 particleSensor;
BluetoothSerial SerialBT;
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Change to 0x3F if needed

// ---------- Variables ----------
#define BUFFER_SIZE 100
uint32_t irBuffer[BUFFER_SIZE];
uint32_t redBuffer[BUFFER_SIZE];
int32_t spo2;
int8_t validSPO2;
int32_t heartRate;
int8_t validHeartRate;

float w1 = 0.12f, w2 = 0.08f, w3 = -0.09f, b = -6.2f;
float sigmoid(float x) { return 1.0f / (1.0f + exp(-x)); }

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  sensors.begin();
  pinMode(ECG_PIN, INPUT);

  // LCD Init
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ESP32 Health Mon");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(2000);
  lcd.clear();

  // Bluetooth init
  SerialBT.begin("ESP32_HealthMonitor");
  Serial.println("Bluetooth Ready — Pair with 'ESP32_HealthMonitor'");
  
  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println(" MAX30102 not found!");
    lcd.clear();
    lcd.print("MAX30102 Error!");
    while (1);
  }
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x24);
  particleSensor.setPulseAmplitudeIR(0x24);
  particleSensor.setPulseAmplitudeGreen(0);
}

// ---------- Rule-based logic ----------
String interpretByRule(int32_t hr, int8_t hr_valid, int32_t sp, int8_t sp_valid, float temp) {
  if (hr_valid && sp_valid) {
    if (sp >= 95 && hr >= 50 && hr <= 100)
      return "✅ Normal";
  }
  if (sp_valid && sp < 92)
    return "⚠️ Low Oxygen";
  if (temp > 38.0 && hr_valid && hr > 100)
    return "🔥 Fever/High HR";
  return String("");
}

// ---------- ML fallback ----------
String mlFallback(float temp, int32_t hr, int32_t sp) {
  float t_norm = (temp - 36.5f) / 2.0f;
  float hr_norm = (hr - 75.0f) / 25.0f;
  float sp_norm = (sp - 97.0f) / 3.0f;
  float x = (w1 * t_norm) + (w2 * hr_norm) + (w3 * sp_norm) + b;
  float out = sigmoid(x);

  if (out > 0.75f)
    return "🔥 Fever (ML)";
  else if (out < 0.35f)
    return "⚠️ Fatigue (ML)";
  else
    return "✅ Normal (ML)";
}

void loop() {
  // ECG Read
  int ecgValue = analogRead(ECG_PIN);
  float ecgVoltage = (ecgValue / 4095.0) * 3.3;

  // SpO2 + HR
  long irRaw = particleSensor.getIR();
  if (irRaw < 5000) {
    SerialBT.println("No Finger Detected – waiting...");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Place finger...");
    delay(1000);
    return;
  }

  for (int i = 0; i < BUFFER_SIZE; i++) {
    while (!particleSensor.available()) particleSensor.check();
    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample();
  }

  maxim_heart_rate_and_oxygen_saturation(irBuffer, BUFFER_SIZE, redBuffer,
                                         &spo2, &validSPO2,
                                         &heartRate, &validHeartRate);

  // Temperature
  sensors.requestTemperatures();
  float temperature = sensors.getTempCByIndex(0);

  // Decision
  String decision = interpretByRule(heartRate, validHeartRate, spo2, validSPO2, temperature);
  if (decision == "") decision = mlFallback(temperature, heartRate, spo2);

  // Format message
  String msg = "🌡 Temp: " + String(temperature, 2) + "°C | 💓 HR: " + String(heartRate) +
               " | SpO₂: " + String(spo2) + " | ⚡ ECG: " + String(ecgVoltage, 2) +
               "V\n " + decision + "\n";

  // Send over Bluetooth & Serial
  Serial.println(msg);
  SerialBT.println(msg);

  // --- LCD Display ---
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temperature, 1);
  lcd.print((char)223); // Degree symbol
  lcd.print(" HR:");
  lcd.print(heartRate);
  
  lcd.setCursor(0, 1);
  lcd.print("SpO2:");
  lcd.print(spo2);
  lcd.print("% ");
  lcd.print(decision.substring(0, 6)); // short form of status

  delay(2000);
}
