// Monitor A0 and A1 and indicate whether both voltages are in range.
//
// Uses the ATmega328P internal 1.1 V analog reference for A/D measurements.
// The monitored voltages must be divided/scaled so the A0/A1 pins never exceed 1.1 V.
// The calculations below assume a divide-by-2 input scale, so the source voltage is
// twice the ADC pin voltage. Adjust INPUT_SCALE_NUMERATOR/DENOMINATOR if the input
// scaling hardware is different.
// If A0 source voltage is within +/-5% of 1.67 V and A1 source voltage is within +/-5% of 1.81 V:
//   D2 = ON,  D3 = OFF
// Otherwise:
//   D2 = OFF, D3 = ON
//
// Diagnostic output is printed over Serial at 115200 baud about every 100 ms.

constexpr uint8_t A0_PIN = A0;
constexpr uint8_t A1_PIN = A1;
constexpr uint8_t PASS_PIN = 2;
constexpr uint8_t FAIL_PIN = 3;

constexpr uint16_t ADC_REFERENCE_MV = 1100;
constexpr uint16_t ADC_MAX_COUNT = 1023;

// Source voltage = ADC pin voltage * INPUT_SCALE_NUMERATOR / INPUT_SCALE_DENOMINATOR.
// For a divide-by-2 input network, the ADC pin sees half of the monitored source voltage.
constexpr uint16_t INPUT_SCALE_NUMERATOR = 2;
constexpr uint16_t INPUT_SCALE_DENOMINATOR = 1;

constexpr uint16_t A0_TARGET_MV = 1670;
constexpr uint16_t A1_TARGET_MV = 1810;
constexpr uint8_t TOLERANCE_PERCENT = 5;
constexpr unsigned long LOOP_DELAY_MS = 100;

constexpr uint16_t toleranceMv(uint16_t targetMv) {
  return (targetMv * TOLERANCE_PERCENT) / 100;
}

constexpr uint16_t A0_MIN_MV = A0_TARGET_MV - toleranceMv(A0_TARGET_MV);
constexpr uint16_t A0_MAX_MV = A0_TARGET_MV + toleranceMv(A0_TARGET_MV);
constexpr uint16_t A1_MIN_MV = A1_TARGET_MV - toleranceMv(A1_TARGET_MV);
constexpr uint16_t A1_MAX_MV = A1_TARGET_MV + toleranceMv(A1_TARGET_MV);

bool lastpass = false;

uint16_t analogReadSourceMv(uint8_t pin) {
  const uint32_t adcPinMv = (static_cast<uint32_t>(analogRead(pin)) * ADC_REFERENCE_MV) / ADC_MAX_COUNT;
  return (adcPinMv * INPUT_SCALE_NUMERATOR) / INPUT_SCALE_DENOMINATOR;
}

bool inRange(uint16_t valueMv, uint16_t minMv, uint16_t maxMv) {
  return valueMv >= minMv && valueMv <= maxMv;
}

void setup() {
  Serial.begin(115200);

  analogReference(INTERNAL);
  delay(5);
  // Discard the first readings after selecting the internal reference so the ADC
  // sample/hold capacitor can settle to the new reference/input levels.
  analogRead(A0_PIN);
  analogRead(A1_PIN);

  pinMode(PASS_PIN, OUTPUT);
  pinMode(FAIL_PIN, OUTPUT);

  digitalWrite(PASS_PIN, LOW);
  digitalWrite(FAIL_PIN, HIGH);
}

void loop() {
  uint16_t a0Mv = analogReadSourceMv(A0_PIN);
  uint16_t a1Mv = analogReadSourceMv(A1_PIN);

  const bool pass1 = inRange(a0Mv, A0_MIN_MV, A0_MAX_MV) &&
                    inRange(a1Mv, A1_MIN_MV, A1_MAX_MV);

  const bool pass = pass1 ; // | lastpass;
  lastpass = pass1;

  digitalWrite(PASS_PIN, pass ? LOW : HIGH);
  digitalWrite(FAIL_PIN, pass ? HIGH : LOW);

  Serial.print(F("A0="));
  Serial.print(a0Mv);
  Serial.print(F(" Min A0="));
  Serial.print(A0_MIN_MV);
  Serial.print(F(" Max A0="));
  Serial.print(A0_MAX_MV);
  Serial.print(F(" mV, A1="));
  Serial.print(a1Mv);
  Serial.print(F(" Min A1="));
  Serial.print(A1_MIN_MV);
  Serial.print(F(" Max A1="));
  Serial.print(A1_MAX_MV);
  Serial.print(F(" mV, state="));
  Serial.println(pass ? F("PASS") : F("FAIL"));

  delay(LOOP_DELAY_MS);
}
