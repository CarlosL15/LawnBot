#include <SoftwareSerial.h>
#include <BluetoothSerial.h>
#include <TinyGPSPlus.h>
#include <SPI.h>
#include <SD.h>

// Initialize GPS Serial (UART2)
SoftwareSerial gpsSerial(17, 16); // RX, TX
TinyGPSPlus gps;


// Init Bluetooth Serial
BluetoothSerial ESP_BT;

// Define SD Card SPI Pins
#define SD_CS 5  

File logFile;
char dataFilename[] = "/LB00.csv";  // Template for dynamic filenames
bool sdActive = false;

// Function to initialize SD card and create a unique file
bool sdSetup() {
    pinMode(SD_CS, OUTPUT);
    if (!SD.begin(SD_CS)) {
        Serial.println("SD Card initialization failed!");
        return false;
    }
    Serial.println("SD Card initialized.");

    // Try different filenames: LB00.csv -> LB99.csv
    for (byte i = 0; i < 100; i++) {
        dataFilename[3] = '0' + i / 10;  // Tens place
        dataFilename[4] = '0' + i % 10;  // Ones place

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

// Function to log data to the SD card
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

// Function to check if GPS is running
bool gpsCheck() {
    unsigned long startTime = millis();
    Serial.print("Waiting for GPS fix...");

    while (millis() - startTime < 30000) {  // Wait up to 30 seconds
        while (gpsSerial.available()) {
            gps.encode(gpsSerial.read());
        }

        if (gps.location.isValid() && gps.satellites.value() > 0) {
            Serial.println("\n✅ GPS is running and has a fix!");
            return true;
        }
    }

    Serial.println("\n❌ GPS fix not found! Check connections and try again.");
    return false;
}

void setup() {
    Serial.begin(115200);  // Debug output
    gpsSerial.begin(9600);  // GPS TX → GPIO 16, RX → GPIO 17

    // Initialize SD Card and find an available filename
    sdSetup();

    // Check if GPS is running
    if (!gpsCheck()) {
        Serial.println("⚠️ GPS is not functioning correctly. Logging will not continue.");
        while (1);  // Halt execution if no GPS fix
    }
}

void loop() {
    while (gpsSerial.available()) {
        gps.encode(gpsSerial.read());
    }

    if (gps.location.isUpdated()) {
        float latitude = gps.location.lat();
        float longitude = gps.location.lng();
        float altitude = gps.altitude.meters();
        float speed = gps.speed.kmph();
        int satellites = gps.satellites.value();
        unsigned long timestamp = millis();

        // Print to Serial Monitor
        Serial.printf("Time: %lu, Lat: %.6f, Lon: %.6f, Alt: %.2f m, Speed: %.2f km/h, Sats: %d\n", 
                      timestamp, latitude, longitude, altitude, speed, satellites);

        // Format data as CSV and log to SD card
        String logEntry = String(timestamp) + "," + String(latitude, 6) + "," + 
                          String(longitude, 6) + "," + String(altitude, 2) + "," + 
                          String(speed, 2) + "," + String(satellites);
        logData(logEntry);
    }

    delay(2000);  // Update every 2 seconds
}
