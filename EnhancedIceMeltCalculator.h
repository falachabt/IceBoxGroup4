#ifndef ENHANCED_ICE_MELT_CALCULATOR_H
#define ENHANCED_ICE_MELT_CALCULATOR_H

#include "config.h"

class EnhancedIceMeltCalculator {
private:
    float iceMass;
    float totalEnergy;
    float currentPower;
    float accumulatedEnergy;

public:
    EnhancedIceMeltCalculator(float mass, float initialTemp) 
        : iceMass(mass), accumulatedEnergy(0) {
        calculateInitialEnergy(initialTemp);
    }

    void calculateInitialEnergy(float initialTemp) {
        // Energy to heat ice to 0°C
        float energyToZero = iceMass * PhysicalConstants::SPECIFIC_HEAT_ICE * 
                            (0 - initialTemp);
        // Energy for phase change
        float energyForMelting = iceMass * PhysicalConstants::LATENT_HEAT_FUSION;
        totalEnergy = energyToZero + energyForMelting;
    }

    float calculateHeatFlux(float ambientTemp, float surfaceTemp, float baseTemp) {
        // Convection from all faces except bottom
        float convectionPower = PhysicalConstants::CONVECTION_COEFF * 
                              IceParameters::ICE_SURFACE_AREA * 5/6 * 
                              (ambientTemp - surfaceTemp);
        
        // Conduction from bottom
        float conductionPower = PhysicalConstants::CONDUCTION_COEFF * 
                              IceParameters::ICE_SURFACE_AREA * (1/6) * 
                              (baseTemp - surfaceTemp) / 
                              IceParameters::ICE_THICKNESS;
        
        currentPower = convectionPower + conductionPower;
        return currentPower;
    }

    float getRemainingTime() {
        if (currentPower <= 0) return -1;
        return totalEnergy / currentPower; // seconds
    }

    void updateEnergy(float timeGap) {
        float energyTransferred = currentPower * timeGap;
        totalEnergy -= energyTransferred;
        accumulatedEnergy += energyTransferred;
        
        if (totalEnergy < 0) totalEnergy = 0;
    }

    float getMeltProgress() {
        return (accumulatedEnergy / (iceMass * PhysicalConstants::LATENT_HEAT_FUSION)) * 100;
    }

    bool isTheoreticallyMelted() {
        return totalEnergy <= 0;
    }
};

#endif