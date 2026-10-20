
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <math.h>

#define MQ3_A A0
#define RELAY 7
#define BUZZER 6
#define BUTTON 2

#define IMPACT_THRESHOLD 3
#define ALCOHOL_THRESHOLD 600

LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_ADXL345_Unified accel =
    Adafruit_ADXL345_Unified(12345);

TinyGPSPlus gps;
SoftwareSerial gpsSerial(4, 3);
SoftwareSerial gsm(9, 8);

// Replace with your own number before using GSM features.
// Do not publish your personal phone number on GitHub.
String phoneNumber = "YOUR_PHONE_NUMBER";

bool accidentDetected = false;
bool alertSent = false;
unsigned long accidentTime = 0;

void sendSMS(String message) {
    gsm.print("AT+CMGS=\"");
    gsm.print(phoneNumber);
    gsm.println("\"");
    delay(1000);

    gsm.print(message);
    delay(500);
    gsm.write(26);
    delay(5000);
}

void makeCall() {
    gsm.print("ATD");
    gsm.print(phoneNumber);
    gsm.println(";");
    delay(20000);
    gsm.println("ATH");
}

void setup() {
    Serial.begin(9600);
    gpsSerial.begin(9600);
    gsm.begin(9600);

    pinMode(MQ3_A, INPUT);
    pinMode(RELAY, OUTPUT);
    pinMode(BUZZER, OUTPUT);
    pinMode(BUTTON, INPUT_PULLUP);

    digitalWrite(RELAY, HIGH);
    digitalWrite(BUZZER, LOW);

    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Smart Vehicle");
    lcd.setCursor(0, 1);
    lcd.print("System Start");
    delay(2000);
    lcd.clear();

    Serial.println("System Starting...");

    // GSM initialization
    gsm.println("AT");
    delay(1000);
    gsm.println("AT+CMGF=1");
    delay(1000);

    if (!accel.begin()) {
        Serial.println("ADXL345 ERROR");
        lcd.setCursor(0, 0);
        lcd.print("ADXL Error!");
        while (1);
    }

    accel.setRange(ADXL345_RANGE_16_G);

    lcd.setCursor(0, 0);
    lcd.print("Sensor Warmup");
    lcd.setCursor(0, 1);
    lcd.print("Please Wait");
    delay(15000);
    lcd.clear();
}

void loop() {
    // -------- GPS READ --------
    while (gpsSerial.available()) {
        gps.encode(gpsSerial.read());
    }

    // -------- ALCOHOL SENSOR --------
    int alcoholLevel = analogRead(MQ3_A);

    // -------- ACCELEROMETER --------
    sensors_event_t event;
    accel.getEvent(&event);

    float x = event.acceleration.x;
    float y = event.acceleration.y;
    float z = event.acceleration.z;

    float total = sqrt(x * x + y * y + z * z);
    float impact = fabs(total - 9.8);

    // -------- SERIAL OUTPUT --------
    Serial.print("Alcohol: ");
    Serial.print(alcoholLevel);
    Serial.print(" Impact: ");
    Serial.println(impact);

    // -------- LCD DISPLAY --------
    lcd.setCursor(0, 0);
    lcd.print("A:");
    lcd.print(alcoholLevel);
    lcd.print(" I:");
    lcd.print(impact, 1);
    lcd.print(" ");

    // -------- ALCOHOL DETECTION --------
    if (alcoholLevel > ALCOHOL_THRESHOLD) {
        lcd.setCursor(0, 1);
        lcd.print("ENGINE LOCKED ");
        digitalWrite(RELAY, LOW);
        digitalWrite(BUZZER, HIGH);
        delay(500);
        return;
    } else {
        if (!accidentDetected) {
            digitalWrite(RELAY, HIGH);
            digitalWrite(BUZZER, LOW);
        }
    }

    // -------- ACCIDENT DETECTION --------
    if (impact > IMPACT_THRESHOLD && !accidentDetected) {
        accidentDetected = true;
        alertSent = false;
        accidentTime = millis();

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ACCIDENT!");
        lcd.setCursor(0, 1);
        lcd.print("Engine Stopped");

        digitalWrite(RELAY, LOW);
        digitalWrite(BUZZER, HIGH);
        Serial.println("Accident Detected");
    }

    // -------- ACCIDENT MODE --------
    if (accidentDetected) {
        digitalWrite(RELAY, LOW);
        digitalWrite(BUZZER, HIGH);

        // Cancel the alert using the push button
        if (digitalRead(BUTTON) == LOW) {
            Serial.println("Accident Cancelled");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Alert Cancelled");

            digitalWrite(BUZZER, LOW);
            digitalWrite(RELAY, HIGH);

            accidentDetected = false;
            alertSent = false;
            delay(2000);
            lcd.clear();
        }

        // -------- SEND ALERT --------
        else if ((millis() - accidentTime > 10000)
                 && !alertSent) {
            Serial.println("ALERT SENT!");

            String message = "ACCIDENT ALERT!\n";

            if (gps.location.isValid()) {
                message += "Location:\n";
                message += "https://maps.google.com/?q=";
                message += String(gps.location.lat(), 6);
                message += ",";
                message += String(gps.location.lng(), 6);
            } else {
                message += "GPS not available";
            }

            sendSMS(message);
            makeCall();

            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Sending Alert");
            lcd.setCursor(0, 1);
            lcd.print("Location Sent");

            alertSent = true;
            delay(3000);
            lcd.clear();
        }
    }

    // -------- GPS DISPLAY --------
    if (!accidentDetected) {
        if (gps.location.isValid()) {
            lcd.setCursor(0, 1);
            lcd.print("Lat:");
            lcd.print(gps.location.lat(), 2);
            lcd.print(" ");

            delay(1500);

            lcd.setCursor(0, 1);
            lcd.print("Lng:");
            lcd.print(gps.location.lng(), 2);
            lcd.print(" ");
        } else {
            lcd.setCursor(0, 1);
            lcd.print("Waiting GPS... ");
        }
    }

    delay(500);
}
