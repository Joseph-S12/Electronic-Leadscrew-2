
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/time.h"
#include "stdio.h"
#include "stdint.h"

#include "main.h"
#include "gpio.h"
#include "leadscrew.h"
#include "spindle.h"
#include "display.h"
#include "motion.h"

//This is the initialisation code for core 0. Everything on this core after initialisation is interrupt driven and deals with the quadrature encoder and driving the stepper.
//It is interrupted by the quadrature encoder pulses.
void main() {
	// int state=0;
	// int rpm=200;
	// int noDivisions=20;
	// int divisionCount=0;
	//Initialise IO
	stdio_init_all();

	printf("Initialising GPIO\n");
	initGPIO0();
	printf("Initialised\n");

	gpio_put(LED_PIN, true);
	sleep_ms(500);
	gpio_put(LED_PIN, false);
	sleep_ms(500);

	while(true) {
		motion_dump_status();
		motion_plan_move(50.0f, 0.0f, 10.0f, 1.0f);
		motion_dump_status();
		gpio_put(LED_PIN, true);
		motion_main(false);
		gpio_put(LED_PIN, false);
		sleep_ms(500);

		motion_dump_status();
		motion_plan_move(50.0f, 360.0f, 1.0f, 45.0f);
		motion_dump_status();
		gpio_put(LED_PIN, true);
		motion_main(false);
		gpio_put(LED_PIN, false);
		sleep_ms(500);

		motion_dump_status();
		motion_plan_move(0.0f, 0.0f, 10.0f, 45.0f);
		motion_dump_status();
		gpio_put(LED_PIN, true);
		motion_main(false);
		gpio_put(LED_PIN, false);
		sleep_ms(500);

		motion_dump_status();
		motion_thread_metric(100.0f, 5.0f, true);
		motion_dump_status();
		gpio_put(LED_PIN, true);
		motion_main(false);
		gpio_put(LED_PIN, false);
		sleep_ms(500);

		motion_dump_status();

		printf("End of program\n");
		gpio_put(LED_PIN, true);
		sleep_ms(900);
		gpio_put(LED_PIN, false);
		sleep_ms(100);
	}
	// initialiseDisplay();
	// initialiseLeadscrew();
	// multicore_launch_core1(&core_1_main);
	//
	// while (true){
	//
	// 	state=getMode();
	// 	//Check Input switch states
	// 	if (gpio_get(LEADSCREW_PIN)){
	// 		updatePitch(getPitch());
	// 		updateRPM(rpm);//Will need speed control here eventually
	//
	// 		if (gpio_get(FORWARD_PIN) || gpio_get(REVERSE_PIN)){
	// 			//Move
	// 			state+=32;
	// 			setSpindleDir(gpio_get(REVERSE_PIN));
	// 			moveSpindle(rpm);
	// 			// doSpindleSteps(1);
	// 			// sleep_us(300);
	// 			doLeadscrewPulse();
	// 		}
	// 		else {
	// 			resetCounters();
	// 			if (gpio_get(METRIC_PIN) || gpio_get(IMPERIAL_PIN)){
	// 				if (gpio_get(INCREASE_PIN)){
	// 					if (pitch_1000<2500) pitch_1000+=50;
	// 					while (gpio_get(INCREASE_PIN)) sleep_ms(5);
	// 				}
	// 				if (gpio_get(DECREASE_PIN)){
	// 					if (pitch_1000>50) rpm-=50;
	// 					while (gpio_get(DECREASE_PIN)) sleep_ms(5);
	// 				}
	// 			}
	// 			else{
	// 				if (gpio_get(INCREASE_PIN)){
	// 					if (rpm<1000) rpm+=50;
	// 					while (gpio_get(INCREASE_PIN)) sleep_ms(5);
	// 				}
	// 				if (gpio_get(DECREASE_PIN)){
	// 					if (rpm>50) rpm-=50;
	// 					while (gpio_get(DECREASE_PIN)) sleep_ms(5);
	// 				}
	// 			}
	// 		}
	// 	}
	// 	else if (gpio_get(DIVIDING_PIN)){
	// 		updatePitch(1000);
	// 		updateRPM(noDivisions);//This is Number of divisions
	// 		if (gpio_get(FORWARD_PIN)){
	// 			divisionCount++;
	// 			indexSpindle(noDivisions, divisionCount);
	// 			sleep_ms(10);
	// 			while (gpio_get(FORWARD_PIN)) sleep_ms(5);
	// 		}
	// 		else if (gpio_get(REVERSE_PIN)){
	// 			divisionCount--;
	// 			indexSpindle(noDivisions, divisionCount);
	// 			sleep_ms(10);
	// 			while (gpio_get(REVERSE_PIN)) sleep_ms(5);
	// 		}
	// 	}
	// 	else{
	// 		resetCounters();
	// 		updatePitch(0000);
	// 		updateRPM(0000);
	// 	}
	// 	updateStatus(state);
	//
	// }
}

void core_1_main(){
	while (true){
		sleep_ms(1000);
		printDisplay();
	}
}
