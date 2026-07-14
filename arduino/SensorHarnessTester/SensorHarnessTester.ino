// FC2 Sensor Harness Tester
//
// Reads the harness test voltages with the ATmega328P internal 1.1 V ADC
// reference, reports raw ADC counts and calculated pre-divider input voltage,
// compares each channel against its expected standard, and drives pass/fail LEDs.
//
// LED indication:
//   ERROR/FAIL: D2 HIGH, D3 LOW
//   PASS:       D2 LOW,  D3 HIGH

struct Channel {
  const char *name;
  uint8_t pin;
  float resistorAOhms;
  float resistorBOhms;
  float standardVolts;
  float calibrationFactor;
  float toleranceVolts;
};

constexpr uint8_t ERROR_LED_PIN = 2;
constexpr uint8_t PASS_LED_PIN = 3;

constexpr float ADC_REFERENCE_VOLTS = 1.1;
constexpr float ADC_MAX_COUNT = 1023.0;
constexpr unsigned long LOOP_PERIOD_MS = 100;  // 10 readings/second
constexpr uint8_t TRAILING_CLEAR_SPACES = 20;

// Calibration factors multiply the resistor-divider-based voltage calculation.
// D_0_10, D_12V, and S_12V factors were calculated from live readings with a
// 12.00 VDC stimulus/pass DUT connected. D_GND and S_CTRLIN are intentionally 1.00.
const Channel channels[] = {
  // name       pin  A ohms    B ohms   standard  cal factor  tolerance
  {"D_0_10",   A3, 150000.0, 10000.0, 10.00,     0.99701,     0.5},
  {"D_GND",    A4,   1000.0,  1000.0,  0.00,     1.00000,     0.5},
  {"D_12V",    A5,  15000.0,  1000.0, 12.00,     0.97967,     0.5},
  {"S_12V",    A6,  15000.0,  1000.0, 12.00,     0.97823,     0.3},
  {"S_CTRLIN", A7,   1000.0,  2200.0,  1.13,     1.00000,     0.5},
};

constexpr size_t CHANNEL_COUNT = sizeof(channels) / sizeof(channels[0]);

uint16_t median3(uint16_t a, uint16_t b, uint16_t c) {
  if (a > b) {
    uint16_t t = a;
    a = b;
    b = t;
  }
  if (b > c) {
    uint16_t t = b;
    b = c;
    c = t;
  }
  if (a > b) {
    uint16_t t = a;
    a = b;
    b = t;
  }
  return b;
}

uint16_t readAdcMedian3(uint8_t pin) {
  // Discard one sample after selecting the mux channel, then take median of 3.
  analogRead(pin);
  const uint16_t a = analogRead(pin);
  const uint16_t b = analogRead(pin);
  const uint16_t c = analogRead(pin);
  return median3(a, b, c);
}

float dividerRatio(const Channel &channel) {
  return channel.resistorBOhms / (channel.resistorAOhms + channel.resistorBOhms);
}

float inputVoltsFromCounts(uint16_t counts, const Channel &channel) {
  const float adcPinVolts = (static_cast<float>(counts) * ADC_REFERENCE_VOLTS) / ADC_MAX_COUNT;
  return adcPinVolts / dividerRatio(channel);
}

float calibratedInputVoltsFromCounts(uint16_t counts, const Channel &channel) {
  return inputVoltsFromCounts(counts, channel) * channel.calibrationFactor;
}

bool channelPasses(float inputVolts, const Channel &channel) {
  return inputVolts >= (channel.standardVolts - channel.toleranceVolts) &&
         inputVolts <= (channel.standardVolts + channel.toleranceVolts);
}

void setLights(bool pass) {
  digitalWrite(ERROR_LED_PIN, pass ? LOW : HIGH);
  digitalWrite(PASS_LED_PIN, pass ? HIGH : LOW);
}

void printSpaces(uint8_t count) {
  while (count-- > 0) {
    Serial.print(' ');
  }
}

void printPaddedText(const char *text, uint8_t width) {
  const uint8_t len = strlen(text);
  Serial.print(text);
  if (len < width) {
    printSpaces(width - len);
  }
}

void printPaddedUint(uint16_t value, uint8_t width) {
  char buffer[8];
  ultoa(value, buffer, 10);
  const uint8_t len = strlen(buffer);
  if (len < width) {
    printSpaces(width - len);
  }
  Serial.print(buffer);
}

void printPaddedFloat(float value, uint8_t width, uint8_t precision) {
  char buffer[16];
  dtostrf(value, width, precision, buffer);
  Serial.print(buffer);
}

void finishDisplayLine() {
  printSpaces(TRAILING_CLEAR_SPACES);
  Serial.println();
}

void startupLedTest() {
  for (uint8_t i = 0; i < 3; ++i) {
    digitalWrite(ERROR_LED_PIN, HIGH);
    digitalWrite(PASS_LED_PIN, LOW);
    delay(150);
    digitalWrite(ERROR_LED_PIN, LOW);
    digitalWrite(PASS_LED_PIN, HIGH);
    delay(150);
  }

  digitalWrite(ERROR_LED_PIN, LOW);
  digitalWrite(PASS_LED_PIN, LOW);
}

void setup() {
  pinMode(ERROR_LED_PIN, OUTPUT);
  pinMode(PASS_LED_PIN, OUTPUT);
  startupLedTest();
  setLights(false);

  Serial.begin(115200);
  Serial.print(F("\x1B[2J\x1B[H"));  // Clear once after reset/startup.

  analogReference(INTERNAL);  // Nano/ATmega328P internal 1.1 V reference
  delay(10);

  // Prime ADC/mux after switching references.
  for (size_t i = 0; i < CHANNEL_COUNT; ++i) {
    analogRead(channels[i].pin);
  }
}

void loop() {
  bool allPass = true;

  // ANSI cursor-home. This overwrites the previous fixed-width display without
  // clearing the whole screen, reducing terminal flashing/flicker.
  Serial.print(F("\x1B[H"));
  Serial.print(F("FC2 sensor harness test"));
  finishDisplayLine();
  Serial.print(F("Signal    Ch  Counts   InputV  Standard       Tol  Result"));
  finishDisplayLine();
  Serial.print(F("--------  --  ------  -------  --------  --------  ------"));
  finishDisplayLine();

  for (size_t i = 0; i < CHANNEL_COUNT; ++i) {
    const Channel &channel = channels[i];
    const uint16_t counts = readAdcMedian3(channel.pin);
    const float inputVolts = calibratedInputVoltsFromCounts(counts, channel);
    const bool pass = channelPasses(inputVolts, channel);

    if (!pass) {
      allPass = false;
    }

    char pinName[4];
    pinName[0] = 'A';
    itoa(channel.pin - A0, &pinName[1], 10);

    printPaddedText(channel.name, 8);
    Serial.print(F("  "));
    printPaddedText(pinName, 2);
    Serial.print(F("  "));
    printPaddedUint(counts, 6);
    Serial.print(F("  "));
    printPaddedFloat(inputVolts, 7, 3);
    Serial.print(F("  "));
    printPaddedFloat(channel.standardVolts, 8, 3);
    Serial.print(F("  +/-"));
    printPaddedFloat(channel.toleranceVolts, 5, 3);
    Serial.print(F("  "));
    Serial.print(pass ? F("PASS") : F("FAIL"));
    finishDisplayLine();
  }

  Serial.print(allPass ? F("unit PASS") : F("unit FAIL"));
  finishDisplayLine();
  finishDisplayLine();

  setLights(allPass);
  delay(LOOP_PERIOD_MS);
}
