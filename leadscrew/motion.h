// Motion control function prototypes

void motion_update_status();

bool motion_complete();
void motion_get_position(float *x_mm, float *a_deg);
void motion_main();

void motion_dump_constants();
void motion_dump_status();

void motion_plan_thread_metric(float pitch_mm);
void motion_plan_thread_imperial(float tpi);
void motion_plan_direction(bool forward, bool lefthanded);
void motion_run();
void motion_stop();
