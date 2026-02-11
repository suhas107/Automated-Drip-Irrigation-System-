/*
 * AUTOMATED DRIP IRRIGATION SYSTEM 
 *
 * Functionalities: (menu driven program)
 *
 * 1. Support for Different Land Types:
 *    - The project accommodates various soil types by using structures to define different land characteristics 
 *      and functionalities for each type.
 *
 * 2. User Input for Threshold: 
 *    - The user specifies a moisture threshold. 
 *    - The water pump (simulated using a DC motor) releases water until the soil moisture level,
 *      measured by a moisture sensor, reaches this threshold.
 *
 * 3. Manual Override:
 *    - The user can manually control the water pump using a push button (ON or OFF).
 *    - When ON, the DC motor rotates to release water; when OFF, the motor stops.
 *
 * 4. Threshold Directory:
 *    - The system maintains an array that stores the last 10 moisture thresholds set by the user.
 */

// Pin Definitions
#define moisturePin A0       // Moisture sensor input pin
#define pumpPin 7            // Pump (DC motor control via transistor) connected to digital pin 7
#define buttonPin 8          // Manual override button
#define ledPinLand1 13      // LED for Land 1 manual mode indicator
#define ledPinLand2 12      // LED for Land 2 manual mode indicator
#define ledPinLand3 11      // LED for Land 3 manual mode indicator

// Union for pump control flags and states
typedef union {
    struct {
        unsigned int manualOverride : 1; // 1 bit to indicate if manual operation is active
        unsigned int pumpState : 1;      // 1 bit to indicate if the pump is ON (1) or OFF (0)
    } bits;
    unsigned int allFlags;               // Used for combined flag manipulation
} PumpControl;

// Struct (Land) representing the parameters and states of a specific land type
typedef struct {
    int moistureThreshold;      // Desired moisture level to trigger watering
    int moistureLevel;          // Current measured moisture level
    PumpControl control;        // Control flags for pump state and manual override
    int manualButtonState;      // State of the manual override button
    int lastButtonState;        // Last recorded state of the manual button for debounce handling
    int thresholdHistory[10];   // Array to store last 10 thresholds for review
    int thresholdCount;         // Current count of stored thresholds
    int ledPin;                 // LED pin associated with each land type
} Land;

// Instances for soil types - (struct Land)
Land land1 = {0, 0, {0, 0}, 0, 0, {0}, 0, ledPinLand1};
Land land2 = {0, 0, {0, 0}, 0, 0, {0}, 0, ledPinLand2};
Land land3 = {0, 0, {0, 0}, 0, 0, {0}, 0, ledPinLand3};

// Function prototypes
void manualMode(Land* land);
void autoMode(Land* land);
void viewDirectory(Land* land);

void setup() {
    // Initialize serial communication and configure pin modes
    Serial.begin(9600);
    pinMode(moisturePin, INPUT);
    pinMode(pumpPin, OUTPUT);
    pinMode(buttonPin, INPUT_PULLUP);
    pinMode(ledPinLand1, OUTPUT);
    pinMode(ledPinLand2, OUTPUT);
    pinMode(ledPinLand3, OUTPUT);

    // Ensure all LEDs and the pump are off initially
    digitalWrite(pumpPin, LOW);
    digitalWrite(ledPinLand1, LOW);
    digitalWrite(ledPinLand2, LOW);
    digitalWrite(ledPinLand3, LOW);
}

void loop() {
    int choice, landChoice;

    // Display land type selection menu
    Serial.println("\n=======================");
    Serial.println("   SELECT LAND TYPE    ");
    Serial.println("=======================");
    Serial.println("1: Land 1");
    Serial.println("2: Land 2");
    Serial.println("3: Land 3");
    Serial.print("Enter your choice: ");
    
    while (Serial.available() == 0) {}  // Wait for user input
    landChoice = Serial.parseInt();
    Serial.read(); // Clear the input buffer

    Land* currentLand;
    switch (landChoice) {
        case 1:
            currentLand = &land1;
            Serial.println("You selected Land 1.");
            break;
        case 2:
            currentLand = &land2;
            Serial.println("You selected Land 2.");
            break;
        case 3:
            currentLand = &land3;
            Serial.println("You selected Land 3.");
            break;
        default:
            Serial.println("Invalid land choice. Returning to menu...");
            return; // Invalid input, return to the menu
    }

    // Display mode selection menu for the chosen land
    Serial.println("\n=======================");
    Serial.println("     MODE SELECTION    ");
    Serial.println("=======================");
    Serial.println("1: Manual Mode");
    Serial.println("2: Auto Mode");
    Serial.println("3: View Threshold Directory");
    Serial.println("r: Return to menu");
    Serial.print("Enter your choice: ");
    
    while (Serial.available() == 0) {}  // Wait for user input
    choice = Serial.parseInt();
    Serial.read(); // Clear the input buffer

    // Turn off all land LEDs before switching modes
    digitalWrite(ledPinLand1, LOW);
    digitalWrite(ledPinLand2, LOW);
    digitalWrite(ledPinLand3, LOW);

    // Handle user choice for the mode
    switch (choice) {
        case 1:
            manualMode(currentLand); // Enter Manual Mode
            break;
        case 2:
            autoMode(currentLand);    // Enter Auto Mode
            break;
        case 3:
            viewDirectory(currentLand); // View saved moisture thresholds
            break;
        default:
            Serial.println("Invalid choice. Please try again.");
            break;
    }
}

// Function to enter Manual Mode
void manualMode(Land* land) {
    Serial.println("\n-----------------------");
    Serial.println("     MANUAL MODE       ");
    Serial.println("-----------------------");
    land->control.bits.manualOverride = 0; // Ensure manual override starts off
    digitalWrite(land->ledPin, LOW); // Turn off the specific land's LED initially
    
    while (1) {
        land->manualButtonState = digitalRead(buttonPin); // Read current state of manual button

        // Check for button press to toggle manual override
        if (land->manualButtonState == LOW && land->lastButtonState == HIGH) {
            // Toggle manual override and pump state
            land->control.bits.manualOverride = !land->control.bits.manualOverride;
            land->control.bits.pumpState = land->control.bits.manualOverride;

            // Control the pump based on manual override status
            digitalWrite(pumpPin, land->control.bits.pumpState ? HIGH : LOW);
            Serial.println(land->control.bits.pumpState ? "Pump is ON" : "Pump is OFF");

            // Update LED state to indicate manual mode activation
            digitalWrite(land->ledPin, land->control.bits.manualOverride ? HIGH : LOW);
            delay(200);  // Debounce delay
        }

        land->lastButtonState = land->manualButtonState; // Store the last button state for future checks

        // Break loop if user chooses to return to the main menu
        if (Serial.available() > 0) {
            char input = Serial.read();
            if (input == 'r') {
                Serial.println("-------------------------");
                Serial.println("<<< RETURNING TO MENU >>>");
                Serial.println("-------------------------");
                break;
            }
        }
        delay(50); // Additional debounce delay
    }
    land->control.bits.manualOverride = 0; // Clear manual override on exit
    digitalWrite(land->ledPin, LOW); // Turn off the specific land's LED on exit
}

// Function to enter Auto Mode
void autoMode(Land* land) {
    Serial.println("\n-----------------------");
    Serial.println("       AUTO MODE       ");
    Serial.println("-----------------------");
    
    Serial.print("Enter moisture threshold (0 to 876): ");
    while (Serial.available() == 0) {}  // Wait for user input
    land->moistureThreshold = Serial.parseInt();
    Serial.read(); // Clear the input buffer

    // Store the entered threshold in history
    if (land->thresholdCount < 10) {
        land->thresholdHistory[land->thresholdCount++] = land->moistureThreshold; // Add to history if space available
    } else {
        // Shift history if full, removing the oldest entry
        for (int i = 1; i < 10; i++) {
            land->thresholdHistory[i - 1] = land->thresholdHistory[i];
        }
        land->thresholdHistory[9] = land->moistureThreshold; // Add new threshold as the latest
    }

    Serial.println("\n=========================");
    Serial.print("New Threshold Set: ");
    Serial.println(land->moistureThreshold);
    Serial.println("=========================\n");

    while (1) {
        land->manualButtonState = digitalRead(buttonPin); // Check the manual button status
        if (land->manualButtonState == LOW && land->lastButtonState == HIGH) {
            Serial.println("Switching to Manual Mode due to button press...");
            manualMode(land); // Switch to manual mode if button is pressed
            break;
        }
        
        // Check for return to menu
        if (Serial.available() > 0) {
            char input = Serial.read();
            if (input == 'r') {
                Serial.println("-------------------------");
                Serial.println("<<< RETURNING TO MENU >>>");
                Serial.println("-------------------------");
                break;
            }
        }

        land->lastButtonState = land->manualButtonState; // Update last button state for debounce handling

        // Read the moisture level from the sensor
        land->moistureLevel = analogRead(moisturePin); 
        Serial.print("Current Moisture Level: ");
        Serial.println(land->moistureLevel);

        // Compare moisture level against the threshold
        if (land->moistureLevel < land->moistureThreshold) {
            // If moisture is below threshold, activate the pump
            digitalWrite(pumpPin, HIGH);
            Serial.println("Pump is ON: Watering plants...");
            land->control.bits.pumpState = 1; // Update pump state flag
        } else {
            // If moisture is adequate, deactivate the pump
            digitalWrite(pumpPin, LOW);
            Serial.println("Pump is OFF: Moisture level adequate.");
            land->control.bits.pumpState = 0; // Update pump state flag
        }
      Serial.println("---------------------------------------------");
        
        delay(1000); // Delay for sensor reading stabilization
    }
}

// Function to view saved moisture thresholds for a land type
void viewDirectory(Land* land) {
    Serial.println("\n=========================");
    Serial.println("   THRESHOLD DIRECTORY   ");
    Serial.println("=========================");
    
    if (land->thresholdCount == 0) {
        Serial.println("No thresholds recorded yet.");
    } else {
        for (int i = 0; i < land->thresholdCount; i++) {
            Serial.print("Threshold ");
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.println(land->thresholdHistory[i]);
        }
    }
    Serial.println("=========================\n");
}
