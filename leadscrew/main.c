
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

static void test_wait_motion() {
	motion_dump_status();
	while(!motion_complete()) {
		gpio_flash_led(333, 166);
		motion_dump_status();
		float x, a;
		motion_get_position(&x, &a);
		updateRPM((int)round(x * 10.0f));
		updatePitch((int)a);
		printDisplay();
	};
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

	printf("Move to X30 A0\n");
	motion_plan_move(30.0f, 0.0f, 10.0f, 1.0f);
	test_wait_motion();

	// About 20 steps more
	printf("Move to X30.015 A0\n");
	motion_plan_move(30.015f, 0.0f, 10.0f, 1.0f);
	test_wait_motion();

	printf("Move to X50 A360\n");
	motion_plan_move(30.0f, 360.0f, 1.0f, 360.0f);
	test_wait_motion();

	printf("Move to X0 A0\n");
	motion_plan_move(0.0f, 0.0f, 10.0f, 360.0f);
	test_wait_motion();

	printf("25mm long LH thread pitch 2mm\n");
	motion_thread_metric(25.0f, 2.0f, true);
	test_wait_motion();

	printf("Move to X0 A0\n");
	motion_plan_move(0.0f, 0.0f, 10.0f, 360.0f);
	test_wait_motion();

	printf("End of program\n");
	gpio_flash_led(1000, 0);

	reset_usb_boot(0, 0);

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
