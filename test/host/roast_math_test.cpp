// Desktop test of src/roast_math.h.  From the repo root:
//   c++ -std=c++11 -Isrc test/host/roast_math_test.cpp -o /tmp/roast_math_test && /tmp/roast_math_test
#include <stdio.h>
#include <stdlib.h>

#include "roast_math.h"

static int failures = 0;
#define CHECK(cond, ...) \
  do { if (!(cond)) { failures++; printf("FAIL %s:%d  ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

// the map as the firmware had it: integer thousands in, rounded integer out
static int legacy(int rLevel, int ip, float dev) {
  int x = rLevel / 1000;
  return (int)round(x - (ip - x) * dev);
}

int main() {
  const int ip = 117;
  const float dev = 0.165f;

  // 1. With no reference, a count on a whole thousand reads what it always did.
  //    (Compared before rounding against an exact .5 tie, which float may break either way.)
  for (int x = 31; x <= 262; x++) {
    float now = countToAgtron(x * 1000.0f, 0.0f, ip, dev);
    CHECK(fabs(now - (x - (ip - x) * dev)) < 1e-3, "x=%d now=%f", x, now);
    float frac = now - floorf(now);
    if (fabs(frac - 0.5f) > 1e-3) CHECK((int)round(now) == legacy(x * 1000, ip, dev), "x=%d now=%f legacy=%d", x, now, legacy(x * 1000, ip, dev));
  }

  // 2. The remainder the old code threw away is now worth up to 1.165 units.
  float lo = countToAgtron(97000.0f, 0.0f, ip, dev), hi = countToAgtron(97999.0f, 0.0f, ip, dev);
  CHECK(legacy(97000, ip, dev) == legacy(97999, ip, dev), "legacy should not resolve within a thousand");
  CHECK(hi - lo > 1.16f && hi - lo < 1.17f, "span %f", hi - lo);

  // 3. The stored reference reads exactly REFERENCE_AGTRON, whatever its count.
  float refs[] = {150000.0f, 198765.0f, 214000.0f, 221234.5f, 250000.0f};
  for (float r : refs) CHECK(fabs(countToAgtron(r, r, ip, dev) - REFERENCE_AGTRON) < 1e-2, "ref %f reads %f", r, countToAgtron(r, r, ip, dev));

  // 4. The reading is unchanged when the LED output drifts, if the reference is re-read:
  //    sample and reference both scale by the same factor.
  float base = countToAgtron(97000.0f, 214000.0f, ip, dev);
  for (float k = 0.90f; k <= 1.10f; k += 0.05f)
    CHECK(fabs(countToAgtron(97000.0f * k, 214000.0f * k, ip, dev) - base) < 2e-2, "k=%f reads %f, base %f", k, countToAgtron(97000.0f * k, 214000.0f * k, ip, dev), base);

  // 5. An unset or garbage reference (erased flash is NaN) is "no reference".
  CHECK(referenceGain(0.0f, ip, dev) == 1.0f, "zero");
  CHECK(referenceGain(-5.0f, ip, dev) == 1.0f, "negative");
  CHECK(referenceGain(NAN, ip, dev) == 1.0f, "nan");

  // 6. The line and its inverse agree; with these constants soda sits at x = 214.0.
  CHECK(fabs(agtronToLine(230.0f, ip, dev) - 213.996f) < 1e-2, "x230 = %f", agtronToLine(230.0f, ip, dev));
  for (float a = 30.0f; a < 240.0f; a += 7.3f) CHECK(fabs(lineToAgtron(agtronToLine(a, ip, dev), ip, dev) - a) < 1e-3, "roundtrip %f", a);

  // 7. Ring: mean and sd of the most recent RING_SIZE samples only.
  SampleRing ring;
  CHECK(ring.mean() == 0.0f && ring.sd() == 0.0f && !ring.full(), "empty ring");
  for (int i = 0; i < 100; i++) ring.push(999999);         // must be forgotten
  for (int i = 0; i < RING_SIZE; i++) ring.push(97000 + i); // 97000 .. 97024
  CHECK(ring.full(), "full");
  CHECK(fabs(ring.mean() - 97012.0f) < 0.01f, "mean %f", ring.mean());
  CHECK(fabs(ring.sd() - 7.3598f) < 0.01f, "sd %f", ring.sd());
  ring.clear();
  ring.push(262143); ring.push(262143);                     // full scale does not overflow
  CHECK(fabs(ring.mean() - 262143.0f) < 0.5f && ring.sd() == 0.0f, "full scale mean %f sd %f", ring.mean(), ring.sd());

  // 8. Noise in counts converts to units through the same gain and slope.
  CHECK(fabs(countsToAgtronSpan(1000.0f, 0.0f, ip, dev) - 1.165f) < 1e-3, "span");

  // 9. Only something near 230 is taken for the reference; the lightest coffee is not.
  CHECK(looksLikeReference(230.0f) && looksLikeReference(212.0f) && looksLikeReference(249.0f), "soda band");
  CHECK(!looksLikeReference(130.0f) && !looksLikeReference(205.0f) && !looksLikeReference(255.0f), "not soda");

  printf(failures ? "%d FAILED\n" : "all passed\n", failures);
  return failures ? 1 : 0;
}
