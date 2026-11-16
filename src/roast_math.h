// Arithmetic of a roast-meter reading, kept free of Arduino so it can be
// compiled and tested on a desktop (test/host/roast_math_test.cpp).
//
// A reading is:   count  --gain-->  normalised count  --line-->  "agtron"
//
//   count   mean of the last RING_SIZE raw 18-bit IR samples
//   gain    1 until a reference is stored; then whatever makes the stored
//           reference count map to REFERENCE_AGTRON exactly
//   line    the original firmware's map, x - (intersectionPoint - x) * deviation
//           with x = count / 1000, evaluated without truncating x
#ifndef ROAST_MATH_H
#define ROAST_MATH_H

#include <math.h>
#include <stdint.h>

#define RING_SIZE 25              // 2 s of samples at 50 Hz averaged by 4
#define REFERENCE_AGTRON 230.0f   // what baking soda reads, by community convention

// A settled reading this close to REFERENCE_AGTRON before normalising is taken
// to be the baking-soda reference. No roasted coffee reads anywhere near it.
#define REFERENCE_BAND 20.0f
// "Settled": the ring is full and its scatter is under this many units.
#define SETTLED_SD 0.5f

// Mean and standard deviation of the most recent RING_SIZE samples.
struct SampleRing {
  uint32_t samples[RING_SIZE];
  uint8_t next = 0;
  uint8_t count = 0;

  void clear() { next = 0; count = 0; }

  void push(uint32_t v) {
    samples[next] = v;
    next = (next + 1) % RING_SIZE;
    if (count < RING_SIZE) count++;
  }

  bool full() const { return count == RING_SIZE; }

  float mean() const {
    if (count == 0) return 0.0f;
    uint64_t sum = 0;
    for (uint8_t i = 0; i < count; i++) sum += samples[i];
    return (float)((double)sum / count);
  }

  // Sample standard deviation, in counts. 0 with fewer than two samples.
  float sd() const {
    if (count < 2) return 0.0f;
    double m = 0.0;
    for (uint8_t i = 0; i < count; i++) m += samples[i];
    m /= count;
    double ss = 0.0;
    for (uint8_t i = 0; i < count; i++) {
      double d = samples[i] - m;
      ss += d * d;
    }
    return (float)sqrt(ss / (count - 1));
  }
};

// The original map, on an un-truncated x = count / 1000.
inline float lineToAgtron(float x, int intersectionPoint, float deviation) {
  return x - (intersectionPoint - x) * deviation;
}

// Its inverse: the x that reads as `agtron`.
inline float agtronToLine(float agtron, int intersectionPoint, float deviation) {
  return (agtron + intersectionPoint * deviation) / (1.0f + deviation);
}

// Multiplier on the count. referenceCount <= 0 (or NaN) means "no reference".
inline float referenceGain(float referenceCount, int intersectionPoint, float deviation) {
  if (!(referenceCount > 0.0f)) return 1.0f;
  return agtronToLine(REFERENCE_AGTRON, intersectionPoint, deviation) * 1000.0f / referenceCount;
}

inline float countToAgtron(float count, float referenceCount, int intersectionPoint, float deviation) {
  float x = count * referenceGain(referenceCount, intersectionPoint, deviation) / 1000.0f;
  return lineToAgtron(x, intersectionPoint, deviation);
}

// One count of sensor noise, expressed in agtron units.
inline float countsToAgtronSpan(float counts, float referenceCount, int intersectionPoint, float deviation) {
  return counts * referenceGain(referenceCount, intersectionPoint, deviation) / 1000.0f * (1.0f + deviation);
}

// Is this settled, un-normalised reading the baking-soda reference?
inline bool looksLikeReference(float rawAgtron) {
  return fabs(rawAgtron - REFERENCE_AGTRON) <= REFERENCE_BAND;
}

#endif
