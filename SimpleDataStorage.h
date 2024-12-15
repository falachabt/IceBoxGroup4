#ifndef SIMPLE_DATA_STORAGE_H
#define SIMPLE_DATA_STORAGE_H

#include "config.h"

struct MeasurementPoint {
    unsigned long timestamp;
    float ambientTemp;
    float surfaceTemp;
    float innerTemp;
    float baseTemp;
    float waterLevel;
    float meltProgress;
    float remainingTime;
    bool isValid;
};

class SimpleDataStorage {
private:
    static const int BUFFER_SIZE = 100;  // Store last 100 measurements
    MeasurementPoint dataBuffer[BUFFER_SIZE];
    int currentIndex;
    int storedCount;
    
public:
    SimpleDataStorage() : currentIndex(0), storedCount(0) {}
    
    void storeMeasurement(const MeasurementPoint& point) {
        dataBuffer[currentIndex] = point;
        currentIndex = (currentIndex + 1) % BUFFER_SIZE;
        if (storedCount < BUFFER_SIZE) storedCount++;
    }
    
    MeasurementPoint getMeasurement(int index) {
        if (index >= storedCount) {
            return MeasurementPoint{0, 0, 0, 0, 0, 0, 0, 0, false};
        }
        
        // Calculate actual index considering circular buffer
        int actualIndex = (currentIndex - storedCount + index + BUFFER_SIZE) % BUFFER_SIZE;
        return dataBuffer[actualIndex];
    }
    
    int getStoredCount() {
        return storedCount;
    }
    
    // Get the last N measurements
    void getLastNMeasurements(int n, MeasurementPoint* result, int& count) {
        count = min(n, storedCount);
        for (int i = 0; i < count; i++) {
            result[i] = getMeasurement(storedCount - count + i);
        }
    }
    
    // Print all stored data via Serial
    void printAllData() {
        Serial.println("\n=== Stored Measurements ===");
        for (int i = 0; i < storedCount; i++) {
            MeasurementPoint point = getMeasurement(i);
            Serial.print("Time: ");
            Serial.print(point.timestamp / 1000);  // Convert to seconds
            Serial.print("s, Temps(°C): ");
            Serial.print(point.ambientTemp);
            Serial.print("/");
            Serial.print(point.surfaceTemp);
            Serial.print("/");
            Serial.print(point.innerTemp);
            Serial.print("/");
            Serial.print(point.baseTemp);
            Serial.print(", Water: ");
            Serial.print(point.waterLevel * 1000);
            Serial.print("mm, Progress: ");
            Serial.print(point.meltProgress);
            Serial.print("%, Remaining: ");
            Serial.print(point.remainingTime / 60.0);
            Serial.println(" min");
        }
        Serial.println("=== End of Data ===\n");
    }
    
    // Calculate average temperatures over last N measurements
    void getAverageTemperatures(int n, float& avgAmbient, float& avgSurface, 
                               float& avgInner, float& avgBase) {
        int count = min(n, storedCount);
        if (count == 0) {
            avgAmbient = avgSurface = avgInner = avgBase = 0;
            return;
        }
        
        float sumAmbient = 0, sumSurface = 0, sumInner = 0, sumBase = 0;
        for (int i = 0; i < count; i++) {
            MeasurementPoint point = getMeasurement(storedCount - count + i);
            sumAmbient += point.ambientTemp;
            sumSurface += point.surfaceTemp;
            sumInner += point.innerTemp;
            sumBase += point.baseTemp;
        }
        
        avgAmbient = sumAmbient / count;
        avgSurface = sumSurface / count;
        avgInner = sumInner / count;
        avgBase = sumBase / count;
    }
};

#endif