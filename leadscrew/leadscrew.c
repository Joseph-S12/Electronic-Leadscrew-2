#include "pico/time.h"
#include "pico/float.h"
#include "pico/stdlib.h"
#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"

#include "leadscrew.h"
#include "spindle.h"
#include "gpio.h"


volatile uint32_t leadscrewCounter;
volatile int diff;

volatile uint16_t pitch_1000; //pitch multiplied by 1000
volatile bool reverse;
volatile bool leadscrew_direction_set;

void initialiseLeadscrew(){
	leadscrew_direction_set = 0;
	spindleCounter=0;
	leadscrewCounter=0;
	pitch_1000=750;
	reverse=0;
}

void resetCounters(){
	spindleCounter = 0;
	leadscrewCounter = 0;
}

void setLeadscrewPitch(uint16_t pitch){
	pitch_1000=pitch;
}

void setLeadscrewReverse(bool reverseThread){
	reverse=reverseThread;
}

volatile uint8_t checkDir() {
	return leadscrew_direction_set;
}

void doLeadscrewPulse(){
	uint32_t step;
	uint32_t currentSpindleCounter=spindleCounter;
	//Do calculations for the current desired step
	step =(uint32_t) ((LEADSCREW_NUM_STEPS * LEADSCREW_NUM_MICROSTEPS  * pitch_1000 * (uint64_t) currentSpindleCounter) /
			(SPINDLE_NUM_STEPS * SPINDLE_NUM_MICROSTEPS * LEADSCREW_PITCH_1000)) ;

	//Calculates how many steps need to be performed

	step = step - leadscrewCounter;
	leadscrewCounter+=step;
	// printf("%ld, %ld\n", step, leadscrewCounter);

	if ((gpio_get(REVERSE_PIN) && gpio_get(LEFT_HAND_PIN)) || (gpio_get(FORWARD_PIN) && gpio_get(RIGHT_HAND_PIN))){
		leadscrew_direction_set=0;
	}
	else {
		leadscrew_direction_set=1;
	}

	gpio_put(LEADSCREW_DIR_PIN,leadscrew_direction_set);
	printf("%ld %ld\n",step, currentSpindleCounter);
	if (abs(step)>0 && abs(step)<40) doLeadscrewSteps((uint16_t) abs(step));
}

void doLeadscrewSteps(uint16_t numSteps) {
	//setDir();
	for (size_t i = 0; i < numSteps; i++) {
		gpio_put(LEADSCREW_PUL_PIN,1);
		sleep_us(40);
		gpio_put(LEADSCREW_PUL_PIN,0);
		sleep_us(40);
	}
}

uint16_t getPitch(){
	return pitch_1000;
}

void setPitch(uint16_t pitch){
	pitch_1000=pitch;
}
