#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Arduino.h>
#include "Talkie.h"
#include "Vocab_US_Large.h"
#include "Vocab_US_Clock.h"
#include "Vocab_US_Acorn.h"
#define buzzer 13
//#define trigPin 2
//#define echoPin 0
#define latchPin 11
#define clockPin 10
#define dataPin 12
#define motion A3
long duration;
int distance, initialDistance, currentDistance, i;

int attempt_counter = 0;
String password = "1234";
String tempPassword;
boolean displayMenu;
boolean triggered; // State of the alarm
boolean activateAlarm = false;
boolean alarmActivated = false;
boolean enteredPassword; // State of the entered password to stop the alarm
boolean passChangeMode = false;
boolean deactivated;
boolean passChanged = false;
unsigned long previousMillis = 0;
const long interval = 5000;
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char keypressed;
//define the buttons on the keypads
char keyMap[ROWS][COLS] = {
    { '1', '2', '3', 'A' },
    { '4', '5', '6', 'B' },
    { '7', '8', '9', 'C' },
    { '*', '0', '#', 'D' }
};
byte rowPins[ROWS] = { 9, 8, 7, 6 }; //Row pinouts of the keypad
byte colPins[COLS] = { 5, 4, 3, 2 }; //Column pinouts of the keypad
Keypad myKeypad = Keypad(makeKeymap(keyMap), rowPins, colPins, ROWS, COLS);
// Define LCD pinout
const int en = 2, rw = 1, rs = 0, d4 = 4, d5 = 5, d6 = 6, d7 = 7, bl = 3;

// Define I2C Address
const int i2c_addr = 0x27;
LiquidCrystal_I2C lcd(i2c_addr, en, rw, rs, d4, d5, d6, d7, bl, POSITIVE);

Talkie voice;

void setup()
{
    lcd.begin(16, 2);
    pinMode(latchPin, OUTPUT);
    pinMode(dataPin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(motion, INPUT); // Set motion sensor output
    pinMode(buzzer, OUTPUT); // Set buzzer as an output
    Serial.begin(9600);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("A - Arm System");
    lcd.setCursor(0, 1);
    lcd.print("B - Set Passcode");
    displayMenu = true;
}

void loop()
{
    if (displayMenu) {
        menuMode();
    }

    if (alarmActivated) {
        checkSensor();
    }
    if (triggered) {
        disarmAlarm();
    }
}
void checkSensor()
{
    int sensorVal;
    sensorVal = digitalRead(motion);
    Serial.println(sensorVal);

    delay(1000);
    if (sensorVal == 1) {
        tone(buzzer, 1000); // Send 1KHz sound signal
        lcd.backlight();
        lcd.clear();
        triggered = true;
    }
}
void disarmAlarm()
{
    int k = 5;
    tempPassword = "";
    triggered = true;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(" *** ALARM *** ");
    lcd.setCursor(0, 1);
    lcd.print("PIN>");
    while (triggered) {
        keypressed = myKeypad.getKey();
        if (keypressed != NO_KEY) {
            if (keypressed == '0' || keypressed == '1' || keypressed == '2' || keypressed == '3' || keypressed == '4' || keypressed == '5' || keypressed == '6' || keypressed == '7' || keypressed == '8' || keypressed == '9') {
                tempPassword += keypressed;
                lcd.setCursor(k, 1);
                lcd.print("*");
                k++;
            }
        }
        if (keypressed == '*') {
            if (tempPassword == password) {
                triggered = false;
                alarmActivated = false;
                deactivated = true;
                noTone(buzzer);
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("A - Arm System");
                lcd.setCursor(0, 1);
                lcd.print("B - Set Passcode");
                displayMenu = true;
            }
            else if (tempPassword != password) {
                attempt_counter++;
                for (int i = 0; i < 3; i++) {
                    lcd.backlight();
                    delay(250);
                    lcd.noBacklight();
                    delay(250);
                }
                lcd.backlight();
                lcd.setCursor(0, 1);
                lcd.print("* Incorrect PIN *");
                delay(1000);
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("*Alarm Activated*");
                lcd.setCursor(0, 1);
                lcd.print("Enter PIN");
                if (attempt_counter <= 2) {
                    lcd.backlight();
                    lcd.setCursor(0, 1);
                    lcd.print("* Incorrect PIN *");
                    delay(1000);
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("*Alarm Activated*");
                    lcd.setCursor(0, 1);
                    lcd.print("Enter PIN");
                }
                if (attempt_counter >= 3) {
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    alarm_lights();
                    lcd.print("*Alarm Activated*");
                    delay(1500);
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    alarm_lights();
                    lcd.print(" ***Police Have Been Notified*** ");
                    for (int positionCounter = 0; positionCounter < 65; positionCounter++) {
                        alarm_lights();
                        lcd.scrollDisplayLeft(); // scroll one position right
                        delay(100);
                    }
                    lcd.clear();
                    disarmAlarm();
                }
            }
        }
    }
}
void menuMode()
{
    keypressed = myKeypad.getKey();
    if (keypressed == 'A') { //If A is pressed, activate the alarm
        tone(buzzer, 1000, 200);
        activateAlarm = true;
        alarmActivated = true;
        setAlarm();
    }
    if (keypressed == 'B') {
        lcd.clear();
        changePassword();
    }
}
void changePassword()
{
    int i = 1;
    tone(buzzer, 2000, 100);
    tempPassword = "";
    lcd.setCursor(0, 0);
    lcd.print("Current Passcode");
    lcd.setCursor(0, 1);
    lcd.print(">");
    passChangeMode = true;
    passChanged = true;
    while (passChanged) {
        keypressed = myKeypad.getKey();
        if (keypressed != NO_KEY) {
            if (keypressed == '0' || keypressed == '1' || keypressed == '2' || keypressed == '3' || keypressed == '4' || keypressed == '5' || keypressed == '6' || keypressed == '7' || keypressed == '8' || keypressed == '9') {
                tempPassword += keypressed;
                lcd.setCursor(i, 1);
                lcd.print("*");
                i++;
                tone(buzzer, 2000, 100);
            }
        }
        if (i > 5 || keypressed == '#') {
            tempPassword = "";
            i = 1;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Current Passcode");
            lcd.setCursor(0, 1);
            lcd.print(">");
        }
        if (keypressed == '*') {
            i = 1;
            tone(buzzer, 2000, 100);
            if (password == tempPassword) {
                tempPassword = "";
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Set New Password");
                lcd.setCursor(0, 1);
                lcd.print(">");
                while (passChangeMode) {
                    keypressed = myKeypad.getKey();
                    if (keypressed != NO_KEY) {
                        if (keypressed == '0' || keypressed == '1' || keypressed == '2' || keypressed == '3' || keypressed == '4' || keypressed == '5' || keypressed == '6' || keypressed == '7' || keypressed == '8' || keypressed == '9') {
                            tempPassword += keypressed;
                            lcd.setCursor(i, 1);
                            lcd.print("*");
                            i++;
                            tone(buzzer, 2000, 100);
                        }
                    }
                    if (i > 5 || keypressed == '#') {
                        tempPassword = "";
                        i = 1;
                        tone(buzzer, 2000, 100);
                        lcd.clear();
                        lcd.setCursor(0, 0);
                        lcd.print("Set New Password");
                        lcd.setCursor(0, 1);
                        lcd.print(">");
                    }
                    if (keypressed == '*') {
                        i = 1;
                        tone(buzzer, 2000, 100);
                        password = tempPassword;
                        passChangeMode = false;
                        passChanged = false;
                        displayMenu = true;
                        lcd.clear();
                        lcd.setCursor(0, 0);
                        lcd.print("A - Arm System");
                        lcd.setCursor(0, 1);
                        lcd.print("B - Set Passcode");
                        menuMode();
                    }
                }
            }
        }
    }
}
void setAlarm()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Alarm will be");
    lcd.setCursor(0, 1);
    lcd.print("activated in");

    int countdown = 9; // 9 seconds count down before activating the alarm
    while (countdown != 0) {
        lcd.setCursor(13, 1);
        lcd.print(countdown);
        countdown_lights();
        countdown--;
        tone(buzzer, 700, 100);
        delay(1000);
    }
    tone(buzzer, 700, 5000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("System Armed");
    activateAlarm = false;
    alarmActivated = true;
    armed_lights();
    lcd.noBacklight();
}

void countdown_lights()
{
    byte LED1 = 0b11111111;
    byte LED2 = 0b00000000;
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, LED1);
    digitalWrite(latchPin, HIGH);
    delay(100);
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, LED2);
    digitalWrite(latchPin, HIGH);
    delay(100);
}

void armed_lights()
{
    byte grnLEDon = 0b00100000;
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, grnLEDon);
    digitalWrite(latchPin, HIGH);
    delay(300);
    byte grnLEDoff = 0b00000000;
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, grnLEDoff);
    digitalWrite(latchPin, HIGH);
    delay(300);

    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, grnLEDon);
    digitalWrite(latchPin, HIGH);
    delay(300);
}
void triggered_lights()
{
    byte redLEDon = 0b01000000;
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, redLEDon);
    digitalWrite(latchPin, HIGH);
    delay(100);
    byte redLEDoff = 0b01000000;
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, redLEDoff);
    digitalWrite(latchPin, HIGH);
    delay(100);
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, redLEDon);
    digitalWrite(latchPin, HIGH);
    delay(100);
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, redLEDoff);
    digitalWrite(latchPin, HIGH);
    delay(100);
}

void alarm_lights()
{

    byte redLEDon = 0b01000000;
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, redLEDon);
    digitalWrite(latchPin, HIGH);
    delay(100);
    byte redLEDoff = 0b01000000;
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, redLEDoff);
    digitalWrite(latchPin, HIGH);
    delay(100);
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, redLEDon);
    digitalWrite(latchPin, HIGH);
    delay(100);
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, redLEDoff);
    digitalWrite(latchPin, HIGH);
    delay(100);
    byte blueLEDon = 0b10000000;
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, blueLEDon);
    digitalWrite(latchPin, HIGH);
    delay(100);
    byte blueLEDoff = 0b00000000;
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, blueLEDoff);
    digitalWrite(latchPin, HIGH);
    delay(100);
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, blueLEDon);
    digitalWrite(latchPin, HIGH);
    delay(100);
    digitalWrite(latchPin, LOW);
    shiftOut(dataPin, clockPin, LSBFIRST, blueLEDoff);
    digitalWrite(latchPin, HIGH);
    delay(100);
}
