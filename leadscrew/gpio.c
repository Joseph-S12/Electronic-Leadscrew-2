#include "gpio.h"

#include "pico/stdlib.h"
#include <stdio.h>

void initGPIO0(){
	//Set up all the switch pins
	//Actually moves things
	gpio_init(FORWARD_PIN);
	gpio_set_dir(FORWARD_PIN, GPIO_IN);
	gpio_pull_down(FORWARD_PIN);
	
	gpio_init(REVERSE_PIN);
	gpio_set_dir(REVERSE_PIN, GPIO_IN);
	gpio_pull_down(REVERSE_PIN);
	
	//Metric or Imperial thread
	gpio_init(METRIC_PIN);
	gpio_set_dir(METRIC_PIN, GPIO_IN);
	gpio_pull_down(METRIC_PIN);
	
	gpio_init(IMPERIAL_PIN);
	gpio_set_dir(IMPERIAL_PIN, GPIO_IN);
	gpio_pull_down(IMPERIAL_PIN);
	
	//Sets the mode (leadscrew or dividing head)
	gpio_init(LEADSCREW_PIN);
	gpio_set_dir(LEADSCREW_PIN, GPIO_IN);
	gpio_pull_down(LEADSCREW_PIN);
	
	gpio_init(DIVIDING_PIN);
	gpio_set_dir(DIVIDING_PIN, GPIO_IN);
	gpio_pull_down(DIVIDING_PIN);
	
	//Cut a left or right handed thread
	gpio_init(RIGHT_HAND_PIN);
	gpio_set_dir(RIGHT_HAND_PIN, GPIO_IN);
	gpio_pull_down(RIGHT_HAND_PIN);
	
	gpio_init(LEFT_HAND_PIN);
	gpio_set_dir(LEFT_HAND_PIN, GPIO_IN);
	gpio_pull_down(LEFT_HAND_PIN);
	
	//Leadscrew Drive Pins
	gpio_init(LEADSCREW_DIR_PIN);
	gpio_set_dir(LEADSCREW_DIR_PIN, GPIO_OUT);
	
	gpio_init(LEADSCREW_PUL_PIN);
	gpio_set_dir(LEADSCREW_PUL_PIN, GPIO_OUT);
	
	//Leadscrew Drive Pins
	gpio_init(SPINDLE_DIR_PIN);
	gpio_set_dir(SPINDLE_DIR_PIN, GPIO_OUT);
	
	gpio_init(SPINDLE_PUL_PIN);
	gpio_set_dir(SPINDLE_PUL_PIN, GPIO_OUT);
	
	//Display controls
	gpio_init(DISPLAY_CLK_PIN);
	gpio_set_dir(DISPLAY_CLK_PIN, GPIO_OUT);

	gpio_init(DISPLAY_SIO_PIN);
	gpio_set_dir(DISPLAY_SIO_PIN, GPIO_OUT);

	gpio_init(DISPLAY_STB_PIN);
	gpio_set_dir(DISPLAY_STB_PIN, GPIO_OUT);
	
	gpio_init(25);
	gpio_set_dir(25, GPIO_OUT);
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
