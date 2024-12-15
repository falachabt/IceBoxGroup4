// config.h
#ifndef CONFIG_H
#define CONFIG_H

// Pin Configuration
struct PinConfig {
  // Temperature Sensors
  static const int AMBIENT_TEMP_SENSOR = 13;      // Ambient air temperature
  static const int ICE_SURFACE_TEMP_SENSOR = 12;  // Ice surface temperature
  static const int ICE_INNER_TEMP_SENSOR = 11;    // Ice core temperature
  static const int BASE_TEMP_SENSOR = 10;         // Base plate temperature

  // Ultrasonic Sensor
  static const int ULTRASONIC_TRIGGER = 4;  // HC-SR04 trigger pin
  static const int ULTRASONIC_ECHO = 5;     // HC-SR04 echo pin

  // Alert System
  static const int BUZZER_PIN = 8;    // Buzzer for alerts
  static const int LED_PIN = 7;       // Status LED
  static const int RESET_BUTTON = 2;  // Reset button with pullup
};

// Network Configuration
struct NetworkConfig {
  static constexpr const char* WIFI_SSID = "Xiaomi 12T";
  static constexpr const char* WIFI_PASSWORD = "falachabt";
  static constexpr const char* SERVER_URL = "op-dev.icam.fr";
  static constexpr const char* AUTHOR = "LIL G4";
  static constexpr const char* SECRET_KEY = "fa059";
  static constexpr const char* THINGSPEAK_API_KEY = "CK43QHFPT8QQTSI8";
};

// Ice Box Parameters
struct IceParameters {
  static constexpr float ICE_MASS = 0.5;                   // kg
  static constexpr float ICE_SURFACE_AREA = 0.06;          // m² (10cm cube)
  static constexpr float ICE_THICKNESS = 0.1;              // m
  static constexpr float WATER_LEVEL_THRESHOLD = 0.95;     // 95% of expected level
  static constexpr float ICE_VOLUME = 0.001;               // m³ (10cm cube)
  static constexpr float WATER_ICE_DENSITY_RATIO = 0.917;  // water/ice density
};

// Physical Constants
struct PhysicalConstants {
  static constexpr float SPECIFIC_HEAT_ICE = 2108;     // J/(kg·°C)
  static constexpr float LATENT_HEAT_FUSION = 334000;  // J/kg
  static constexpr float CONVECTION_COEFF = 25;        // W/(m²·°C)
  static constexpr float CONDUCTION_COEFF = 2.22;      // W/(m·°C)
  static constexpr float SPEED_OF_SOUND = 343.0;       // m/s at 20°C
};

// Timing Constants
struct TimingConfig {
  static constexpr unsigned long MEASUREMENT_INTERVAL = 500;  // ms
  static constexpr unsigned long UPDATE_INTERVAL = 300000;    // 5 minutes
  static constexpr unsigned long BUZZER_INTERVAL = 1000;      // 1 second
  static constexpr unsigned long ULTRASONIC_TIMEOUT = 25000;  // 25ms timeout
};

#endif