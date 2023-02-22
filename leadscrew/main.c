#include "main.h"
#include "gpio.h"
#include "gpio.c"
#include "leadscrew.h"
#include "leadscrew.c"
#include "spindle.h"
#include "spindle.c"
#include "display.h"
#include "display.c"

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/time.h"
#include <stdio.h>

//This is the initialisation code for core 0. Everything on this core after initialisation is interrupt driven and deals with the quadrature encoder and driving the stepper.
//It is interrupted by the quadrature encoder pulses.
void main() {
	int state=0;
	int rpm=250;
	int noDivisions=20;
	//Initialise IO
	stdio_init_all();
	initGPIO0();
	initialiseDisplay();
	initialiseLeadscrew();
	//multicore_launch_core1(&core_1_main);
	
	while (true){ 
		
		state=getMode();
		//Check Input switch states
		if (gpio_get(LEADSCREW_PIN)){
			updatePitch(getPitch());
			updateRPM(rpm);//Will need speed control here eventually
			
			if (gpio_get(FORWARD_PIN) || gpio_get(REVERSE_PIN)){
				//Move
				state+=32;
				setSpindleDir(gpio_get(REVERSE_PIN));
				// moveSpindle(rpm);
				doSpindleSteps(1);
				sleep_us(300);
				doLeadscrewPulse();
			}
		}
		else if (gpio_get(DIVIDING_PIN)){
			updatePitch(1000);
			updateRPM(noDivisions);//This is Number of divisions
		}
		else{
			updatePitch(0000);
			updateRPM(0000);
		}
		updateStatus(state);
		
	}
}

void core_1_main(){
	while (true){
		sleep_ms(1000);
		printDisplay();
	}
}
