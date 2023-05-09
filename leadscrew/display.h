#include "pico/stdlib.h"

void initialiseDisplay();
void updateStatus(int);
void updateRPM(uint16_t rpm_int, int decimal_position);
void updatePitch(uint16_t pitch_int, int decimal_position);
void printDisplay();
