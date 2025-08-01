#include <FastLED.h>

#define NUM_LEDS 47
#define LED_DATA_PIN 3
#define LED_BRIGHTNESS 20
#define PWM_INPUT_PIN 12

CRGB leds[NUM_LEDS];

void setup() {
  // Serial.begin(115200);
  pinMode(PWM_INPUT_PIN, INPUT);
  FastLED.addLeds<WS2812B, LED_DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(255 * LED_BRIGHTNESS / 100);
}

void loop() {
  // Clear all LEDs initially
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();

  // Measure pulse width (HIGH duration) in microseconds with a timeout of 25ms
  unsigned long highTime = pulseIn(PWM_INPUT_PIN, HIGH, 25000);

  // ESP32 outputs PWM at 5kHz (period = 1/freq = 200 microseconds)
  int pwmNumLEDs = map(highTime, 0, 200, 0, 255);
  pwmNumLEDs = constrain(pwmNumLEDs, 0, 255);

  int numLEDs = map(pwmNumLEDs, 0, 255, 1, NUM_LEDS - 1);
  fill_solid(leds, numLEDs, CRGB::Green);
  FastLED.show();

  delay(50);
}
