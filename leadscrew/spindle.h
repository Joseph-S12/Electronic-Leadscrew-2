
#ifndef SPINDLE_NUM_STEPS
#define SPINDLE_NUM_STEPS 200
#endif

#ifndef SPINDLE_NUM_MICROSTEPS
#define SPINDLE_NUM_MICROSTEPS 4
#endif

extern volatile uint32_t spindleCounter;
void setSpindleDir(uint8_t);
void doSpindleSteps(uint16_t);
void indexSpindle(uint16_t, uint16_t);
