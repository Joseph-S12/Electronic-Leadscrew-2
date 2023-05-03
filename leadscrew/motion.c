// Motion control module

#include "stdlib.h"
#include <math.h>
#include "hardware.h"

#define X_MM_PER_STEP ((float)X_ADVANCE_MM / ((float)(X_STEPS * X_MICROSTEPS) * X_RATIO))
#define A_DEG_PER_STEP ((float)360 / ((float)(A_STEPS * A_MICROSTEPS) * A_RATIO))

#define X_AXIS_FRACTION_BITS 20
#define A_AXIS_FRACTION_BITS 20

#define X_MM_PER_UNIT ((float)X_MM_PER_STEP * (float)(1L << X_AXIS_FRACTION_BITS))
#define A_DEG_PER_UNIT ((float)A_DEG_PER_STEP * (float)(1L << A_AXIS_FRACTION_BITS))

#define IDLE_LOOP_MS 100
#define STEP_US 10

/* Microseconds */
static int accel_delays[] = {
  10000, 8000, 6400, 5000, 4000, 3200, 2500, 2000, 1600, 1250,
  1000, 800, 640, 500, 400, 320, 250, 200, 160, 125,
  100, 80, 64, 50
};
static int accel_limit = 24; // Size of accel_delays

/* Internal units are in fractional steps */
/* NB: x_rate and y_rate must both be <= 1 full step */
static int64_t x_pos = 0, x_rate = 0;
static int64_t a_pos = 0, a_rate = 0;

static bool run = false;

static int accel_index = 0;
static int n_segs = 0;

/* Internal prototypes */
static void move();
static void step();

/* Entry points */
void motion_get_position(float *x, float *a) {
  *x = (float)x_pos * X_MM_PER_UNIT;
  *a = (float)a_pos * A_DEG_PER_UNIT;
}

/* Motion core main */
void motion_main() {
  while(true) {
    while(!run) { sleep_ms(IDLE_LOOP_MS); }
    move();
  }
}

/* Threaded */
void motion_spiral_move_x(float x1_mm, float t_pitch_mm, float t_degrees, bool lefthanded) {
  int32_t x1 = round(x1_mm / X_MM_PER_UNIT);

  float a_step_per_x_step = (t_degrees / t_pitch_mm) * (X_MM_PER_STEP / A_DEG_PER_STEP);

  if(a_step_per_x_step >= 1.0) {
    a_rate = 1 << A_AXIS_FRACTION_BITS;
    x_rate = round((float)(1 << X_AXIS_FRACTION_BITS) / a_step_per_x_step);
  } else {
    x_rate = 1 << X_AXIS_FRACTION_BITS;
    a_rate = round((float)(1 << A_AXIS_FRACTION_BITS) * a_step_per_x_step);
  }

  n_segs = round((float)abs(x1 - x_pos) / (float)x_rate);

  if(lefthanded) a_rate = -a_rate;

  if(x1 < x_pos) {
    x_rate = -x_rate;
    a_rate = -a_rate;
  }

  run = true;
}

/* Internal functions */

/* GPIO functions */

static inline void motor_en(bool en) {
  gpio_put(STEP_M_EN_PIN, en != STEP_M_EN_INVERT);
}

static inline void x_pulse(bool step) {
  gpio_put(STEP_X_PUL_PIN, step != STEP_X_PUL_INVERT);
}

static inline void a_pulse(bool step) {
  gpio_put(STEP_A_PUL_PIN, step != STEP_A_PUL_INVERT);
}

static inline void x_direction(bool forward) {
  gpio_put(STEP_X_DIR_PIN, forward != STEP_X_DIR_INVERT);
}

static inline void a_direction(bool forward) {
  gpio_put(STEP_A_DIR_PIN, forward != STEP_A_DIR_INVERT);
}

static void move() {
  accel_index = 0;

  /* Set pulse idle */
  x_pulse(false);
  a_pulse(false);

  /* Set directions */
  x_direction(x_rate > 0);
  a_direction(a_rate > 0);

  /* Motors on */
  motor_en(true);

  /* Run up */
  while(accel_index < accel_limit && accel_index < n_segs) {
    step();
    sleep_us(accel_delays[accel_index++]);
  }

  /* Motion */
  while(n_segs >= accel_index) {
    step();
    sleep_us(accel_delays[accel_index]);
  }

  /* Run down */
  while(n_segs > 0) {
    step();
    sleep_us(accel_delays[--accel_index]);
  }

  /* Strictly this might not be appropriate */
  motor_en(false);

  run = false;
}

/* Do a motion step */
static void step() {
  int x0 = x_pos >> X_AXIS_FRACTION_BITS;
  int a0 = a_pos >> A_AXIS_FRACTION_BITS;

  x_pos += x_rate;
  a_pos += a_rate;

  int x1 = x_pos >> X_AXIS_FRACTION_BITS;
  int a1 = a_pos >> A_AXIS_FRACTION_BITS;

  x_pulse(x1 != x0);
  a_pulse(a1 != a0);

  --n_segs;

  sleep_us(STEP_US);

  x_pulse(false);
  a_pulse(false);
}
