// WS2812B vars
#include "FastLED.h"

#define LED_TYPE WS2811 // led type
#define NUM_LEDS 100     // 290. Count of leds
#define LED_PIN 12       // Pin to cotrol ws812b leds 

#define LED_BRIGHTNESS_STEP 10 // step for change led brightness

int cur_brightness = 30;

int start_from = 0;

// mode 2 vars
#define MODE_2_LENGTH_COLOR_SEGMENT 5
int mode_2_length_rainbow = 0;
int mode_2_step = 0;

// mode 3 vars
int mode_3_ball_position = 0;
int mode_3_ball_width = 5;
int mode_3_ball_direction = 1;

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

// led modes
// #define MODE_COUNT 4 // count of color modes

// @TODO: rewrite to count modes from this array and try to define it
const int COLOR_MODE_SPEED [4] =
{
  150,
  10,
  35,
  150
};

const byte MODE_COUNT ((sizeof(COLOR_MODE_SPEED) / sizeof(COLOR_MODE_SPEED[0])));

unsigned long mode_timer; // timer for print mode
unsigned long global_timer; // global timer

// IR library alternative for IRremote.h
#include "IRLremote.h"

#define IR_RECEIVER_MODE_SWT_PIN 3 // IR PIN for control modes by remote control

CHashIR IRLremote;
uint32_t IRdata;
volatile boolean ir_flag;

// IR vars
#define IR_BTN_ON_OFF  0xAC2A56AD // Button "Play"      - on/off device
#define IR_BTN_MINUS   0x5484B6AD // Button "-"         - decrease brightness
#define IR_BTN_PLUS    0xD22353AD // Button "+"         - increase brightness
#define IR_BTN_NEXT    0x1B92DDAD // Button "Next"      - next mode
#define IR_BTN_PREV    0x517068AD // Button "Previous"  - previous mode
#define IR_BTN_1       0x18319BAD // Button "1"         - mode 0
#define IR_BTN_2       0xF39EEBAD // Button "2"         - mode 1
#define IR_BTN_3       0x4AABDFAD // Button "3"         - mode 2
#define IR_BTN_4       0xE25410AD // Button "4"         - mode 3
#define IR_BTN_5       0x297C76AD // Button "5"         - mode 4
#define IR_BTN_6       0x14CE54AD // Button "6"         - mode 5
#define IR_BTN_7       0xAF3F1BAD // Button "7"         - mode 6
#define IR_BTN_8       0xC089F6AD // Button "8"         - mode 7
#define IR_BTN_9       0x38379AD // Button "9"         - mode 8

boolean device_state = true; // on/off state for device
#define GLOBAL_LOOP 2 // global timer loop

// Switch color mode button vars
#define BTN_COLOR_MODE_SWT_PIN 2 // Button PIN for change color mode

boolean btn_color_mode_swt_last_state = LOW; // last state for switch color mode button
boolean btn_color_mode_swt_current_state = LOW;

byte current_color_mode = 0; // Current mode for color mode
byte current_music_mode = 0; // Current mode for music mode

// Swith device mode (color or music)
#define BTN_DEVICE_MODE_SWT_PIN 4 // Button PIN for change device mode
byte current_device_mode = 0; // Current mode for device (0: color / 1: music)

boolean btn_device_mode_swt_last_state = LOW; // last state for switch color mode button
boolean btn_device_mode_swt_current_state = LOW;

// Device mode indicator leds 
#define LED_DEVICE_MODE_COLOR_PIN 6
#define LED_DEVICE_MODE_MUSIC_PIN 5

boolean debounce(uint8_t btn_pin, boolean last) {
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
void mode_color_0() {
  int colors_count = (sizeof(colors) / sizeof(colors[0]));
  int color_num = start_from;

  for (int i = 0 ; i < NUM_LEDS; i++ ) {
    if (color_num++ >= colors_count) color_num = 0; // reset color to first if it last

    leds[i].setRGB( colors[color_num][0], colors[color_num][1], colors[color_num][2]);

    color_num += 1;
  }

  start_from += 1;

  if (start_from >= colors_count) {
    start_from = 0;
  }
}

void mode_color_1() {
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

// big rainbow
void mode_color_2() {
  int colors_count = (sizeof(colors) / sizeof(colors[0]));
  int color_num = start_from;

  mode_2_length_rainbow = 0;
  mode_2_length_rainbow += mode_2_step;

  for (int i = 0 ; i < NUM_LEDS; i++ ) {
    if (++mode_2_length_rainbow >= MODE_2_LENGTH_COLOR_SEGMENT) {
      if (color_num++ >= colors_count) color_num = 0; // reset color to first if it last

      mode_2_length_rainbow = 0;
    }

    leds[i].setRGB( colors[color_num][0], colors[color_num][1], colors[color_num][2]);
  }

  if (++mode_2_step >= MODE_2_LENGTH_COLOR_SEGMENT) {
    if (++start_from > colors_count) start_from = 0; // reset start
    mode_2_step = 0;
  }
}

// ball
void mode_color_3() {
  for (int i = 0 ; i <= NUM_LEDS; i++ ) {
    if ((i >= mode_3_ball_position) && (i <= (mode_3_ball_position + mode_3_ball_width - 1))) {
      leds[i].setRGB( colors[4][0], colors[4][1], colors[4][2]);
    } else {
      leds[i].setRGB( colors[0][0], colors[0][1], colors[0][2]);     
      // leds[i].setRGB( 0, 0, 0);     
    }
  }

  mode_3_ball_position += mode_3_ball_direction;

  if (((mode_3_ball_position + mode_3_ball_width - 1) >= NUM_LEDS || mode_3_ball_position < 0)) {
    mode_3_ball_direction *= -1;
    mode_3_ball_position += mode_3_ball_direction * 2;
  }
}

// setup project
void setup() {
  Serial.begin(9600);

  // setup buttons for switching device and color modes
  pinMode(BTN_COLOR_MODE_SWT_PIN, INPUT_PULLUP); // pin mode for switchin color modes button with pull up resistor
  pinMode(BTN_DEVICE_MODE_SWT_PIN, INPUT_PULLUP); // pin mode for switchin device modes button with pull up resistor

  // setup indicator leds for device mode
  pinMode(LED_DEVICE_MODE_COLOR_PIN, OUTPUT); // led for color mode
  pinMode(LED_DEVICE_MODE_MUSIC_PIN, OUTPUT); // led for music mode

  // run device mode leds
  change_device_mode();

  // setup led
  FastLED.setBrightness(cur_brightness);  // set leds brightness
  FastLED.addLeds<LED_TYPE, LED_PIN, GRB>(leds, NUM_LEDS);

  // make lead black before start
  FastLED.clear();
  FastLED.show();

  //IrReceiver.begin(IR_RECEIVER_MODE_SWT_PIN, ENABLE_LED_FEEDBACK, USE_DEFAULT_FEEDBACK_LED_PIN); // Enable IR for switching modes

  // setup IR for remote control
  IRLremote.begin(IR_RECEIVER_MODE_SWT_PIN);

  Serial.print("Count of mode: ");  
  Serial.println(MODE_COUNT);
}

void change_device_mode() {
  switch (current_device_mode) {
    case 0:  // color mode
      digitalWrite(LED_DEVICE_MODE_COLOR_PIN, HIGH);
      digitalWrite(LED_DEVICE_MODE_MUSIC_PIN, LOW);
      break;
    case 1:  // music mode
      digitalWrite(LED_DEVICE_MODE_COLOR_PIN, LOW);
      digitalWrite(LED_DEVICE_MODE_MUSIC_PIN, HIGH);
      break;
  }
}

void loop() {

  // Try receive IR data
  if (IRLremote.available()) {
    auto data = IRLremote.read();
    IRdata = data.command;
    ir_flag = true;

    if (ir_flag) {
      switch (IRdata) {
        case IR_BTN_ON_OFF:  // button "play"
          device_state = !device_state;

          if (!device_state) { // switch off leds
            FastLED.clear();
            FastLED.show();            
          }
          break;
        case IR_BTN_NEXT:   // button "Next"        
          if (!device_state)
            break;
          if (++current_color_mode >= MODE_COUNT) current_color_mode = 0; // switch to next mode
          break;
        case IR_BTN_PREV:   // button "Previous"        
          if (!device_state)
              break;
          if (--current_color_mode < 0) current_color_mode = MODE_COUNT - 1; // switch to previous mode
          break;
        case IR_BTN_MINUS:  // button "-"
          if (!device_state)
            break;
          cur_brightness -= LED_BRIGHTNESS_STEP;        

          if (cur_brightness < 0) cur_brightness = 0;
          FastLED.setBrightness(cur_brightness);  // increase leds brightness
          break;
        case IR_BTN_PLUS:  // button "+"
          if (!device_state)
            break;
              
          cur_brightness += LED_BRIGHTNESS_STEP;

          if (++cur_brightness > 255) cur_brightness = 255;

          FastLED.setBrightness(cur_brightness);  // decrease leds brightness
          break;
        case IR_BTN_1:  // button "1"
          if (!device_state)
              break;
              
          current_color_mode = 0; // change current mode to 0
          break;
        case IR_BTN_2:  // button "2"
          if (!device_state)
            break;
                
          current_color_mode = 1; // change current mode to 1
          break;
        case IR_BTN_3:  // button "3"
          if (!device_state)
            break;
                
          current_color_mode = 2; // change current mode to 2
          break;
        case IR_BTN_4:  // button "4"
          if (!device_state)
            break;
                
          current_color_mode = 3; // change current mode to 3
          break;
      }
    }

    ir_flag = false;
  }

  // ALTERNATIVE ON IR LIBRARY, DONT WORK CORRECTLY WITH WS2812B
  // if (IrReceiver.decode()) // if data received run commands
  //  {
  //    Serial.print("IR code: ");
  //    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);

  //   //  IrReceiver.printIRResultShort(&Serial);

  //   //  if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
  //   //         // We have an unknown protocol here, print more info
  //   //         IrReceiver.printIRResultRawFormatted(&Serial, true);
  //   //     }

  //    switch (IrReceiver.decodedIRData.decodedRawData) {
  //      case IR_BTN_MINUS: // button "-"
  //       if (--current_color_mode < 0) current_color_mode = MODE_COUNT - 1;
  //       break;
  //     break;        
  //    }

  //    IrReceiver.resume(); 
  //  }

  // check if change color mode button was pressed
  btn_color_mode_swt_current_state = debounce(BTN_COLOR_MODE_SWT_PIN, btn_color_mode_swt_last_state);

  // if last state was low and curret low change mode. Current state low as pressed because used INPUT_PULLUP pin mode
  if (btn_color_mode_swt_last_state == HIGH && btn_color_mode_swt_current_state == LOW)
  {
    if (++current_color_mode >= MODE_COUNT) current_color_mode = 0;
  }

  btn_color_mode_swt_last_state = btn_color_mode_swt_current_state;

  // check if change device mode button was pressed
  btn_device_mode_swt_current_state = debounce(BTN_DEVICE_MODE_SWT_PIN, btn_device_mode_swt_last_state);

  // if last state was low and curret low change mode. Current state low as pressed because used INPUT_PULLUP pin mode
  if (btn_device_mode_swt_last_state == HIGH && btn_device_mode_swt_current_state == LOW)
  {
    if (++current_device_mode >= 2) current_device_mode = 0;

    change_device_mode();
  }

  btn_device_mode_swt_last_state = btn_device_mode_swt_current_state;

  // check current mode and run function for show leds
  if (device_state) {
    if (millis() - global_timer > GLOBAL_LOOP) {
      global_timer = millis();  // reset main timer

      if (millis() - mode_timer > COLOR_MODE_SPEED[current_color_mode]) {
        mode_timer = millis();  // reset mode timer

        switch (current_color_mode) {
          case 0:
            mode_color_0();
            break;
          case 1:
            mode_color_1();
            break;
          case 2:
            mode_color_2();
            break;
          case 3:
            mode_color_3();
            break;
        }
      }
    }

    if (!IRLremote.receiving()) {  // if IR not receive signal show leds
      FastLED.show();              // send data to leds
    }
  }
}
