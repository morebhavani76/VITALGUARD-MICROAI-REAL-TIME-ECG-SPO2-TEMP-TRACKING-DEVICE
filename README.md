# VITALGUARD-MICROAI-REAL-TIME-ECG-SPO2-TEMP-TRACKING-DEVICE
This project is a real-time health monitoring system using ESP32 and TinyML to track ECG, SpO₂, and body temperature. I used ECG sensor, MAX30102, and DS18B20. It is programmed in C/C++ with Arduino IDE using required libraries. The system gives instant alerts through the Bluetooth Serial App without internet connectivity.
Objectives:
To design a real-time health monitoring system for ECG, SpO₂, and body temperature.To implement TinyML-based anomaly detection on-device without internet.To provide instant alerts for abnormal health conditions.To ensure low power consumption and portable design.To maintain data privacy with edge-based processing.
Technologies Used:
ESP32 Microcontroller,
Arduino IDE,
C and C++ Programming,
TensorFlow Lite for Microcontrollers (TinyML),
Embedded Systems & IoT Concepts,
Bluetooth Serial Communication,
Signal Filtering Techniques.
Sensors Used:
ECG Sensor (for heart signal monitoring).
MAX30102 Sensor (for SpO₂ and heart rate).
DS18B20 / LM35 Temperature Sensor (for body temperature).
How to Run:
To run this project, first connect the ECG sensor, MAX30102 (SpO₂ and heart rate) sensor, and DS18B20/LM35 temperature sensor properly to the ESP32 along with the LCD display if used. Install Arduino IDE and required libraries such as SparkFun MAX30102, DallasTemperature, LiquidCrystal, and TensorFlow Lite for Microcontrollers. Open the .ino file, select ESP32 board and correct COM port, then upload the code. After uploading, open the Serial Monitor to view real-time readings and use the Bluetooth Serial App to monitor data and receive instant alerts.
