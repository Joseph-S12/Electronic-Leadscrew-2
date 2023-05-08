
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/time.h"
#include "pico/bootrom.h"
#include "stdio.h"
#include "stdint.h"

#include "main.h"
#include "gpio.h"
#include "display.h"
#include "motion.h"
#include "intercore.h"

static void display_loop() {
	// motion_dump_status();
	float x, a;
	motion_get_position(&x, &a);
	gpio_flash_led(333, 166);
	updateRPM((int)round(x * 10.0f));
	updatePitch((int)a);
	printDisplay();
}

static void test_wait_motion() {
	for(int i=0; i<5; ++i) display_loop();

	printf("Stop motion\n");
	motion_stop();

	while(!motion_complete()) display_loop();

	motion_dump_status();
	sleep_ms(500);
}

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
	printf("Initialising Queues\n");
	intercore_init();
	printf("Initialising Display\n");
	initialiseDisplay();
	printf("Launching Core 1\n");
	multicore_launch_core1(motion_main);
	printf("Initialised\n");

	motion_dump_constants();

	gpio_flash_led(500, 500);

	printf("2mm pitch RH metric thread (forwards)\n");
	motion_plan_thread_metric(2.0f);
	motion_plan_direction(true, false);
	motion_run();
	test_wait_motion();

	printf("12TPI LH imperial thread (backwards)\n");
	motion_plan_thread_imperial(12.0f);
	motion_plan_direction(false, true);
	motion_run();
	test_wait_motion();

	printf("40TPI LH imperial thread (backwards)\n");
	motion_plan_thread_imperial(40.0f);
	motion_plan_direction(false, true);
	motion_run();
	test_wait_motion();

	printf("End of program\n");
	gpio_flash_led(1000, 0);

	reset_usb_boot(0, 0);

	/* TODO */
	// Infinite loop
	// Read the switch inputs
	// Get motion status
	// When RUNNING
	//   Check if F/R switch released
	//     Call motion stop
	// When STOPPED
	//   Check forward / reverse and status = STOPPED
	//     Read LH/RH switch
	//     Call motion plan direction indicate F/B L/R
	//     Call motion run
	//   Otherwise
	//     Check metric / imperial switch
	//       Swap out and display current value
	//     Read UP / DOWN
	//       Adjust TPI or mm pitch


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
		sleep_ms(500);
		printDisplay();
	}
}
