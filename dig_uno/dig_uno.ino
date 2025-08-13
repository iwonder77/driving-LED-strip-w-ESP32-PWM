/* 
* ----------------------------------------------
* PROJECT NAME: driving-LED-strip-w-ESP32
* Description: code for the QuinLED Dig Uno that reads a pwm signal sent from a master ESP32 and uses it to 
*              light up an LED bar strip 
* 
* Author: Isai Sanchez
* Date: 8-4-25
* Board Used: ESP32-DevkitC-V4
* Libraries:
*   - FastLED.h: fastled.io
*   - RunningAverage.h: https://github.com/RobTillaart/RunningAverage
*       -- circular buffer library used to smoothen out sample readings
* Notes:
* ----------------------------------------------
*/

#include <FastLED.h>
#include <RunningAverage.h>

#define NUM_LEDS 59
#define LED_DATA_PIN 3
#define LED_BRIGHTNESS 20
#define PWM_INPUT_PIN 32

volatile unsigned long pulse_start_time = 0;
volatile unsigned long pulse_width = 0;
volatile unsigned long last_period_start = 0;
volatile unsigned long period_length = 0;
volatile bool new_data_available = false;

CRGB leds[NUM_LEDS];
const int NUM_ANIMTION_LEDs = 30;

const int AVERAGE_SAMPLES = 5;
RunningAverage dutyCycleFilter(AVERAGE_SAMPLES);

// simple idle animation variables
const unsigned long IDLE_TIMEOUT = 5000;  // Start idling after 5 seconds
unsigned long lastActivityTime = 0;
float wavePhase = 0;

// core functionality variables
const int LED_HYSTERESIS_THRESHOLD = 2;
int lastNumLEDs = 0;
int currentTargetLEDs = 0;  // The base LED count from potentiometer

void IRAM_ATTR pwmInterrupt() {
  unsigned long current_time = micros();
  int pinState = digitalRead(PWM_INPUT_PIN);

  switch (pinState) {
    // rising edge, start of a new pulse
    case HIGH:
      // calculate period (time between consecutive rising edges)
      if (last_period_start != 0) {
        period_length = current_time - last_period_start;
      }
      last_period_start = current_time;
      pulse_start_time = current_time;
      break;
    // falling edge, record pulse width time
    case LOW:
      pulse_width = current_time - pulse_start_time;
      new_data_available = true;  // signal main loop that data is ready
      break;
  };
}

void setup() {
  Serial.begin(115200);
  pinMode(PWM_INPUT_PIN, INPUT_PULLUP);
  FastLED.addLeds<WS2815, LED_DATA_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(255 * LED_BRIGHTNESS / 100);

  dutyCycleFilter.clear();
  lastActivityTime = millis();

  attachInterrupt(digitalPinToInterrupt(PWM_INPUT_PIN), pwmInterrupt, CHANGE);

  currentTargetLEDs = 10;
}

void loop() {
  // make sure high time reading is valid
  if (new_data_available) {
    noInterrupts();
    unsigned long safe_pulse_width = pulse_width;
    unsigned long safe_period = period_length;
    new_data_available = false;
    interrupts();
    if (safe_period >= 100) {
      float duty_cycle = (float)safe_pulse_width / safe_period * 100;
      duty_cycle = constrain(duty_cycle, 0, 100);
      dutyCycleFilter.addValue(duty_cycle);
      float smoothedDutyCycle = dutyCycleFilter.getAverage();
      int targetLEDs = map(smoothedDutyCycle, 0, 100, 0, NUM_LEDS - 1);
      targetLEDs = constrain(targetLEDs, 0, NUM_LEDS - 1);

      // Check if potentiometer changed significantly
      if (abs(targetLEDs - currentTargetLEDs) >= LED_HYSTERESIS_THRESHOLD) {
        currentTargetLEDs = targetLEDs;
        lastActivityTime = millis();  // Reset idle timer
      }
    }
  }
  // Always update display (either normal or with idle animation)
  updateDisplay();
  delay(10);  // Smooth animation timing
}

void updateDisplay() {
  unsigned long currentTime = millis();
  bool isIdling = (currentTime - lastActivityTime) > IDLE_TIMEOUT;

  int displayLEDs = currentTargetLEDs;

  if (isIdling && currentTargetLEDs > 0) {
    // Create breathing/wave effect during idle
    wavePhase += 0.05;  // Speed of breathing animation
    if (wavePhase > TWO_PI) wavePhase = 0;

    // Create wave that oscillates ±25% around the target
    float waveMultiplier = 1.0 + (sin(wavePhase) * 0.25);  // 0.75 to 1.25
    displayLEDs = (int)(NUM_ANIMTION_LEDs * waveMultiplier);
    displayLEDs = constrain(displayLEDs, 1, NUM_LEDS);

    // Add subtle randomness for more organic feel
    if (random(100) < 10) {          // 10% chance each frame
      displayLEDs += random(-1, 2);  // ±1 LED jitter
      displayLEDs = constrain(displayLEDs, 1, NUM_LEDS);
    }
  }

  // Clear all LEDs
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  // Choose color based on state
  CRGB ledColor = isIdling ? CRGB(255, 80, 0) : CRGB::Green;  // Red for idle, Green for active

  // Create the wave effect you want
  for (int i = 0; i < displayLEDs && i < NUM_LEDS; i++) {
    if (isIdling) {
      // During idle: create a wave intensity that fades toward the edges
      float distanceFromCenter = abs(i - (displayLEDs / 2.0));
      float maxDistance = displayLEDs / 2.0;
      float intensity = 1.0 - (distanceFromCenter / maxDistance * 0.5);  // 50% fade to edges

      // Add wave motion along the strip
      float waveIntensity = sin(wavePhase + (i * 0.3)) * 0.3 + 0.7;  // 0.4 to 1.0
      intensity *= waveIntensity;

      leds[i] = CRGB(
        (int)(ledColor.r * intensity),
        (int)(ledColor.g * intensity),
        (int)(ledColor.b * intensity));
    } else {
      // Normal operation: solid color
      leds[i] = ledColor;
    }
  }

  FastLED.show();
}
