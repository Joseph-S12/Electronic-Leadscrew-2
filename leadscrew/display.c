#include "display.h"
#include "gpio.h"
#include <math.h>

bool indicatorLEDs[8];
uint8_t rpm_display[4];
uint8_t pitch_display[4];
int8_t rpm_decimal = 0;
int8_t pitch_decimal = 0;

//This is a table of which LEDs to activate, depending upon the digit that needs displaying
const uint8_t number[] = {
  0b00111111,
  0b00000110,
  0b01011011,
  0b01001111,
  0b01100110, // 0-4
  0b01101101,
  0b01111101,
  0b00000111,
  0b01111111,
  0b01101111, // 5-9
  0b00000000, // Blank
  0b01000000, // Hyphen
};

static void display_begin() {
  gpio_put(DISPLAY_STB_PIN, 0);
  sleep_us(10);
}

static void display_end() {
  gpio_put(DISPLAY_STB_PIN, 1);
  sleep_us(10);
}

static void display_byte(uint8_t byte) {
  for (int8_t i = 7; i >= 0; i--) {
    gpio_put(DISPLAY_SIO_PIN, byte & 1);
    byte >>= 1;
    gpio_put(DISPLAY_CLK_PIN, 0);
    sleep_us(10);
    gpio_put(DISPLAY_CLK_PIN, 1);
    sleep_us(10);
  }
}

void initialiseDisplay() {
  uint8_t command0 = 0b10001111;
  uint8_t command1 = 0b01000000;
  uint8_t command2 = 0b11000000;

  //initialise outputs
  gpio_put(DISPLAY_STB_PIN, 1);
  sleep_us(100);
  gpio_put(DISPLAY_CLK_PIN, 1);

  //Init indicatorLEDs
  for (size_t i = 0; i < 8; i++) {
    indicatorLEDs[i]=1;
  }

  //Init display digits
  for (size_t i = 0; i < 4; i++) {
    rpm_display[i]=0;
    pitch_display[i]=0;
  }

  //Start the board
  display_begin();
  display_byte(command0);

  //Start resetting by setting to auto increment
  display_byte(command1);
  display_end();

  //Set start address to 0
  display_begin();
  display_byte(command2);

  //Wipe all display digits
  for (size_t x = 0; x < 16; x++)
    display_byte(number[10]);

  display_end();

  //Flashes the LED
  gpio_flash_led(500, 500);

  updateRPM(-1, -1);
  updatePitch(-1, -1);
  printDisplay();
}

void updateStatus(int status){
  for (size_t i = 0; i < 8; ++i) {
    indicatorLEDs[i] = status & 1;
    status >>= 1;
  }
}

void update_display(uint8_t display[], uint16_t number) {
  for(int i=0; i<4; ++i) display[i] = 0;

  if(number > 9999) {
    for(int i=0; i<4; ++i) display[i] = 11;
    return;
  }

  if(number >= 8000) { display[3] += 8; number -= 8000; }
  if(number >= 4000) { display[3] += 4; number -= 4000; }
  if(number >= 2000) { display[3] += 2; number -= 2000; }
  if(number >= 1000) { display[3] += 1; number -= 1000; }
  if(!display[3]) display[3] = 10;

  if(number >= 800) { display[2] += 8; number -= 800; }
  if(number >= 400) { display[2] += 4; number -= 400; }
  if(number >= 200) { display[2] += 2; number -= 200; }
  if(number >= 100) { display[2] += 1; number -= 100; }
  if(!display[2] && display[3] == 10) display[2] = 10;

  if(number >= 80) { display[1] += 8; number -= 80; }
  if(number >= 40) { display[1] += 4; number -= 40; }
  if(number >= 20) { display[1] += 2; number -= 20; }
  if(number >= 10) { display[1] += 1; number -= 10; }
  if(!display[1] && display[2] == 10) display[1] = 10;

  display[0] = number;
}

void updateRPM(uint16_t rpm_int, int decimal_position) {
  update_display(rpm_display, rpm_int);
  rpm_decimal = decimal_position;
}

void updatePitch(uint16_t pitch_int, int decimal_position) {
  update_display(pitch_display, pitch_int);
  pitch_decimal = decimal_position;
}

void printDisplay(){
  uint8_t command = 0b11000000;

  //Set start address
  display_begin();
  display_byte(command);

  //Set rpm digits
  for (int x = 3; x >= 0; --x) {
    uint8_t decimal_indicator = (x == rpm_decimal) ? 0x80 : 0;
    display_byte(number[rpm_display[x]] | decimal_indicator);
    display_byte(indicatorLEDs[x + 4] ? 0xff : 0x00);
  }
  //Set pitch digits
  for (int x = 3; x >= 0; --x) {
    uint8_t decimal_indicator = (x == pitch_decimal) ? 0x80 : 0;
    display_byte(number[pitch_display[x]] | decimal_indicator);
    display_byte(indicatorLEDs[x] ? 0xff : 0x00);
  }

  display_end();
}
