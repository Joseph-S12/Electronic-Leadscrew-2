#include "pico/util/queue.h"

#include "intercore.h"

static queue_t command_q;
static queue_t response_q;

response_t intercore_response;

void intercore_init() {
  queue_init(&command_q, sizeof(command_t), 1);
  queue_init(&response_q, sizeof(response_t), 1);
}

void intercore_command(int8_t cmd) {
  command_t c;
  c.cmd = cmd;

  queue_add_blocking(&command_q, &c);
  queue_remove_blocking(&response_q, &intercore_response);
}

bool intercore_getcommand_nb(command_t *command) {
  return queue_try_remove(&command_q, command);
}

void intercore_respond_nb(response_t *response) {
  while(!queue_try_add(&response_q, response));
}
