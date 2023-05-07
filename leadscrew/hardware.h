// Hardware parameters

/* Motion parameters */
#define X_ADVANCE_MM 1.5f
#define X_STEPS 200
#define X_MICROSTEPS 4
#define X_RATIO (2.0f / 1.0f)

#define A_STEPS 200
#define A_MICROSTEPS 4
#define A_RATIO (2.0f / 1.0f)

/* Pin configuration */

/* Motor enable */
#define STEP_M_EN_PIN 999
#define STEP_M_EN_INVERT false

/* Pulse pins */
#define STEP_X_PUL_PIN 12
#define STEP_A_PUL_PIN 15
#define STEP_X_PUL_INVERT false
#define STEP_A_PUL_INVERT false

/* Direction pins */
#define STEP_X_DIR_PIN 11
#define STEP_A_DIR_PIN 14
#define STEP_X_DIR_INVERT false
#define STEP_A_DIR_INVERT false