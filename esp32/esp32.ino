/* 
* ----------------------------------------------
* PROJECT NAME: driving-LED-strip-w-ESP32
* Description: code for the ESP32 that reads a potentiometer value and sends a PWM signal to the QuinLED
*   Dig Uno LED driver, who then lights a bar of LEDs on a strip corresponding to the strength of that signal
* 
* Author: Isai Sanchez
* Date: 8-4-25
* Board Used: ESP32-DevkitC-V4
* Libraries:
*   - RunningAverage.h: https://github.com/RobTillaart/RunningAverage
*       -- circular buffer library used to smoothen out sample readings
* Notes:
*   - the running average was applied to the raw ADC pot value because:
*       -- higher resolution: the ADC gives us a 12-bit resolution (0-4095) so we have more data points
*                             to work with
*       -- preserver precision: mapping the raw value first would reduce the resolution to 8-bit and therefore
*                               lose information that would benefit the smoothing
*       -- better noise rejection: best to filter at the source of the ADC noise
* ----------------------------------------------
*/

#include <RunningAverage.h>

#define POT_PIN 26
#define PWM_OUT_PIN 25

#define PWM_FREQ 5000     // 5kHz PWM frequency
#define PWM_RESOLUTION 8  // 8-bit resolution (0-255)

const int SAMPLE_WINDOW_SIZE = 8;    // number of samples to average for smoothing raw readings
const int HYSTERESIS_THRESHOLD = 5;  // the minimum change in reading that must occur to update PWM
RunningAverage potFilter(SAMPLE_WINDOW_SIZE);
int lastPwmValue = 0;

void setup() {
  Serial.begin(115200);
  pinMode(POT_PIN, INPUT);

  bool success = ledcAttach(PWM_OUT_PIN, PWM_FREQ, PWM_RESOLUTION);
  if (!success) {
    Serial.println("ERROR: failed to attach PWM to pin");
    while (1);
  }

  // initialize filter with initial readings
  potFilter.clear();
  for (int i = 0; i < SAMPLE_WINDOW_SIZE; i++) {
    int initVal = analogRead(POT_PIN);
    potFilter.addValue(initVal);
    delay(5);
  }

  Serial.println("ESP32 ready, reading pot value...");
}

void loop() {
  // read from 0-4095 (12-bit ADC)
  int rawPotVal = analogRead(POT_PIN);

  // add raw pot reading to the running average object
  potFilter.add(rawPotVal);

  // get the smoothed average value
  float avgPotVal = potFilter.getAverage();

  // map the smoothed value to PWM range
  int pwmVal = map(avgPotVal, 0, 4095, 0, 255);  // scale to 0-255 for 8-bit PWM


  // only update PWM if change is significant (hysteresis)
  if (abs(pwmVal - lastPwmValue) >= HYSTERESIS_THRESHOLD) {
    ledcWrite(PWM_OUT_PIN, pwmVal);
    lastPwmValue = pwmVal;
    Serial.print("Raw pot: ");
    Serial.print(rawPotVal);
    Serial.print(" | Avg pot: ");
    Serial.print(avgPotVal, 1);  // Show 1 decimal place
    Serial.print(" | PWM Output: ");
    Serial.print(pwmVal);
    Serial.println();
  }


  delay(25);
}
