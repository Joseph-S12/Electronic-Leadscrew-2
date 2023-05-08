// Motion control function prototypes

bool motion_complete();
void motion_get_position(float *x_mm, float *a_deg);
void motion_main();
void motion_spiral_move_x(float x1_mm, float t_pitch_mm, float t_degrees);

void motion_dump_constants();
void motion_dump_status();

inline static void motion_thread_metric(float x1_mm, float metric_pitch, bool lefthanded) {
  motion_spiral_move_x(x1_mm, metric_pitch, lefthanded ? -360.0f : 360.0f);
}

inline static void motion_thread_imperial(float x1_mm, float tpi, bool lefthanded) {
  motion_spiral_move_x(x1_mm, lefthanded ? -25.4f : 25.4f, 360.0f * tpi);
}

void motion_plan_thread_metric(float pitch_mm);
void motion_plan_thread_imperial(float tpi);
void motion_plan_direction(bool forward, bool lefthanded);
void motion_run();
void motion_stop();
