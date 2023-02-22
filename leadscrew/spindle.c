#include <stdio.h>

#include "gpio.h"
#include "spindle.h"

volatile uint64_t previousTime;
volatile uint32_t spindleCounter;
volatile uint8_t spindleDirection;

void initSpindle(){
	spindleCounter=0;
	spindleDirection=0;
}

void setSpindleDir(uint8_t direction){
	gpio_put(SPINDLE_DIR_PIN, direction); //0 = Forward, 1 = Backwards
	spindleDirection = direction;
}

void moveSpindle(int rpm){
	uint64_t currentTime = time_us_64();
	uint64_t currentSteps, changeSteps;
	
	currentSteps = ((currentTime * rpm * SPINDLE_NUM_STEPS * SPINDLE_NUM_MICROSTEPS) / 60000000);
	changeSteps = currentSteps - spindleCounter;
	
	if (changeSteps > 0 && changeSteps < 20){
		printf("%lld %lld\n",changeSteps, currentTime-previousTime);
		previousTime=currentTime;
		doSpindleSteps((uint16_t)changeSteps);
	}
	else if (changeSteps >= 20) spindleCounter+=changeSteps;
}

void indexSpindle(uint16_t noDivisions, uint16_t divisionCount){
	uint32_t steps = ((uint32_t) noDivisions * divisionCount) / (SPINDLE_NUM_STEPS * SPINDLE_NUM_MICROSTEPS);
	
	steps = steps - spindleCounter;
	
	for (size_t i = 0; i < steps; i++) {
		gpio_put(SPINDLE_PUL_PIN, 1);
		sleep_us(50);
		gpio_put(SPINDLE_PUL_PIN, 0);
		sleep_us(50);
		spindleCounter++;
  }
}

void doSpindleSteps(uint16_t numSteps) {
	//setDir();
	for (size_t i = 0; i < numSteps; i++) {
		gpio_put(SPINDLE_PUL_PIN, 1);
		sleep_us(50);
		gpio_put(SPINDLE_PUL_PIN, 0);
		sleep_us(50);
		spindleCounter++;
  }
}
