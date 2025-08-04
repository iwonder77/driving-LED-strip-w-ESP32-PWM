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

#define NUM_LEDS 47
#define LED_DATA_PIN 3
#define LED_BRIGHTNESS 20
#define PWM_INPUT_PIN 12
#define PWM_MEASUREMENT_NUM 3

CRGB leds[NUM_LEDS];

const int AVERAGE_SAMPLES = 5;  // Smaller circular buffer for PWM measurements
RunningAverage pwmAverage(AVERAGE_SAMPLES);

const int LED_HYSTERESIS_THRESHOLD = 2;
int lastNumLEDs = 0;    // variable to check past num of LEDs lit up

void setup() {
  pinMode(PWM_INPUT_PIN, INPUT);
  
  // FastLED setup
  FastLED.addLeds<WS2812B, LED_DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(255 * LED_BRIGHTNESS / 100);
  
  pwmAverage.clear();
}

void loop() {
  // Take multiple PWM measurements for stability
  unsigned long highTime = pulseIn(PWM_INPUT_PIN, HIGH, 25000);
 
  // Only add valid readings to the running average
  if(highTime > 0 && highTime <= 250) {
    pwmAverage.addValue(highTime);
    
    float avgHighTime = pwmAverage.getAverage();
    
    // Convert pulse width to PWM value (0-255)
    int pwmValue = map(avgHighTime, 0, 200, 0, 255);
    pwmValue = constrain(pwmValue, 0, 255);
    
    // Convert to number of LEDs
    int numLEDs = map(pwmValue, 0, 255, 0, NUM_LEDS);
    numLEDs = constrain(numLEDs, 0, NUM_LEDS);
    
    // Only update LEDs if change is significant (reduces flickering)
    if(abs(numLEDs - lastNumLEDs) >= LED_HYSTERESIS_THRESHOLD) {
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      fill_solid(leds, numLEDs, CRGB::Green);
      FastLED.show();
      lastNumLEDs = numLEDs;
    }
  }
  
  delay(20); // Faster updates for smoother response
}
