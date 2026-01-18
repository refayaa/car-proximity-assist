// ======================================================
//  Ultrasonic CW Receiver – Averaged Output Every 3 sec
// ======================================================

const int pwmPin     = 25;
const int pwmChannel = 0;
const int pwmFreq    = 40000;
const int pwmRes     = 8;

const int adcPin = 34;
const float ADC_REF_VOLT = 3.3;
const int ADC_MAX = 4095;

// ===== Calibration Table =====
const int N_POINTS = 9;

float distPoints[N_POINTS] = {
  10, 20, 30, 40, 50, 60, 70, 90, 100
};

float voltPoints[N_POINTS] = {
  3.0, 1.8, 0.8, 0.50, 0.25, 0.15, 0.07, 0.03, 0.01
};

// ===== Average ADC VOLTAGE over N samples =====
float getAveragedAdcVoltage(int samples) {
  long sum = 0;

  for (int i = 0; i < samples; i++) {
    int raw = analogRead(adcPin);
    sum += raw;
    delay(2);                     // slight spacing to reduce correlation noise
  }

  float avgRaw = (float)sum / samples;
  return (avgRaw / ADC_MAX) * ADC_REF_VOLT;
}

// ===== Voltage → Distance Mapping =====
float voltageToDistance(float v) {
  if (v >= voltPoints[0]) return distPoints[0];
  if (v <= voltPoints[N_POINTS - 1]) return distPoints[N_POINTS - 1];

  for (int i = 0; i < N_POINTS - 1; i++) {
    float vHigh = voltPoints[i];
    float vLow  = voltPoints[i + 1];

    if (v <= vHigh && v >= vLow) {
      float dHigh = distPoints[i];
      float dLow  = distPoints[i + 1];

      float t = (vHigh - v) / (vHigh - vLow);
      return dHigh + t * (dLow - dHigh);
    }
  }

  return -1;
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  delay(1000);

  analogReadResolution(12);
  analogSetPinAttenuation(adcPin, ADC_11db);

  ledcSetup(pwmChannel, pwmFreq, pwmRes);
  ledcAttachPin(pwmPin, pwmChannel);
  ledcWrite(pwmChannel, 128);

  Serial.println("\n=== Stable Distance Output (Every 3 sec) ===\n");
}

// ===== LOOP =====
void loop() {
  // Take many samples (100 is good – you can increase to 200+ if needed)
  int NUM_SAMPLES = 100;

  float vAdc = getAveragedAdcVoltage(NUM_SAMPLES);
  float distance = voltageToDistance(vAdc);

  Serial.print("Avg Voltage: ");
  Serial.print(vAdc, 4);
  Serial.print(" V   |   Distance: ");

  if (distance < 0) {
    Serial.println("Out of range");
  } else {
    Serial.print(distance, 1);
    Serial.println(" cm");
  }

  delay(3000); // print every 3 seconds
}
