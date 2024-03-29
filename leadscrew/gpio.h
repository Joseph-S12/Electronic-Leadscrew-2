#include "pico/stdlib.h"

// TODO
// Switch arrangement
// Run:
// FORWARD < SET-UP > REVERSE

// Mode selection:
// THREAD L < NEXT > THREAD R
// LINEAR  < DIVIDE > POWERFEED

// Parameter selection:
// IMPERIAL < RATE > METRIC

// Parameter change:
// INCREMENT < -- > DECREMENT

// LED Indication (mode / param):
// LH RH LIN PWR DIV
//               CNT IMP RAT MET

#ifndef FORWARD_PIN
#define FORWARD_PIN 0   		//Input, Pin 0
#define REVERSE_PIN 1   		//Input, Pin 1

#define METRIC_PIN 2        	//Input, Pin 3
#define IMPERIAL_PIN 3      	//Input, Pin 4
#define LEADSCREW_PIN 4     	//Input, Pin 5
#define DIVIDING_PIN 5     		//Input, Pin 6

#define INCREASE_PIN 6      	//Input, Pin 8
#define DECREASE_PIN 7      	//Input, Pin 9

#define RIGHT_HAND_PIN 8    	//Input, Pin 10
#define LEFT_HAND_PIN 9     	//Input, Pin 11

#define LED_PIN 25

#define DISPLAY_CLK_PIN 18   	//Output, Pin 24
#define DISPLAY_SIO_PIN 19   	//Output, Pin 25
#define DISPLAY_STB_PIN 20   	//Output, Pin 26
#endif

void gpio_initialise();
void gpio_read_switches();

inline static void gpio_flash_led(int on, int off) {
  gpio_put(LED_PIN, true);
  sleep_ms(on);
  gpio_put(LED_PIN, false);
  sleep_ms(off);
}

typedef struct {
  uint8_t forward : 1;
  uint8_t reverse : 1;

  uint8_t metric : 1;
  uint8_t imperial : 1;

  uint8_t leadscrew : 1;
  uint8_t dividing : 1;

  uint8_t increase : 1;
  uint8_t decrease : 1;

  uint8_t rhand : 1;
  uint8_t lhand : 1;
} switches_t;

extern switches_t gpio_switches;
