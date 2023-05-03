// Motion control function prototypes

void motion_get_position(float *x, float *a);
void motion_main();
void motion_spiral_move_x(float x1_mm, float t_pitch_mm, float t_degrees, bool lefthanded);

inline static void motion_thread_metric(float x1_mm, float metric_pitch, bool lefthanded) {
  motion_spiral_move_x(x1_mm, metric_pitch, 360.0f, lefthanded);
}

inline static void motion_thread_imperial(float x1_mm, float tpi, bool lefthanded) {
  motion_spiral_move_x(x1_mm, 25.4f, 360.0f / tpi, lefthanded);
}
