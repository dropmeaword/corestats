#include <corestats.h>

#define SENSOR_PIN A0

MovingAverage mavg(0.01);

void setup() {
  Serial.begin(115200);
}

void loop() {
  // process input.
  int reading = analogRead(SENSOR_PIN);

  mavg.update( reading );
  
  Serial.print(reading);
  Serial.print(",");
  Serial.print( mavg.get() );
  Serial.println();

  delay(20);
}
