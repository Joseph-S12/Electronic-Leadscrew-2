#ifndef LEADSCREW_NUM_STEPS
#define LEADSCREW_NUM_STEPS 200
#endif

#ifndef LEADSCREW_NUM_MICROSTEPS
#define LEADSCREW_NUM_MICROSTEPS 4
#endif

#ifndef LEADSCREW_PITCH_1000
#define LEADSCREW_PITCH_1000 1500
#endif

void initialiseLeadscrew();
void resetCounters();
void setLeadscrewPitch(uint16_t);
void setLeadscrewReverse(bool);
volatile uint8_t checkDir();
void doLeadscrewPulse();
void doLeadscrewSteps(uint16_t);
uint16_t getPitch();
void setPitch(uint16_t);

extern volatile uint16_t pitch_1000;
