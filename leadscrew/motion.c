// Motion control module

#include "pico/stdlib.h"
#include "stdlib.h"
#include "math.h"
#include "hardware.h"
#include "motion.h"
#include "stdio.h"
#include "intercore.h"

#define X_MM_PER_STEP ((float)X_ADVANCE_MM / ((float)(X_STEPS * X_MICROSTEPS) * X_RATIO))
#define A_DEG_PER_STEP ((float)360 / ((float)(A_STEPS * A_MICROSTEPS) * A_RATIO))

#define IDLE_LOOP_MS 100
#define STEP_US 10
#define LOOP_OFFSET_US (1 + STEP_US)

#define ACCEL_TABLE_MAX_SIZE 2000
#define ACCEL_TABLE_MIN_DELAY_US 10

/* Microseconds */
static int accel_delays[ACCEL_TABLE_MAX_SIZE];
static int accel_delays_size = 0; // Size of accel_delays

/* Internal units are whole microsteps */
static int32_t x_pos, a_pos;

static int status = STATUS_STOPPED;

#define FOLLOWER_UNIT 1000000000
static bool x_leads = false;
static int32_t follower_rate = 0;
static int32_t follower_counter = 0;

static bool x_forward = true, a_forward = true;

static int steps_left = 0;

/* Internal prototypes */
static void move();
static void step();
static void handle_commands();
static void plan_accel_table(float accel_time_s, float lead_step_unit, float lead_feedrate_unit_s);

/* Entry points */

/* For debugging */
void motion_dump_constants() {
  printf("Motion constants\n");
  printf("  X motion parameters:\t");
  printf(
    "X_ADVANCE_MM=%2.3f X_STEPS=%d X_MICROSTEPS=%d X_RATIO=%1.3f ",
    (float)X_ADVANCE_MM, (int)X_STEPS, (int)X_MICROSTEPS, (float)X_RATIO
  );
  printf("X_MM_PER_STEP=%f\n", (float)X_MM_PER_STEP);

  printf("  A motion parameters:\t");
  printf(
    "A_STEPS=%d A_MICROSTEPS=%d A_RATIO=%1.3f ",
    (int)A_STEPS, (int)A_MICROSTEPS, (float)A_RATIO
  );
  printf("A_DEG_PER_STEP=%f\n", (float)A_DEG_PER_STEP);
}

void motion_dump_status() {
  printf("Motion status\n");

  response_t response;
  intercore_command(CMD_Q_STATUS, &response);

  printf("  Current position (microsteps): X=%d A=%d\t", (int)response.x_pos, (int)response.a_pos);
  printf(
    "X=%1.3f mm; A=%1.3f deg\n",
    (float)response.x_pos * X_MM_PER_STEP,
    (float)response.a_pos * A_DEG_PER_STEP
  );

  printf("  Acceleration table (%d entries in us):\t", (int)accel_delays_size);
#ifdef MOTION_DEBUG
  for(int i = 0; i < accel_delays_size; ++i)
    printf("%5d%c", (int)accel_delays[i], (i % 20) ? ' ' : '\n');
#else
  printf("%5d ... %5d", (int)accel_delays[0], (int)accel_delays[accel_delays_size - 1]);
#endif
  printf("\n");

  const char *status_str;
  switch(response.status) {
    case STATUS_STOPPED: status_str = "STOPPED"; break;
    case STATUS_RUN: status_str = "RUNNING"; break;
    case STATUS_STOPPING: status_str = "STOPPING"; break;
    case STATUS_ESTOPPED: status_str = "ESTOPPED"; break;
  }

  printf(
    "  Status: %s\t%s Leading\tX %s\tA %s",
    status_str ? "RUN" : "IDLE", x_leads ? "X" : "A",
    x_forward ? "Forward" : "Reverse", a_forward ? "Forward" : "Reverse"
  );

  printf("  Steps left:\t%d\n", response.steps_left);
  printf("  Follower rate=%d\n", follower_rate);
}

/* Retrieve coordinates */
void motion_get_position(float *x_mm, float *a_deg) {
  response_t response;
  intercore_command(CMD_Q_STATUS, &response);

  if(x_mm) *x_mm = (float)response.x_pos * (float)X_MM_PER_STEP;
  if(a_deg) *a_deg = (float)response.a_pos * (float)A_DEG_PER_STEP;
}

bool motion_complete() {
  response_t response;
  intercore_command(CMD_Q_STATUS, &response);

  return response.status == STATUS_STOPPED;
}

/* Motion Planner */
void motion_plan_move(float x1_mm, float a1_deg, float x_feedrate_mm_s, float a_feedrate_deg_s) {
  response_t response;
  intercore_command(CMD_Q_STATUS, &response);
  if(response.status != STATUS_STOPPED)
  {
    printf("MOTION PLAN WHILE NOT IDLE - ESTOP");
    intercore_command(CMD_ESTOP, &response);
  }

  printf("Plan move to X %2.3f mm, A %2.3f deg at %2.3f mm/s, %2.3f deg/s max \n", x1_mm, a1_deg, x_feedrate_mm_s, a_feedrate_deg_s);

  // Determine end position in native units
  int32_t x1_pos = round(x1_mm / (float)X_MM_PER_STEP);
  int32_t a1_pos = round(a1_deg / (float)A_DEG_PER_STEP);

  // Determine motor directions
  x_forward = x1_pos >= response.x_pos;
  a_forward = a1_pos >= response.a_pos;

  // Determine step count for move
  int32_t x_steps = abs(x1_pos - response.x_pos);
  int32_t a_steps = abs(a1_pos - response.a_pos);

  // Determine which stepper leads and which follows
  x_leads = x_steps > a_steps;

  // Apply velocity limits
  if(x_feedrate_mm_s > X_MAX_VELO_MM_S) x_feedrate_mm_s = X_MAX_VELO_MM_S;
  if(a_feedrate_deg_s > A_MAX_VELO_DEG_S) a_feedrate_deg_s = A_MAX_VELO_DEG_S;

  // Relate feedrates and limit one of them
  float a_feedrate__x_steps = a_feedrate_deg_s * (float)x_steps;
  float x_feedrate__a_steps = x_feedrate_mm_s * (float)a_steps;
  if(a_feedrate__x_steps > x_feedrate__a_steps) a_feedrate_deg_s = x_feedrate__a_steps / (float)x_steps;
  else if(x_feedrate__a_steps > a_feedrate__x_steps) x_feedrate_mm_s = a_feedrate__x_steps / (float)a_steps;

  // Calculate acceleration time per axis and take largest
  float x_accel_time_s = x_feedrate_mm_s / X_MAX_ACCEL_MM_S2;
  float a_accel_time_s = a_feedrate_deg_s / A_MAX_ACCEL_DEG_S2;
  float accel_time_s = x_accel_time_s > a_accel_time_s ? x_accel_time_s : a_accel_time_s;

  // Determine the number of lead steps required to accelerate and follower rate
  if(x_leads) {
    plan_accel_table(accel_time_s, X_MM_PER_STEP, x_feedrate_mm_s);
    follower_rate = round((float)FOLLOWER_UNIT * (float)a_steps / (float)x_steps);
    steps_left = x_steps;
  } else {
    plan_accel_table(accel_time_s, A_DEG_PER_STEP, a_feedrate_deg_s);
    follower_rate = round((float)FOLLOWER_UNIT * (float)x_steps / (float)a_steps);
    steps_left = a_steps;
  }

  intercore_command(CMD_RUN, &response);
}

/* Threaded */
void motion_spiral_move_x(float x1_mm, float t_pitch_mm, float t_degrees) {
  float x0_mm, a0_deg;
  motion_get_position(&x0_mm, &a0_deg);

  float a1_deg = (t_degrees / t_pitch_mm) * (x1_mm - x0_mm);

  motion_plan_move(x1_mm, a1_deg, 1000000, 1000000);
}

/* Motion core main */
void motion_main() {
  while(true) {
    if(status == STATUS_RUN) move();
    else {
      sleep_ms(IDLE_LOOP_MS);
      handle_commands();
    }
  }
}

/* Internal functions */

/* Acceleration table computation */
static void plan_accel_table(float accel_time_s, float lead_step_unit, float lead_feedrate_unit_s) {
  printf("Plan acceleration over %2.3f with step size %2.3f u/step %2.3f u/s\n", accel_time_s, lead_step_unit, lead_feedrate_unit_s);

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
  if(step) x_forward ? ++x_pos : --x_pos;
}

static inline void a_pulse(bool step) {
  gpio_put(STEP_A_PUL_PIN, step != STEP_A_PUL_INVERT);
  if(step) a_forward ? ++a_pos : --a_pos;
}

static inline void x_direction(bool forward) {
  gpio_put(STEP_X_DIR_PIN, forward != STEP_X_DIR_INVERT);
}

static inline void a_direction(bool forward) {
  gpio_put(STEP_A_DIR_PIN, forward != STEP_A_DIR_INVERT);
}

static void move() {
  int accel_index = 0;
  follower_counter = FOLLOWER_UNIT / 2;

  /* Enable motors */
  motor_en(true);

  /* Set pulse idle */
  x_pulse(false);
  a_pulse(false);

  /* Set directions */
  x_direction(x_forward);
  a_direction(a_forward);

  while(steps_left > 0 && status != STATUS_STOPPED) {
#ifdef MOTION_DEBUG
    printf("n=%6d i=%4d T=%5d\t", steps_left, accel_index, accel_delays[accel_index]);
    if(!(steps_left & 0x7)) printf("\n");
#endif
    step();
    handle_commands();
    if(status == STATUS_ESTOPPED) return;

    sleep_us(accel_delays[accel_index]);

    if(accel_index < (accel_delays_size - 1) && accel_index < steps_left)
      ++accel_index;

    if(steps_left < accel_index) status = STATUS_STOPPING;

    if(status == STATUS_STOPPING)
      --accel_index;

    if(accel_index == 0) status = STATUS_STOPPED;
  }

#ifdef MOTION_DEBUG
  printf("n=%d i=%d\n", steps_left, accel_index);
#endif
}

/* Do a motion step */
static void step() {
  --steps_left;

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

static void handle_commands() {
  command_t command;
  response_t response;

  if(intercore_getcommand_nb(&command)) {
    switch(command.cmd) {
    case CMD_ESTOP:
      motor_en(false);
      status = STATUS_ESTOPPED;
      response.result = RESULT_OK;
      break;

    case CMD_Q_STATUS:
      response.result = RESULT_OK;
      break;

    case CMD_RUN:
      if(status == STATUS_STOPPED || status == STATUS_STOPPING)
        status = STATUS_RUN;
      response.result = RESULT_OK;
      break;

    case CMD_STOP:
      if(status == STATUS_RUN) status = STATUS_STOPPING;
      response.result = RESULT_OK;
      break;
    }

    response.status = status;
    response.x_pos = x_pos;
    response.a_pos = a_pos;
    response.steps_left = steps_left;

    intercore_respond_nb(&response);
  }
}
