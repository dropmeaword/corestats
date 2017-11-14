#include <corestats.h>

#define SENSOR_PIN A0

// normalize incoming signal to be inside range [0, 1]
AdaptiveNormalizer normalizer(0, 1);

void setup() {
  Serial.begin(115200);
}

void loop() {
  // process input.
  int reading = analogRead(SENSOR_PIN);

  normalizer.put( reading );
  
  Serial.print(reading);
  Serial.print(",");
  Serial.print( normalizer.get() );
  Serial.println();

  delay(20);
}
