#include "pico/stdlib.h"
#include "hardware.h"
#include "gpio.h"

switches_t gpio_switches;

inline static void output_pin(int pin) {
	gpio_init(pin);
	gpio_set_dir(pin, GPIO_OUT);
}

inline static void input_pin(int pin) {
	gpio_init(pin);
	gpio_set_dir(pin, GPIO_IN);
	gpio_pull_down(pin);
}

void gpio_initialise(){
	//Set up all the switch pins
	//Actually moves things
	input_pin(FORWARD_PIN);
	input_pin(REVERSE_PIN);

	//Metric or Imperial thread
	input_pin(METRIC_PIN);
	input_pin(IMPERIAL_PIN);

	//Sets the mode (leadscrew or dividing head)
	input_pin(LEADSCREW_PIN);
	input_pin(DIVIDING_PIN);

	//Cut a left or right handed thread
	input_pin(RIGHT_HAND_PIN);
	input_pin(LEFT_HAND_PIN);

	// Motor pins
	output_pin(STEP_X_DIR_PIN);
	output_pin(STEP_X_PUL_PIN);
	output_pin(STEP_A_DIR_PIN);
	output_pin(STEP_A_PUL_PIN);

	//Display controls
	output_pin(DISPLAY_CLK_PIN);
	output_pin(DISPLAY_SIO_PIN);
	output_pin(DISPLAY_STB_PIN);

	// Onboard LED
	output_pin(LED_PIN);
}

void gpio_read_switches() {
	gpio_switches.forward = gpio_get(FORWARD_PIN);
	gpio_switches.reverse = gpio_get(REVERSE_PIN);
	gpio_switches.metric = gpio_get(METRIC_PIN);
	gpio_switches.imperial = gpio_get(IMPERIAL_PIN);
	gpio_switches.leadscrew = gpio_get(LEADSCREW_PIN);
	gpio_switches.dividing = gpio_get(DIVIDING_PIN);
	gpio_switches.increase = gpio_get(INCREASE_PIN);
	gpio_switches.decrease = gpio_get(DECREASE_PIN);
	gpio_switches.rhand = gpio_get(RIGHT_HAND_PIN);
	gpio_switches.lhand = gpio_get(LEFT_HAND_PIN);
}
