
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/time.h"
#include "pico/bootrom.h"
#include "stdio.h"
#include "stdint.h"
#include "math.h"

#include "main.h"
#include "gpio.h"
#include "display.h"
#include "motion.h"
#include "intercore.h"

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
	gpio_initialise();
	printf("Initialising Queues\n");
	intercore_init();
	printf("Initialising Display\n");
	initialiseDisplay();
	printf("Launching Core 1\n");
	multicore_launch_core1(motion_main);
	printf("Initialised\n");

	motion_dump_constants();

	gpio_flash_led(500, 500);

	float this_a, last_a = 0.0f;
	bool forward = true;

	int pitch_mm_100 = 100;
	int pitch_tpi_10 = 80;
	while(true) {
		sleep_ms(100);

		gpio_read_switches();
		motion_update_status();

		last_a = this_a;
		motion_get_position(0, &this_a);

		uint8_t led_status = 0;
		switch(intercore_response.status) {
		case STATUS_RUN:
			led_status |= 4;
			if(!(forward ? gpio_switches.forward : gpio_switches.reverse))
				motion_stop();
			break;

		case STATUS_STOPPED:
			led_status |= 1;
			if(gpio_switches.forward || gpio_switches.reverse) {
				forward = gpio_switches.forward;
				if(!forward) led_status |= 64;
				if(gpio_switches.lhand) {
					led_status |= 32;
					motion_plan_direction(forward, true);
					motion_run();
				} else if(gpio_switches.rhand) {
					motion_plan_direction(forward, false);
					motion_run();
				}
			} else {
				if(gpio_switches.metric) {
					led_status |= 16;
					if(gpio_switches.increase) pitch_mm_100 += 5;
					if(gpio_switches.decrease) pitch_mm_100 -= 5;
					if(pitch_mm_100 < 20) pitch_mm_100 = 20;
					if(pitch_mm_100 > 2500) pitch_mm_100 = 2500;

					motion_plan_thread_metric((float)pitch_mm_100 / 100.0f);
					updatePitch(pitch_mm_100, 2);
				} else if(gpio_switches.imperial) {
					led_status |= 8;

					if(gpio_switches.increase) pitch_tpi_10 += 5;
					if(gpio_switches.decrease) pitch_tpi_10 -= 5;
					if(pitch_tpi_10 < 10) pitch_tpi_10 = 10;
					if(pitch_tpi_10 > 800) pitch_tpi_10 = 800;

					motion_plan_thread_imperial((float)pitch_tpi_10 / 10.0f);
					updatePitch(pitch_tpi_10, 1);
				}
			}
			break;

		case STATUS_STOPPING:
			led_status |= 2;
			if((forward ? gpio_switches.forward : gpio_switches.reverse))
				motion_run();
			break;

		case STATUS_ESTOPPED:
			led_status |= 7;
			updatePitch(-1, -1);
			updateRPM(-1, -1);
		  break;
		}

		updateStatus(led_status);
		updateRPM(60.0f / 360.0f * (this_a > last_a) ? (this_a - last_a) : (last_a - this_a), 0);
		printDisplay();
	}
}

void core_1_main(){
	while (true){
		sleep_ms(500);
		printDisplay();
	}
}
