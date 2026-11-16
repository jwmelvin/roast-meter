// VERSION 1.1.0-fr  (full resolution, averaged, reference-normalised)
//
// What changed from 1.0.1-beta, and why:
//
//  * The reading keeps the whole IR count. 1.0.1 divided the count by 1000 and
//    dropped the remainder before mapping it, so the display moved in steps of
//    1.165. It now shows tenths.
//  * The reading is the mean of the last 2 s of samples (RING_SIZE), not one
//    sample, and the scatter of those samples is shown beside it.
//  * Baking soda re-references the meter. Any settled reading within
//    REFERENCE_BAND of 230 is taken to be the soda standard: from then until
//    power-off every reading is scaled so that soda is exactly 230.0. Read soda
//    at the start of a session and LED drift since calibration drops out.
//    Until soda has been read the meter behaves as before (ledBrightness alone
//    sets the scale) and the display says "raw".
//
// The arithmetic is in roast_math.h and is tested on a desktop
// (test/host/roast_math_test.cpp).
#include <SFE_MicroOLED.h>
#include <Wire.h>

#include "MAX30105.h"
#include "roast_math.h"

#define PIN_RESET 9
#define DC_JUMPER 1

#define DISPLAY_INTERVAL_MS 250

MAX30105 particleSensor;
MicroOLED oled(PIN_RESET, DC_JUMPER);

const uint32_t unblockedValue = 30000;  // counts below this (+100) mean "no sample"

// The map from IR to the displayed number: x - (intersectionPoint - x) * deviation
const int intersectionPoint = 117;
const float deviation = 0.165;

SampleRing ring;
float referenceCount = 0;  // mean count on baking soda this session; 0 = not read yet
unsigned long lastDisplay = 0;

void displayLoadSample() {
  oled.clear(PAGE);
  oled.setCursor(0, 0);
  oled.setFontType(1);
  oled.print("Please load   sample!");
  oled.display();
}

// Big whole number on the left, as before. To its right: the tenths, the
// scatter of the samples behind it (same units), and whether a reference is
// in force. Tenths show ".." until 2 s of samples are in.
void displayMeasurement(float agtron, float sdUnits, bool settled, bool isReference) {
  long tenths = lround(agtron * 10.0f);
  if (tenths < 0) tenths = 0;
  int whole = tenths / 10;
  int frac = tenths % 10;

  oled.clear(PAGE);

  oled.setFontType(3);
  int bigWidth = oled.getFontWidth();
  String wholeText = String(whole);
  int pad = 3 - (int)wholeText.length();
  if (pad < 0) pad = 0;
  oled.setCursor(pad * (bigWidth + 1), 0);  // the library advances one pixel past each glyph
  oled.print(wholeText);

  int column = 3 * (bigWidth + 1) + 1;

  oled.setFontType(1);
  oled.setCursor(column, 0);
  if (ring.full()) {
    oled.print(".");
    oled.print(frac);
  } else {
    oled.print("..");
  }

  oled.setFontType(0);
  oled.setCursor(column, 20);
  oled.print(sdUnits < 9.995f ? String(sdUnits, 2) : String("9.99"));
  oled.setCursor(column, 30);
  oled.print(settled ? "ok" : "~");
  oled.setCursor(column, 40);
  oled.print(isReference ? "REF" : (referenceCount > 0 ? "ref" : "raw"));

  oled.display();
}

// Pull every new sample out of the sensor. Returns the latest one, 0 if none arrived.
uint32_t drainSensor() {
  uint32_t latest = 0;
  particleSensor.check();
  while (particleSensor.available()) {
    latest = particleSensor.getFIFOIR();
    particleSensor.nextSample();
    if ((long)latest - (long)unblockedValue > 100L) {
      ring.push(latest);
    } else {
      ring.clear();  // sample removed: the next one starts from nothing
    }
  }
  return latest;
}

void setup() {
  Serial.begin(9600);

  Wire.begin();
  oled.begin();      // Initialize the OLED
  oled.clear(ALL);   // Clear the display's internal memory
  oled.clear(PAGE);  // Clear the buffer.

  delay(100);  // Delay 100 ms
  oled.setFontType(3);

  // Initialize sensor
  if (particleSensor.begin(Wire, I2C_SPEED_FAST) == false)  // Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1)
      ;
  }

  // The variable below calibrates the LED output on your hardware: set it so
  // baking soda reads near 230 with "raw" showing. It only has to land within
  // REFERENCE_BAND of 230 -- the soda reading itself then sets the exact scale.
  byte ledBrightness = 128;  // this meter: 32 at the old 4096 ADC range, x4 for 16384 (upstream used 135)

  byte sampleAverage = 4;  // Options: 1, 2, 4, 8, 16, --32--
  byte ledMode = 2;        // Options: 1 = Red only, --2 = Red + IR--, 3 = Red + IR + Green
  int sampleRate = 50;     // Options: 50, 100, 200, 400, 800, 1000, 1600, --3200--
  int pulseWidth = 411;    // Options: 69, 118, 215, --411--
  int adcRange = 16384;    // Options: 2048, --4096--, 8192, 16384

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);  // Configure sensor with these settings

  particleSensor.setPulseAmplitudeRed(0);
  particleSensor.setPulseAmplitudeGreen(0);

  particleSensor.disableSlots();
  particleSensor.enableSlot(2, 0x02);  // Enable only SLOT_IR_LED = 0x02

  particleSensor.clearFIFO();
}

void loop() {
  drainSensor();

  if (millis() - lastDisplay < DISPLAY_INTERVAL_MS) return;
  lastDisplay = millis();

  if (ring.count == 0) {
    displayLoadSample();
    return;
  }

  float count = ring.mean();
  float sdCounts = ring.sd();

  // What 1.0.1 would have shown, at full resolution: the test for "is this soda?"
  float rawAgtron = countToAgtron(count, 0, intersectionPoint, deviation);
  bool rawSettled = ring.full() && countsToAgtronSpan(sdCounts, 0, intersectionPoint, deviation) < SETTLED_SD;

  bool isReference = rawSettled && looksLikeReference(rawAgtron);
  if (isReference) referenceCount = count;

  float agtron = countToAgtron(count, referenceCount, intersectionPoint, deviation);
  float sdUnits = countsToAgtronSpan(sdCounts, referenceCount, intersectionPoint, deviation);
  bool settled = ring.full() && sdUnits < SETTLED_SD;

  displayMeasurement(agtron, sdUnits, settled, isReference);

  Serial.println("real:" + String(count, 0));
  Serial.println("sd:" + String(sdCounts, 1));
  Serial.println("n:" + String(ring.count));
  Serial.println("raw:" + String(rawAgtron, 1));
  Serial.println("reference:" + String(referenceCount, 0));
  Serial.println("agtron:" + String(agtron, 1));
  Serial.println("===========================");
}
