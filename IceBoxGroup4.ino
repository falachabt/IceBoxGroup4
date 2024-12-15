#include "MultiTempSensor.h"
#include "UltrasonicSensor.h"
#include "EnhancedIceMeltCalculator.h"
#include "AlertSystem.h"
#include "NetworkManager.h"
#include "SimpleDataStorage.h"

// Global objects
MultiTempSensor tempSensors;
UltrasonicSensor waterSensor;
AlertSystem alertSystem;
NetworkManager network(NetworkConfig::WIFI_SSID, 
                      NetworkConfig::WIFI_PASSWORD, 
                      NetworkConfig::SERVER_URL,
                      NetworkConfig::THINGSPEAK_API_KEY);
SimpleDataStorage dataStorage;

EnhancedIceMeltCalculator* iceMelt = nullptr;

// Timing variables
unsigned long lastMeasurement = 0;
unsigned long lastUpdate = 0;
bool systemInitialized = false;
bool meltingComplete = false;

void setup() {
    Serial.begin(9600);
    while (!Serial) {
        delay(100);  // Wait for Serial connection
    }
    
    Serial.println("Initializing Ice Box Monitoring System...");

    // Initialize Network Connection
    if (!network.connect()) {
        Serial.println("Warning: Failed to connect to WiFi! System will run without network connectivity.");
    } else {
        Serial.println("WiFi connected successfully.");
        // Send initial status message
        network.sendToTweeticam(NetworkConfig::AUTHOR, 
                              NetworkConfig::SECRET_KEY, 
                              "Ice Box Monitoring System Starting...");
    }

    // Initialize Sensors and wait for first readings
    Serial.println("Waiting for temperature sensors to stabilize...");
    delay(1000);  // Give sensors time to initialize

    auto temps = tempSensors.getAllTemperatures();
    auto waterLevel = waterSensor.getCurrentLevel();

    if (!temps.isValid) {
        Serial.println("Error: Temperature sensors initialization failed!");
        return;
    }

    if (!waterLevel.isValid) {
        Serial.println("Error: Water level sensor initialization failed!");
        return;
    }

    // Initialize Ice Melt Calculator with initial temperature
    iceMelt = new EnhancedIceMeltCalculator(IceParameters::ICE_MASS, temps.inner);

    // Print initial conditions
    Serial.println("\nInitial Conditions:");
    Serial.print("Ice Inner Temperature: ");
    Serial.print(temps.inner);
    Serial.println("°C");
    Serial.print("Water Level: ");
    Serial.print(waterLevel.level * 1000);
    Serial.println(" mm");

    // Initialize timing variables
    lastMeasurement = millis();
    lastUpdate = lastMeasurement;

    // Set system as initialized
    systemInitialized = true;
    Serial.println("\nSystem initialized successfully!");
    Serial.println("Type 'data' to view stored measurements.");
}

void loop() {
    if (!systemInitialized) {
        Serial.println("System not initialized. Retrying...");
        setup();
        delay(1000);
        return;
    }

    unsigned long currentTime = millis();

    // Regular measurements and calculations
    if (currentTime - lastMeasurement >= TimingConfig::MEASUREMENT_INTERVAL) {
        auto temps = tempSensors.getAllTemperatures();
        auto waterLevel = waterSensor.getCurrentLevel();

        if (temps.isValid && waterLevel.isValid) {
           
            
            // Calculate heat flux and update energy state
            float heatFlux = iceMelt->calculateHeatFlux(temps.ambient, 
                                                       temps.surface, 
                                                       temps.base);
            float timeGap = (currentTime - lastMeasurement) / 1000.0;
            iceMelt->updateEnergy(timeGap);
            
            // Get remaining time
            float remainingTime = iceMelt->getRemainingTime();
            
            // Store measurement
            MeasurementPoint point = {
                .timestamp = currentTime,
                .ambientTemp = temps.ambient,
                .surfaceTemp = temps.surface,
                .innerTemp = temps.inner,
                .baseTemp = temps.base,
                .waterLevel = waterLevel.level,
                .meltProgress = iceMelt->getMeltProgress(),
                .remainingTime = remainingTime,
                .isValid = true
            };
            dataStorage.storeMeasurement(point);
            
            // Print status with averages
            float avgAmbient, avgSurface, avgInner, avgBase;
            dataStorage.getAverageTemperatures(10, avgAmbient, avgSurface, 
                                             avgInner, avgBase);
            printStatus(temps, waterLevel.level, remainingTime, 
                       avgAmbient, avgSurface, avgInner, avgBase);

            // Check for melting completion
            if (!meltingComplete && iceMelt->isTheoreticallyMelted() && 
                waterSensor.isMeltComplete()) {
                meltingComplete = true;
                alertSystem.activate();
                sendMeltCompleteNotification(waterLevel.level);
                // Print all stored data at completion
                dataStorage.printAllData();
            }
        } else {
            Serial.println("Error reading sensors!");
        }

        lastMeasurement = currentTime;
    }

    // Update alert system
    alertSystem.update();

    // Periodic updates to server
    if (currentTime - lastUpdate >= TimingConfig::UPDATE_INTERVAL) {
        sendStatusUpdate();
        lastUpdate = currentTime;
    }

    // Handle commands from Serial
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        if (command == "data") {
            dataStorage.printAllData();
        }
    }
}

void printStatus(const MultiTempSensor::TemperatureReadings& temps, 
                float waterLevel, float remainingTime,
                float avgAmbient, float avgSurface, 
                float avgInner, float avgBase) {
    Serial.println("\n=== Status Update ===");
    Serial.println("Current Temperatures (°C):");
    Serial.print("  Ambient: ");
    Serial.print(temps.ambient);
    Serial.print(" (Avg: ");
    Serial.print(avgAmbient);
    Serial.println(")");
    Serial.print("  Surface: ");
    Serial.print(temps.surface);
    Serial.print(" (Avg: ");
    Serial.print(avgSurface);
    Serial.println(")");
    Serial.print("  Inner: ");
    Serial.print(temps.inner);
    Serial.print(" (Avg: ");
    Serial.print(avgInner);
    Serial.println(")");
    Serial.print("  Base: ");
    Serial.print(temps.base);
    Serial.print(" (Avg: ");
    Serial.print(avgBase);
    Serial.println(")");
    
    Serial.print("Water Level: ");
    Serial.print(waterLevel * 1000);
    Serial.println(" mm");
    
    Serial.print("Melt Progress: ");
    Serial.print(iceMelt->getMeltProgress());
    Serial.println("%");
    
    if (remainingTime > 0) {
        Serial.print("Estimated Remaining Time: ");
        Serial.print(remainingTime / 60.0);
        Serial.println(" minutes");
    }
    Serial.println("==================\n");
}

void sendStatusUpdate() {
    if (iceMelt == nullptr) return;
    
    // Get current readings
    auto temps = tempSensors.getAllTemperatures();
    auto waterLevel = waterSensor.getCurrentLevel();
    
    // Get averages for the message
    float avgAmbient, avgSurface, avgInner, avgBase;
    dataStorage.getAverageTemperatures(10, avgAmbient, avgSurface, avgInner, avgBase);
    
    // Calculate remaining time in minutes
    float remainingTime = iceMelt->getRemainingTime() / 60.0;

    // Send to ThingSpeak with all 8 fields
    network.sendToThingSpeak(
        temps.ambient,                    // field1: ambient temp
        temps.surface,                    // field2: surface temp
        temps.inner,                      // field3: inner temp
        temps.base,                       // field4: base temp
        waterLevel.level * 1000,          // field5: water level in mm
        iceMelt->getMeltProgress(),       // field6: melt progress percentage
        remainingTime,                    // field7: remaining time in minutes
        iceMelt->calculateHeatFlux(temps.ambient, temps.surface, temps.base)  // field8: heat flux
    );
    
    // Send to Tweeticam
    String message = String("Status: ") +
                    "Progress=" + String(iceMelt->getMeltProgress(), 1) + "%, " +
                    "Time=" + String(remainingTime, 1) + "min";
    
    network.sendToTweeticam(NetworkConfig::AUTHOR, 
                           NetworkConfig::SECRET_KEY, 
                           message);
}

void sendMeltCompleteNotification(float finalWaterLevel) {
    String message = String("Ice Melt Complete! ") +
                    "Final Water Level: " + 
                    String(finalWaterLevel * 1000, 1) + "mm";
    
    // Send final data to ThingSpeak
    auto temps = tempSensors.getAllTemperatures();
    network.sendToThingSpeak(
        temps.ambient,                    // field1: ambient temp
        temps.surface,                    // field2: surface temp
        temps.inner,                      // field3: inner temp
        temps.base,                       // field4: base temp
        finalWaterLevel * 1000,          // field5: water level in mm
        100.0,                           // field6: melt progress percentage (100% as complete)
        0.0,                             // field7: remaining time (0 as complete)
        iceMelt->calculateHeatFlux(temps.ambient, temps.surface, temps.base)  // field8: heat flux
    );
                    
    // Send completion message to Tweeticam
    network.sendToTweeticam(NetworkConfig::AUTHOR, 
                           NetworkConfig::SECRET_KEY, 
                           message);
}