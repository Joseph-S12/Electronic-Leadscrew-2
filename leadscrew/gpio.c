#include "pico/stdlib.h"
#include "hardware.h"
#include "gpio.h"

inline static void output_pin(int pin) {
	gpio_init(pin);
	gpio_set_dir(pin, GPIO_OUT);
}

inline static void input_pin(int pin) {
	gpio_init(pin);
	gpio_set_dir(pin, GPIO_IN);
	gpio_pull_down(pin);
}

void initGPIO0(){
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

int getMode(){
	//0 = Off
	//1 = mm pitch
	//2 = TPI
	//4 = Dividing Head Mode
	//8 =
	//16 = invert direction
	//32 = is moving

	int status = 0;

	if (gpio_get(LEADSCREW_PIN)){
		if (gpio_get(METRIC_PIN)) 			status+=1;
		else if (gpio_get(IMPERIAL_PIN))	status+=2;
		if (gpio_get(RIGHT_HAND_PIN)
			&& gpio_get(REVERSE_PIN))		status+=16;
		else if (gpio_get(LEFT_HAND_PIN)
			&& !(gpio_get(REVERSE_PIN)))	status+=16;
	}
	else if (gpio_get(DIVIDING_PIN)){
		status+=4;
	}

	return status;
}
