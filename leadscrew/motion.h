// Motion control function prototypes

void motion_get_position(float *x_mm, float *a_deg);
void motion_main(bool loop);
void motion_plan_move(float x1_mm, float a1_deg, float x_feedrate_mm_s, float a_feedrate_deg_s);
void motion_spiral_move_x(float x1_mm, float t_pitch_mm, float t_degrees);

void motion_dump_status();

inline static void motion_thread_metric(float x1_mm, float metric_pitch, bool lefthanded) {
  motion_spiral_move_x(x1_mm, metric_pitch, lefthanded ? -360.0f : 360.0f);
}

inline static void motion_thread_imperial(float x1_mm, float tpi, bool lefthanded) {
  motion_spiral_move_x(x1_mm, lefthanded ? -25.4f : 25.4f, 360.0f / tpi);
}
