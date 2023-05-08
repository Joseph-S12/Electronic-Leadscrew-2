#include "display.h"
#include "gpio.h"
#include <math.h>

bool indicatorLEDs[8];
uint8_t rpm_display[4];
uint8_t pitch_display[4];

//This is a table of which LEDs to activate, depending upon the digit that needs displaying
const bool number[][8] = {  {0,0,1,1,1,1,1,1}, // 0
                            {0,0,0,0,0,1,1,0},
                            {0,1,0,1,1,0,1,1},
                            {0,1,0,0,1,1,1,1},
                            {0,1,1,0,0,1,1,0},
                            {0,1,1,0,1,1,0,1},
                            {0,1,1,1,1,1,0,1},
                            {0,0,0,0,0,1,1,1},
                            {0,1,1,1,1,1,1,1},
                            {0,1,1,0,1,1,1,1}, // 9
                            {0,0,0,0,0,0,0,0}, //
                            {0,1,0,0,0,0,0,0}, // -
                          };

void initialiseDisplay() {
  bool command0[8] = {1,0,0,0,1,1,1,1};
  bool command1[8] = {0,1,0,0,0,0,0,0};
  bool command2[8] = {1,1,0,0,0,0,0,0};

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
  gpio_put(DISPLAY_STB_PIN, 0);
  for (int8_t i = 7; i >= 0; i--) {
    sleep_us(100);
    gpio_put(DISPLAY_SIO_PIN, command0[i]);
    gpio_put(DISPLAY_CLK_PIN, 0);
    sleep_us(100);
    gpio_put(DISPLAY_CLK_PIN, 1);
  }
  gpio_put(DISPLAY_STB_PIN, 1);
  sleep_us(100);

  //Start resetting by setting to auto increment
  gpio_put(DISPLAY_STB_PIN, 0);
  for (int8_t i = 7; i >= 0; i--) {
    sleep_us(100);
    gpio_put(DISPLAY_SIO_PIN, command1[i]);
    gpio_put(DISPLAY_CLK_PIN, 0);
    sleep_us(100);
    gpio_put(DISPLAY_CLK_PIN, 1);
  }
  gpio_put(DISPLAY_STB_PIN, 1);
  sleep_us(100);

  //Set start address to 0
  gpio_put(DISPLAY_STB_PIN, 0);
  for (int8_t i = 7; i >= 0; i--) {
    sleep_us(100);
    gpio_put(DISPLAY_SIO_PIN, command2[i]);
    gpio_put(DISPLAY_CLK_PIN, 0);
    sleep_us(100);
    gpio_put(DISPLAY_CLK_PIN, 1);
  }

  //Wipe all display digits
  for (size_t x = 0; x < 16; x++) {
    for (int8_t i = 7; i >= 0; i--) {
      sleep_us(100);
      gpio_put(DISPLAY_SIO_PIN, 0);
      gpio_put(DISPLAY_CLK_PIN, 0);
      sleep_us(100);
      gpio_put(DISPLAY_CLK_PIN, 1);
    }
  }
  gpio_put(DISPLAY_STB_PIN, 1);
  sleep_us(100);

  //Flashes the LED`
  sleep_ms(500);
  gpio_put(25, true);
  sleep_ms(500);
  gpio_put(25, false);

  updateRPM(0000);
  updatePitch(1500);
  printDisplay();
}

void updateStatus(int status){

  for (size_t i = 0; i < 8; i++) {
    indicatorLEDs[i] = ((status & (int) pow(2, (double) i))!=0);
  }
}

void update_display(uint8_t display[], uint16_t number) {
  for(int i=0; i<4; ++i) display[i] = 0;

  if(number > 9999) {
    for(int i=0; i<4; ++i) display[i] = 11;
    return;
  }

  if(number >= 8000) { display[0] += 8; number -= 8000; }
  if(number >= 4000) { display[0] += 4; number -= 4000; }
  if(number >= 2000) { display[0] += 2; number -= 2000; }
  if(number >= 1000) { display[0] += 1; number -= 1000; }
  if(!display[0]) display[0] = 10;

  if(number >= 800) { display[1] += 8; number -= 800; }
  if(number >= 400) { display[1] += 4; number -= 400; }
  if(number >= 200) { display[1] += 2; number -= 200; }
  if(number >= 100) { display[1] += 1; number -= 100; }
  if(!display[1] && display[0] == 10) display[1] = 10;

  if(number >= 80) { display[2] += 8; number -= 80; }
  if(number >= 40) { display[2] += 4; number -= 40; }
  if(number >= 20) { display[2] += 2; number -= 20; }
  if(number >= 10) { display[2] += 1; number -= 10; }
  if(!display[2] && display[1] == 10) display[2] = 10;

  display[3] = number;
}

void updateRPM(uint16_t rpm_int) {
  update_display(rpm_display, rpm_int);
}

void updatePitch(uint16_t pitch_int) {
  update_display(pitch_display, pitch_int);
}

void printDisplay(){
  bool command[8] = {1,1,0,0,0,0,0,0};

  //Set start address
  gpio_put(DISPLAY_STB_PIN, 0);
  for (int8_t i = 7; i >= 0; i--) {
    sleep_us(100);
    gpio_put(DISPLAY_SIO_PIN, command[i]);
    gpio_put(DISPLAY_CLK_PIN, 0);
    sleep_us(100);
    gpio_put(DISPLAY_CLK_PIN, 1);
  }


  //Set rpm digits
  for (size_t x = 0; x < 4; x++) {
    for (int8_t i = 7; i >= 0; i--) {
      sleep_us(100);
      gpio_put(DISPLAY_SIO_PIN, number[rpm_display[x]][i]);
      gpio_put(DISPLAY_CLK_PIN, 0);
      sleep_us(100);
      gpio_put(DISPLAY_CLK_PIN, 1);
    }
    for (int8_t i = 7; i >= 0; i--) {
      sleep_us(100);
      gpio_put(DISPLAY_SIO_PIN, indicatorLEDs[x]);
      gpio_put(DISPLAY_CLK_PIN, 0);
      sleep_us(100);
      gpio_put(DISPLAY_CLK_PIN, 1);
    }
  }
  //Set pitch digits
  for (size_t x = 0; x < 4; x++) {
    for (int8_t i = 7; i >= 0; i--) {
      sleep_us(100);
      if (x == 0 && i == 0){
        gpio_put(DISPLAY_SIO_PIN, 1);
      }
      else {
        gpio_put(DISPLAY_SIO_PIN, number[pitch_display[x]][i]);
      }
      gpio_put(DISPLAY_CLK_PIN, 0);
      sleep_us(100);
      gpio_put(DISPLAY_CLK_PIN, 1);
    }
    for (int8_t i = 7; i >= 0; i--) {
      sleep_us(100);
      gpio_put(DISPLAY_SIO_PIN, indicatorLEDs[x+4]);
      gpio_put(DISPLAY_CLK_PIN, 0);
      sleep_us(100);
      gpio_put(DISPLAY_CLK_PIN, 1);
    }
  }
  gpio_put(DISPLAY_STB_PIN, 1);
  sleep_us(100);
}
