#include "display.h"
#include "gpio.h"
#include <math.h>

bool indicatorLEDs[8];
uint8_t rpm_display[4];
uint8_t pitch_display[4];

//This is a table of which LEDs to activate, depending upon the digit that needs displaying
const bool number[11][8] = {{0,0,1,1,1,1,1,1},
                            {0,0,0,0,0,1,1,0},
                            {0,1,0,1,1,0,1,1},
                            {0,1,0,0,1,1,1,1},
                            {0,1,1,0,0,1,1,0},
                            {0,1,1,0,1,1,0,1},
                            {0,1,1,1,1,1,0,1},
                            {0,0,0,0,0,1,1,1},
                            {0,1,1,1,1,1,1,1},
                            {0,1,1,0,1,1,1,1},
                            {0,0,0,0,0,0,0,0}};

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

void updateRPM(uint16_t rpm_int) {
  rpm_display[0]=rpm_int/1000;
  rpm_display[1]=(rpm_int/100)-(rpm_display[0]*10);
  rpm_display[2]=(rpm_int/10)-(rpm_display[0]*100)-(rpm_display[1]*10);
  rpm_display[3]=rpm_int-(rpm_display[0]*1000)-(rpm_display[1]*100)-(rpm_display[2]*10);
  //Remove header 0s
  if (rpm_display[0]==0 && rpm_display[1]==0 && rpm_display[2]==0){
    rpm_display[0]=10;
    rpm_display[1]=10;
    rpm_display[2]=10;
  }
  else if (rpm_display[0]==0 && rpm_display[1]==0){
    rpm_display[0]=10;
    rpm_display[1]=10;
  }
  if (rpm_display[0]==0){
    rpm_display[0]=10;
  }
}

void updatePitch(uint16_t pitch_int) {
  pitch_display[0]=pitch_int/1000;
  pitch_display[1]=(pitch_int/100)-(pitch_display[0]*10);
  pitch_display[2]=(pitch_int/10)-(pitch_display[0]*100)-(pitch_display[1]*10);
  pitch_display[3]=pitch_int-(pitch_display[0]*1000)-(pitch_display[1]*100)-(pitch_display[2]*10);
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
