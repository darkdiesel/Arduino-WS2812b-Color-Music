#include "IRremote.h"
#define pinIR 3

void setup() {
  Serial.begin(9600);
  Serial.println(F("Startup"));
  
  IrReceiver.begin(pinIR, ENABLE_LED_FEEDBACK, USE_DEFAULT_FEEDBACK_LED_PIN); // Enable IR for switching modes
}

void loop() {
  if (IrReceiver.decode()) // if data received run commands
   {
     Serial.print("0x");
     Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);

     IrReceiver.printIRResultShort(&Serial);

     if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
            // We have an unknown protocol here, print more info
            IrReceiver.printIRResultRawFormatted(&Serial, true);
        }
    
     IrReceiver.resume(); 
   }

}
