#include "FastLED.h"

#define BTN_MODE_SWT_PIN 2 // Button for change color mode

#define MODE_COUNT 2 // count of color modes

#define SPEED_0 250
#define SPEED_1 10

#define NUM_LEDS 150 // 290. Count of leds
#define LED_DT 12 // Pin to cotrol ws812b leds 

boolean btn_mode_swt_last_state = LOW;
boolean btn_mode_swt_current_state = LOW;

byte current_mode = 0; // Currect color mode

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

  LEDS.setBrightness(max_bright);  // ограничить максимальную яркость

  FastLED.addLeds<WS2811, LED_DT, GRB>(leds, NUM_LEDS);

  //(0, 0, 0);          // погасить все светодиоды
  LEDS.show();
}


void loop() {
  btn_mode_swt_current_state = debounce(BTN_MODE_SWT_PIN, btn_mode_swt_last_state);

  if (btn_mode_swt_last_state == LOW && btn_mode_swt_current_state == HIGH)
  {
    if (++current_mode >= MODE_COUNT) current_mode = 0;
  }

  btn_mode_swt_last_state = btn_mode_swt_current_state;

  Serial.println(current_mode);

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

  LEDS.show();
}
