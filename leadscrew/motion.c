// Motion control module

#include "pico/stdlib.h"
#include "stdlib.h"
#include "math.h"
#include "hardware.h"
#include "motion.h"
#include "stdio.h"

#define X_MM_PER_STEP ((float)X_ADVANCE_MM / ((float)(X_STEPS * X_MICROSTEPS) * X_RATIO))
#define A_DEG_PER_STEP ((float)360 / ((float)(A_STEPS * A_MICROSTEPS) * A_RATIO))

#define IDLE_LOOP_MS 100
#define STEP_US 10
#define LOOP_OFFSET_US (1 + STEP_US)

#define ACCEL_TABLE_MAX_SIZE 800
#define ACCEL_TABLE_MIN_DELAY_US 10

/* Microseconds */
static int accel_delays[ACCEL_TABLE_MAX_SIZE];
static int accel_delays_size = 0; // Size of accel_delays

/* Internal units are whole microsteps */
static int32_t x_pos, a_pos;

static bool run = false;

#define FOLLOWER_UNIT 1000000000
static bool x_leads = false;
static int32_t follower_rate = 0;
static int32_t follower_counter = 0;

static bool x_forward = true, a_forward = true;

static int accel_index = 0;
static int steps_left = 0;

/* Internal prototypes */
static void move();
static void step();
static void plan_accel_table(float accel_time_s, float lead_step_unit, float lead_feedrate_unit_s);

/* Entry points */

/* For debugging */
void motion_dump_status() {
  printf("motion_dump_status()\n");
  printf("  X motion parameters\n");
  printf(
    "    X_ADVANCE_MM=%2.3f X_STEPS=%d X_MICROSTEPS=%d X_RATIO=%1.3f\n",
    (float)X_ADVANCE_MM, (int)X_STEPS, (int)X_MICROSTEPS, (float)X_RATIO
  );
  printf("    X_MM_PER_STEP=%f\n", (float)X_MM_PER_STEP);

  printf("  A motion parameters\n");
  printf(
    "    A_STEPS=%d A_MICROSTEPS=%d A_RATIO=%1.3f\n",
    (int)A_STEPS, (int)A_MICROSTEPS, (float)A_RATIO
  );
  printf("    A_DEG_PER_STEP=%f\n", (float)A_DEG_PER_STEP);

  printf("  Acceleration table (%d entries in us)\n    ", (int)accel_delays_size);
  for(int i = 0; i < accel_delays_size; ++i)
    printf("%d ", (int)accel_delays[i]);
  printf("\n");

  printf("  Current position (microsteps) X=%d A=%d\n", (int)x_pos, (int)a_pos);
  float x, a;
  motion_get_position(&x, &a);
  printf("  Current posiiton X=%1.3f mm; A=%1.3f deg\n", x, a);

  printf(run ? "  RUNNING\n" : "  IDLE\n");
  printf(x_leads ? "  X Leading\n" : "  A Leading\n");

  printf("  Follower registers\n    Rate=%d Count=%dn");
  printf(x_forward ? "  X Forward\n" : "  X Reverse\n");
  printf(a_forward ? "  A Forward\n" : "  A Reverse\n");
}

/* Retrieve coordinates */
void motion_get_position(float *x_mm, float *a_deg) {
  if(x_mm) *x_mm = (float)x_pos * (float)X_MM_PER_STEP;
  if(a_deg) *a_deg = (float)a_pos * (float)A_DEG_PER_STEP;
}

/* Motion core main */
void motion_main(bool loop) {
  while(loop) {
    while(!run) { sleep_ms(IDLE_LOOP_MS); }
    move();
  }
}

/* Motion Planner */
void motion_plan_move(float x1_mm, float a1_deg, float x_feedrate_mm_s, float a_feedrate_deg_s) {
  // Determine end position in native units
  int32_t x1_pos = round(x1_mm / (float)X_MM_PER_STEP);
  int32_t a1_pos = round(a1_deg / (float)A_DEG_PER_STEP);

  // Determine motor directions
  x_forward = x1_pos >= x_pos;
  a_forward = a1_pos >= a_pos;

  // Determine step count for move
  int32_t x_steps = abs(x1_pos - x_pos);
  int32_t a_steps = abs(a1_pos - a_pos);

  // Determine which stepper leads and which follows
  x_leads = x_steps > a_steps;

  // Apply velocity limits
  if(x_feedrate_mm_s > X_MAX_VELO_MM_S) x_feedrate_mm_s = X_MAX_VELO_MM_S;
  if(a_feedrate_deg_s > A_MAX_VELO_DEG_S) a_feedrate_deg_s = A_MAX_VELO_DEG_S;

  // Relate feedrates and limit one of them
  float x_over_a = (float)x_steps / (float)a_steps;
  if((a_feedrate_deg_s * x_over_a) > x_feedrate_mm_s) a_feedrate_deg_s = x_feedrate_mm_s / x_over_a;
  else if(x_feedrate_mm_s > (a_feedrate_deg_s * x_over_a)) x_feedrate_mm_s = a_feedrate_deg_s * x_over_a;

  // Calculate acceleration time per axis and take largest
  float x_accel_time_s = x_feedrate_mm_s / X_MAX_ACCEL_MM_S2;
  float a_accel_time_s = a_feedrate_deg_s / A_MAX_ACCEL_DEG_S2;
  float accel_time_s = x_accel_time_s > a_accel_time_s ? x_accel_time_s : a_accel_time_s;

  // Determine the number of lead steps required to accelerate and follower rate
  if(x_leads) {
    plan_accel_table(accel_time_s, X_MM_PER_STEP, x_feedrate_mm_s);
    follower_rate = round((float)FOLLOWER_UNIT / x_over_a);
    steps_left = x_steps;
  } else {
    plan_accel_table(accel_time_s, A_DEG_PER_STEP, a_feedrate_deg_s);
    follower_rate = round((float)FOLLOWER_UNIT * x_over_a);
    steps_left = a_steps;
  }
}

/* Threaded */
void motion_spiral_move_x(float x1_mm, float t_pitch_mm, float t_degrees) {
  float a_step_per_x_step = (t_degrees / t_pitch_mm) * (X_MM_PER_STEP / A_DEG_PER_STEP);

  float x0_mm, a0_deg;
  motion_get_position(&x0_mm, &a0_deg);

  float a1_deg = a_step_per_x_step * (x1_mm - x0_mm);

  motion_plan_move(x1_mm, a1_deg, 1000000, 1000000);

  run = true;
}

/* Internal functions */

/* Acceleration table computation */
static void plan_accel_table(float accel_time_s, float lead_step_unit, float lead_feedrate_unit_s) {
  float accel_unit_s2 = lead_feedrate_unit_s / accel_time_s;
  float accel_disp_units = 0.5f * accel_unit_s2 * accel_time_s * accel_time_s;
  int accel_steps = round(accel_disp_units / lead_step_unit);

  float time_scale_us = sqrt(lead_step_unit / (2.0f * lead_feedrate_unit_s)) * 1000000.0;

  int i;
  int t_last_us = 0;

  for(i = 0; i < ACCEL_TABLE_MAX_SIZE; ++i) {
    if(i > accel_steps) break;

    int t_us = round(sqrt(i + 1) * time_scale_us);

    int delta_t = t_us - t_last_us - LOOP_OFFSET_US;
    if(delta_t < ACCEL_TABLE_MIN_DELAY_US) break;

    accel_delays[i] = delta_t;
    t_last_us = t_us;
  }

  accel_delays_size = i;
}

/* GPIO functions */

static inline void motor_en(bool en) {
  // gpio_put(STEP_M_EN_PIN, en != STEP_M_EN_INVERT);
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
  follower_counter = FOLLOWER_UNIT / 2;

  /* Set pulse idle */
  x_pulse(false);
  a_pulse(false);

  /* Set directions */
  x_direction(x_forward);
  a_direction(a_forward);

  /* Motors on */
  motor_en(true);

  /* Run up */
  while(accel_index < accel_delays_size && accel_index < steps_left) {
    step();
    sleep_us(accel_delays[accel_index]);
    ++accel_index;
  }

  /* Motion */
  while(steps_left >= accel_index) {
    step();
    sleep_us(accel_delays[accel_index]);
  }

  /* Run down */
  while(steps_left > 0) {
    --accel_index;
    step();
    sleep_us(accel_delays[accel_index]);
  }

  /* Strictly this might not be appropriate */
  motor_en(false);

  run = false;
}

/* Do a motion step */
static void step() {
  follower_counter += follower_rate;
  bool follower_pulse = (follower_counter > FOLLOWER_UNIT);
  if(follower_pulse) follower_counter -= FOLLOWER_UNIT;

  if(x_leads) {
    x_pulse(true);
    a_pulse(follower_pulse);
  } else {
    x_pulse(follower_pulse);
    a_pulse(true);
  }

  sleep_us(STEP_US);

  x_pulse(false);
  a_pulse(false);
}
