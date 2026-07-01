// 10 Hz blink on Arduino Nano D13 (built-in LED).
// 10 Hz means a 100 ms period: 50 ms on, 50 ms off.

constexpr uint8_t LED_PIN = 13;
constexpr unsigned long HALF_PERIOD_MS = 50;

void setup() {
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  delay(HALF_PERIOD_MS);
  digitalWrite(LED_PIN, LOW);
  delay(HALF_PERIOD_MS);
}
