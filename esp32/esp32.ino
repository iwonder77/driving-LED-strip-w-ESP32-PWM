#define POT_PIN 26
#define PWM_OUT_PIN 27

#define PWM_FREQ 5000     // 5kHz PWM frequency
#define PWM_RESOLUTION 8  // 8-bit resolution (0-255)

void setup() {
  Serial.begin(115200);

  pinMode(POT_PIN, INPUT);

  bool success = ledcAttach(PWM_OUT_PIN, PWM_FREQ, PWM_RESOLUTION);
  if (!success) {
    Serial.println("ERROR: failed to attach PWM to pin");
    while (1);
  }

  Serial.println("ESP32 ready, reading pot value...");
}

void loop() {
  int potVal = analogRead(POT_PIN);           // read from 0-4095 (12-bit ADC)
  int pwmVal = map(potVal, 0, 4095, 0, 255);  // scale to 0-255 for 8-bit PWM

  ledcWrite(PWM_OUT_PIN, pwmVal);

  Serial.print("pot value: ");
  Serial.print(potVal);
  Serial.print(" ---> PWM Output: ");
  Serial.print(pwmVal);
  Serial.println();

  delay(100);
}
