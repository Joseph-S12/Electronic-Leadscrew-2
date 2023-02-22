#include "pico/stdlib.h"

extern void initialiseDisplay();
extern void updateStatus(int);
extern void updateRPM(uint16_t rpm_int);
extern void updatePitch(uint16_t pitch_int);
extern void printDisplay();
