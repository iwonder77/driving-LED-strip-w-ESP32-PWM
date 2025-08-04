# Driving an LED strip with ESP32 PWM

A stable, noise-free LED bar controller using two ESP32s-based boards. The first board, an ESP32 DevKit-C v4, reads a potentiometer and transmits PWM signals to a [QuinLED Dig Uno](https://quinled.info/pre-assembled-quinled-dig-uno/) LED Driver, which controls a WS2815 12V LED strip.

This project is a building block for more complex implementations, where we need to create a smooth, stable, analog-to-LED bar display by combining basic hardware filtering and software smoothing techniques to eliminate noise and jitter.

### Hardware

-   Main board: ESP32-DevKitC v4 (potentiometer reader & PWM transmitter)
-   LED Driver: [QuinLED Dig Uno](https://quinled.info/pre-assembled-quinled-dig-uno/) 
-   10kΩ 3V3 Potentiometer
-   WS2815 12V LED strip (w/ 47 individual LEDs)
-   50nF ceramic capacitor (noise filtering)

### Hardware Setup

Connections

- Potentiometer: 3.3V → Pot → Ground, Wiper → ESP32 Pin 26
- Hardware Filter: 50nF ceramic capacitor between Pin 26 and Ground
- PWM Output: ESP32 Pin 27 → QuinLED Pin 12
- LED Strip: QuinLED Pin 3 → WS2812B Data Line

## ESP32-DevKitC Controller Code 

### Notable snippets:

1. Running Average Filter

Using the [Running Average](https://github.com/RobTillaart/RunningAverage) arduino library, we are able to leverage a circular buffer to smooth out raw ADC readings and eliminate noise. 

```cpp
#include <RunningAverage.h>

const int SAMPLE_WINDOW_SIZE = 8;   // Balance of smoothing vs responsiveness
RunningAverage potFilter(SAMPLE_WINDOW_SIZE);

// In setup():
potFilter.clear();
for(int i = 0; i < SAMPLE_WINDOW_SIZE; i++) {
    potFilter.addValue(analogRead(POT_PIN));
    delay(5);
}
```

2. Hysteresis Threshold

Only updates PWM output when changes to present output exceed a minimum threshold, which helps prevent jitter

```cpp
const int HYSTERESIS_THRESHOLD = 2; // Minimum change to update PWM
int lastPwmVal = 0;

// Only update PWM if change is significant
if (abs(pwmVal - lastPwmVal) >= HYSTERESIS_THRESHOLD) {
    ledcWrite(PWM_OUT_PIN, pwmVal);
    lastPwmVal = pwmVal;
}
```


