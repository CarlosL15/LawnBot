#include <SoftwareSerial.h>
#include <BluetoothSerial.h>
#include <TinyGPSPlus.h>
#include <SPI.h>
#include <SD.h>
#include <Bluepad32.h>

// === MOTOR DRIVER PINS (L298N) ===
int ENApin = 14; // Motor 1 speed
int IN1pin = 27; // Motor 1 dir1
int IN2pin = 26; // Motor 1 dir2
int IN3pin = 25; // Motor 2 dir1
int IN4pin = 33; // Motor 2 dir2
int ENBpin = 32; // Motor 2 speed

// === GPS PINS ===
SoftwareSerial gpsSerial(17, 16); // RX, TX
TinyGPSPlus gps;

// === SD CARD SETTINGS ===
#define SD_CS 5  
File logFile;
char dataFilename[] = "/LB00.csv";
bool sdActive = false;

// === RAIN SENSOR PINS ===
#define POWER_PIN 19
#define DO_PIN 25
#define A0_PIN 34
bool previousRainState = false;
bool returningToStart = false;

// === BLUETOOTH CONTROLLER ===
ControllerPtr myControllers[BP32_MAX_GAMEPADS];

// === POSITION TRACKING ===
float startLatitude, startLongitude;
bool startPositionSet = false;

// === MOTOR CONTROL FUNCTIONS ===
void moveForward(int speed) {
    digitalWrite(IN1pin, HIGH);
    digitalWrite(IN2pin, LOW);
    analogWrite(ENApin, speed);
    digitalWrite(IN3pin, HIGH);
    digitalWrite(IN4pin, LOW);
    analogWrite(ENBpin, speed);
}

void moveBackward(int speed) {
    digitalWrite(IN1pin, LOW);
    digitalWrite(IN2pin, HIGH);
    analogWrite(ENApin, speed);
    digitalWrite(IN3pin, LOW);
    digitalWrite(IN4pin, HIGH);
    analogWrite(ENBpin, speed);
}

void stopMotors() {
    analogWrite(ENApin, 0);
    analogWrite(ENBpin, 0);
}

// === SD CARD FUNCTIONS ===
bool sdSetup() {
    pinMode(SD_CS, OUTPUT);
    if (!SD.begin(SD_CS)) {
        Serial.println("SD Card initialization failed!");
        return false;
    }
    Serial.println("SD Card initialized.");

    for (byte i = 0; i < 100; i++) {
        dataFilename[3] = '0' + i / 10;
        dataFilename[4] = '0' + i % 10;

        if (!SD.exists(dataFilename)) {
            logFile = SD.open(dataFilename, FILE_WRITE);
            if (logFile) {
                Serial.printf("Created file: %s\n", dataFilename);
                logFile.println("Timestamp (ms),Latitude,Longitude,Altitude (m),Speed (km/h),Satellites");
                logFile.close();
                sdActive = true;
                return true;
            } else {
                Serial.println("Failed to create file.");
                return false;
            }
        }
    }
    Serial.println("No available filenames left.");
    return false;
}

void logData(const String &Data) {
    if (!sdActive) return;
    logFile = SD.open(dataFilename, FILE_APPEND);
    if (logFile) {
        logFile.println(Data);
        logFile.close();
    } else {
        Serial.println("Failed to write to SD file.");
    }
}

// === GPS FUNCTIONS ===
bool gpsCheck() {
    unsigned long startTime = millis();
    Serial.print("Waiting for GPS fix...");
    while (millis() - startTime < 30000) {
        while (gpsSerial.available()) gps.encode(gpsSerial.read());
        if (gps.location.isValid() && gps.satellites.value() > 0) {
            Serial.println("\n✅ GPS is running!");
            return true;
        }
    }
    Serial.println("\n❌ No GPS fix found!");
    return false;
}

// === BLUETOOTH CONTROLLER CALLBACKS ===
void onConnectedController(ControllerPtr ctl) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == nullptr) {
            Serial.printf("CALLBACK: Controller connected, index=%d\n", i);
            myControllers[i] = ctl;
            return;
        }
    }
    Serial.println("CALLBACK: No empty slot for new controller.");
}

void onDisconnectedController(ControllerPtr ctl) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == ctl) {
            Serial.printf("CALLBACK: Controller disconnected from index=%d\n", i);
            myControllers[i] = nullptr;
            return;
        }
    }
}

// === PROCESS CONTROLLER INPUT ===
void processGamepad(ControllerPtr ctl) {
    if (returningToStart) return;

    int motorSpeed = map(abs(ctl->axisY()), 25, 512, 70, 255);

    if (ctl->axisY() <= -25) moveForward(motorSpeed);
    if (ctl->axisY() >= 25) moveBackward(motorSpeed);
    if (ctl->axisY() > -25 && ctl->axisY() < 25) stopMotors();
}

void processControllers() {
    for (auto myController : myControllers) {
        if (myController && myController->isConnected() && myController->hasData()) {
            processGamepad(myController);
        }
    }
}

// === RAIN SENSOR FUNCTION ===
void checkRainSensor() {
    digitalWrite(POWER_PIN, HIGH);
    delay(10);

    int rain_state = digitalRead(DO_PIN);
    bool currentRainState = (rain_state == LOW);

    if (currentRainState != previousRainState) {
        if (currentRainState) {
            Serial.println("Rain detected! Returning to start...");
            returningToStart = true;
        } else {
            Serial.println("Rain stopped. Resuming manual control.");
            returningToStart = false;
        }
        previousRainState = currentRainState;
    }
}

// === RETURN TO START FUNCTION ===
void returnToStart() {
    if (!returningToStart || !startPositionSet) return;

    float latitude = gps.location.lat();
    float longitude = gps.location.lng();
    
    float distanceToStart = TinyGPSPlus::distanceBetween(latitude, longitude, startLatitude, startLongitude);

    if (distanceToStart > 1.0) {  
        moveBackward(150);
        Serial.printf("Returning... Distance to start: %.2f m\n", distanceToStart);
    } else {
        stopMotors();
        Serial.println("Back at starting position. Waiting for rain to stop...");
        returningToStart = false;
    }
}

// === SETUP FUNCTION ===
void setup() {
    Serial.begin(115200);
    gpsSerial.begin(9600);

    pinMode(ENApin, OUTPUT);
    pinMode(IN1pin, OUTPUT);
    pinMode(IN2pin, OUTPUT);
    pinMode(IN3pin, OUTPUT);
    pinMode(IN4pin, OUTPUT);
    pinMode(ENBpin, OUTPUT);

    pinMode(POWER_PIN, OUTPUT);
    pinMode(DO_PIN, INPUT);

    BP32.setup(&onConnectedController, &onDisconnectedController);
    BP32.forgetBluetoothKeys();
    BP32.enableVirtualDevice(false);

    sdSetup();

    if (gpsCheck()) {
        startLatitude = gps.location.lat();
        startLongitude = gps.location.lng();
        startPositionSet = true;
        Serial.printf("Start Position: %.6f, %.6f\n", startLatitude, startLongitude);
    }
}

// === LOOP FUNCTION ===
void loop() {
    BP32.update();
    checkRainSensor();
    returnToStart();

    while (gpsSerial.available()) gps.encode(gpsSerial.read());
    
    if (!returningToStart) processControllers();

    delay(200);
}
