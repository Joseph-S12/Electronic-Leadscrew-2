#ifndef LEADSCREW_NUM_STEPS
#define LEADSCREW_NUM_STEPS 200
#endif

#ifndef LEADSCREW_NUM_MICROSTEPS
#define LEADSCREW_NUM_MICROSTEPS 4
#endif

#ifndef LEADSCREW_PITCH_1000
#define LEADSCREW_PITCH_1000 1500
#endif

extern void initialiseLeadscrew();
extern void resetCounters();
extern void setLeadscrewPitch(uint16_t);
extern void setLeadscrewReverse(bool);
extern volatile uint8_t checkDir();
extern void doLeadscrewPulse();
extern void doLeadscrewSteps(uint16_t);
extern uint16_t getPitch();
extern void setPitch(uint16_t);
