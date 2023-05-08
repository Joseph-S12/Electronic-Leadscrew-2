#include "pico/stdlib.h"


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

extern void initGPIO0();
extern int getMode();

inline static void gpio_flash_led(int on, int off) {
  gpio_put(LED_PIN, true);
  sleep_ms(on);
  gpio_put(LED_PIN, false);
  sleep_ms(off);
}
