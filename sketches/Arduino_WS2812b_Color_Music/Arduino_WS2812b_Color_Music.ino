#include "FastLED.h"
#include "IRremote.h"

#define BTN_MODE_SWT_PIN 2 // Button PIN for change color mode
#define IR_RECEIVER_MODE_SWT_PIN 3 // IR PIN for control modes by remote control

#define MODE_COUNT 2 // count of color modes

#define SPEED_0 150
#define SPEED_1 10

#define LED_TYPE WS2811
#define NUM_LEDS 50 // 290. Count of leds
#define LED_DT 12 // Pin to cotrol ws812b leds 

// IR vars
//IRrecv irrecv(IR_MODE_SWT_PIN);
decode_results results;

// Switch mode button vars
boolean btn_mode_swt_last_state = LOW; // last state for switch mode button
boolean btn_mode_swt_current_state = LOW;

byte current_mode = 0; // Currect color mode

// WS2812B vars
int max_bright = 30;
int start_from = 0;

byte r = 0;
byte g = 0;
byte b = 0;

int colors[][3] =
{
  {255, 0, 0}, // red
  {255, 128, 0}, // orange
  {255, 255, 0}, // yellow
  {0, 255, 0}, // green
  {0, 255, 255}, // cyan
  {0, 0, 255}, // blue
  {128, 0, 128}, // purple
};

CRGB leds[NUM_LEDS];

//@TODO: rewrite to get button pin as parameter
boolean debounce(uint8_t btn_pin, boolean last)
{
  boolean current = digitalRead(btn_pin);

  if (last != current)
  {
    delay(5);
    current = digitalRead(btn_pin);
  }

  return current;
}

void one_color_all(int cred, int cgrn, int cblu) {       //-SET ALL LEDS TO ONE COLOR
  for (int i = 0 ; i < NUM_LEDS; i++ ) {
    leds[i].setRGB( cred, cgrn, cblu);
  }
}

// moved 7 colors of rainbow to 1 led every tik
void mode_0() {
  int colors_count = (sizeof(colors) / sizeof(colors[0]));
  int color_num = start_from;

  for (int i = 0 ; i < NUM_LEDS; i++ ) {
    if (color_num >= colors_count) {
      color_num = 0;
    }

    leds[i].setRGB( colors[color_num][0], colors[color_num][1], colors[color_num][2]);

    color_num += 1;
  }

  start_from += 1;

  if (start_from >= colors_count) {
    start_from = 0;
  }
}

void mode_1() {
  if (colors[start_from][0] == r && colors[start_from][1] == g && colors[start_from][2] == b) {
    start_from++;

    int colors_count = (sizeof(colors) / sizeof(colors[0]));

    if (start_from >= colors_count) {
      start_from = 0;
    }
  }

  if (colors[start_from][0] > r) {
    r++;
  } else if (colors[start_from][0] < r) {
    r--;
  }

  if (colors[start_from][1] > g) {
    g++;
  } else if (colors[start_from][1] < g) {
    g--;
  }

  if (colors[start_from][2] > b) {
    b++;
  } else if (colors[start_from][2] < b) {
    b--;
  }

  one_color_all(r, g, b);
}

// setup project
void setup() {
  Serial.begin(9600);

  pinMode(BTN_MODE_SWT_PIN, INPUT_PULLUP); // pin mode for switchin modes button with pull up resistor

  FastLED.setBrightness(max_bright);  // ограничить максимальную яркость

  FastLED.addLeds<LED_TYPE, LED_DT, GRB>(leds, NUM_LEDS);

  FastLED.clear();
  FastLED.show();

  IrReceiver.begin(IR_RECEIVER_MODE_SWT_PIN, ENABLE_LED_FEEDBACK, USE_DEFAULT_FEEDBACK_LED_PIN); // Enable IR for switching modes
}


void loop() {
  //Serial.print("Current mode: ");
  //Serial.println(current_mode);
  //delay(1000);

  //The function decode(&results)) is deprecated and may not work as expected! Just use decode() without a parameter and IrReceiver.decodedIRData.<fieldname> .

  if (IrReceiver.decode()) // если данные пришли выполняем команды
   {
     Serial.print("IR code: ");
     Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);

    //  IrReceiver.printIRResultShort(&Serial);

    //  if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
    //         // We have an unknown protocol here, print more info
    //         IrReceiver.printIRResultRawFormatted(&Serial, true);
    //     }
    // Serial.println();

      /*
      16761405 - button play
      */

     switch (IrReceiver.decodedIRData.decodedRawData) {
       case 0xF807FF00: // button "-"
        if (--current_mode < 0) current_mode = MODE_COUNT - 1;
        break;
       case 0xEA15FF00: // button "+"
        if (++current_mode >= MODE_COUNT) current_mode = 0;
        break;
      case 0xF30CFF00: // button "1"
        current_mode = 0;
        break;
      case 0xE718FF00: // button "2"
        current_mode = 1;
        break;
      break;        
     }

     IrReceiver.resume(); 
   }

  // check if button was pressed
  btn_mode_swt_current_state = debounce(BTN_MODE_SWT_PIN, btn_mode_swt_last_state);

  Serial.print("last: ");
     Serial.println(btn_mode_swt_last_state);
     
  Serial.print("current: ");
     Serial.println(btn_mode_swt_current_state);

  // if last state was low and curret low change mode. Current state low as pressed because used INPUT_PULLUP pin mode
  if (btn_mode_swt_last_state == HIGH && btn_mode_swt_current_state == LOW)
  {
    if (++current_mode >= MODE_COUNT) current_mode = 0;
  }

  btn_mode_swt_last_state = btn_mode_swt_current_state;

  // check current mode and run function for show leds
  switch (current_mode) {
    case 0:
      mode_0();
      delay(SPEED_0);
      break;
    case 1:
      mode_1();
      delay(SPEED_1);
      break;
  }

  FastLED.show();    
}
