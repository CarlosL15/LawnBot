#define POWER_PIN 19  // Pin to provide power to the rain sensor
#define DO_PIN 25     // Pin connected to the D0 pin of the rain sensor (digital output)
#define A0_PIN 34     // Pin connected to the A0 pin of the rain sensor (analog output)

bool previousRainState = false; // Variable to track the previous rain state

void setup() {
  // initialize serial communication
  Serial.begin(9600);
  
  // initialize the ESP32's pins
  pinMode(POWER_PIN, OUTPUT);  // configure the power pin as OUTPUT
  pinMode(DO_PIN, INPUT);      // configure the D0 pin as INPUT
}

void loop() {
  // Turn the rain sensor's power ON
  digitalWrite(POWER_PIN, HIGH);
  delay(10); // Wait 10 milliseconds for the sensor to stabilize

  // Read digital signal (rain detected or not)
  int rain_state = digitalRead(DO_PIN); // Read digital signal (HIGH/LOW)

  // Read the analog signal (rain intensity)
  int rain_intensity = analogRead(A0_PIN); // Read the analog value (0-4095)

  // Adjust the threshold to make it more sensitive
  int rain_threshold = 1000;  // Lower the threshold to trigger with less rain

  // Determine the current rain state based on digital output
  bool currentRainState = (rain_state == LOW);  // LOW means rain is detected (depending on your sensor)

  // Only print if the rain state has changed
  if (currentRainState != previousRainState) {
    if (currentRainState) {
      Serial.println("The rain is detected");
    } else {
      Serial.println("The rain is NOT detected");
    }

    // Update the previous state
    previousRainState = currentRainState;
  }

  // Print rain intensity regardless of state change
  Serial.print("Rain Intensity (analog): ");
  Serial.println(rain_intensity);  // Output the analog intensity value

  delay(1000); // Pause for 1 second
}

