#include "stdint.h"

typedef struct {
  int8_t cmd;
} command_t;

#define CMD_ESTOP    -1
#define CMD_Q_STATUS  0
#define CMD_RUN       1
#define CMD_STOP      2

typedef struct {
  int8_t result;
  int8_t status;
  int32_t x_pos, a_pos;
  int32_t steps_left;
} response_t;

#define RESULT_OK 0

#define STATUS_STOPPED  0
#define STATUS_RUN      1
#define STATUS_STOPPING 2
#define STATUS_ESTOPPED -1

void intercore_init();
void intercore_command(int8_t cmd);
bool intercore_getcommand_nb(command_t *command);
void intercore_respond_nb(response_t *response);

extern response_t intercore_response;
