#ifndef ALERT_SYSTEM_H
#define ALERT_SYSTEM_H

#include "config.h"

class AlertSystem {
private:
    const int buzzerPin;
    const int ledPin;
    const int resetButtonPin;
    bool isActive;
    unsigned long lastBuzzerToggle;
    bool buzzerState;
    bool ledBlinkState;

public:
    AlertSystem() 
        : buzzerPin(PinConfig::BUZZER_PIN),
          ledPin(PinConfig::LED_PIN),
          resetButtonPin(PinConfig::RESET_BUTTON),
          isActive(false),
          lastBuzzerToggle(0),
          buzzerState(false),
          ledBlinkState(false) {
        pinMode(buzzerPin, OUTPUT);
        pinMode(ledPin, OUTPUT);
        pinMode(resetButtonPin, INPUT_PULLUP);
    }

    void activate() {
        isActive = true;
        digitalWrite(ledPin, HIGH);
    }

    void update() {
        if (!isActive) return;

        if (digitalRead(resetButtonPin) == LOW) {
            deactivate();
            return;
        }

        unsigned long currentTime = millis();
        if (currentTime - lastBuzzerToggle >= TimingConfig::BUZZER_INTERVAL) {
            buzzerState = !buzzerState;
            digitalWrite(buzzerPin, buzzerState);
            
            // Blink LED when buzzing
            ledBlinkState = !ledBlinkState;
            digitalWrite(ledPin, ledBlinkState);
            
            lastBuzzerToggle = currentTime;
        }
    }

    void deactivate() {
        isActive = false;
        digitalWrite(buzzerPin, LOW);
        digitalWrite(ledPin, LOW);
        buzzerState = false;
        ledBlinkState = false;
    }

    bool isActiveState() const {
        return isActive;
    }
};

#endif