#ifndef ULTRASONIC_SENSOR_H
#define ULTRASONIC_SENSOR_H

#include "config.h"

class UltrasonicSensor {
private:
    const int triggerPin;
    const int echoPin;
    const float containerHeight;
    const float threshold;

    float getDistance() {
        // Using the proven logic from the original code
        digitalWrite(triggerPin, LOW);
        delayMicroseconds(2);
        
        digitalWrite(triggerPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(triggerPin, LOW);
        
        // Get the pulse duration and convert directly to distance
        long duration = pulseIn(echoPin, HIGH);
        float distance = duration / 58.0; // Original conversion factor
        
        // Convert from cm to meters for consistency
        return distance / 100.0;
    }

public:
    UltrasonicSensor() 
        : triggerPin(PinConfig::ULTRASONIC_TRIGGER),
          echoPin(PinConfig::ULTRASONIC_ECHO),
          containerHeight(0.15), // 15cm from sensor to bottom
          threshold(IceParameters::WATER_LEVEL_THRESHOLD) {
        pinMode(triggerPin, OUTPUT);
        pinMode(echoPin, INPUT);
    }

    struct WaterLevelData {
        float level;
        bool isValid;
    };

    WaterLevelData getCurrentLevel() {
        WaterLevelData data;
        float distance = getDistance();
        
        if (distance < 0 || distance > containerHeight) {
            data.level = -1;
            data.isValid = false;
        } else {
            data.level = containerHeight - distance;
            data.isValid = true;
        }
        
        return data;
    }

    bool isMeltComplete() {
        WaterLevelData data = getCurrentLevel();
        if (!data.isValid) return false;
        
        float expectedWaterLevel = IceParameters::ICE_THICKNESS * 
                                 IceParameters::WATER_ICE_DENSITY_RATIO;
        return data.level >= (expectedWaterLevel * threshold);
    }

    
};

#endif