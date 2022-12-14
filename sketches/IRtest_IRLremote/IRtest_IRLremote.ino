// Original sketch https://github.com/AlexGyver/ColorMusic/tree/master/firmware/IRtest_2.0

#include "IRLremote.h"
#define pinIR 3
CHashIR IRLremote;

void setup() {
  Serial.begin(9600);
  Serial.println(F("Startup"));
  if (!IRLremote.begin(pinIR))
    Serial.println(F("You did not choose a valid pin."));
}

void loop() {
  if (IRLremote.available()) {
    auto data = IRLremote.read();
    if (data.command != 292984781) {
      Serial.print("0x");
      Serial.println(data.command, HEX);
    }
  }
}
