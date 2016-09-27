#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <inttypes.h>

#include "utarray.h"
#include "common.h"
#include "state/task_queue.h"
#include "state/db.h"
#include "event_loop.h"
#include "task.h"
#include "photon.h"
#include "io.h"

typedef struct {
  event_loop *loop;
  db_handle *db;
  UT_array *task_queue;
} local_scheduler_state;

void init_local_scheduler(local_scheduler_state *s) {
  s->loop = event_loop_create();
};

void process_message(event_loop *loop, int client_sock, void *context, int events) {
  local_scheduler_state *s = context;

  uint8_t *message;
  int64_t type;
  int64_t length;
  read_message(client_sock, &type, &length, &message);

  switch (type) {
    case SUBMIT_TASK: {
      task_spec *task = (task_spec *) message;
      CHECK(task_size(task) == length);
      unique_id id = globally_unique_id();
      task_queue_submit_task(s->db, id, task);
    } break;
    case TASK_DONE: {
    } break;
    case DISCONNECT_CLIENT: {
      LOG_INFO("Disconnecting client on fd %d", client_sock);
      event_loop_remove_file(loop, client_sock);
    } break;
    case LOG_MESSAGE: {
    } break;
    default:
      /* This code should be unreachable. */
      CHECK(0);
  }
  free(message);
}

void new_client_connection(event_loop *loop, int listener_sock, void *context,
                           int events) {
  local_scheduler_state *s = context;
  int new_socket = accept(listener_sock, NULL, NULL);
  if (new_socket < 0) {
    if (errno != EWOULDBLOCK) {
      LOG_ERR("accept failed");
      exit(-1);
    }
    return;
  }
  event_loop_add_file(s->loop, new_socket, EVENT_LOOP_READ, process_message, s);
  LOG_INFO("new connection with fd %d", new_socket);
}

void start_server(const char* socket_name, const char* redis_addr,
                  int redis_port) {
  int fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd == -1) {
    LOG_ERR("socket error");
    exit(-1);
  }
  int on = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) < 0) {
    LOG_ERR("setsockopt failed");
    close(fd);
    exit(-1);
  }
  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, socket_name, sizeof(addr.sun_path) - 1);
  unlink(socket_name);
  bind(fd, (struct sockaddr*)&addr, sizeof(addr));
  listen(fd, 5);
  local_scheduler_state state;
  init_local_scheduler(&state);

  state.db = db_connect(redis_addr, redis_port, "photon", "", -1);
  db_attach(state.db, state.loop);

  /* Run event loop. */
  event_loop_add_file(state.loop, fd, EVENT_LOOP_READ, new_client_connection,
                      &state);
  event_loop_run(state.loop);
}

int main(int argc, char *argv[]) {
  /* Path of the listening socket of the local scheduler. */
  char *scheduler_socket_name = NULL;
  /* IP address and port of redis. */
  char *redis_addr_port = NULL;
  int c;
  while ((c = getopt(argc, argv, "s:r:")) != -1) {
    switch (c) {
    case 's':
      scheduler_socket_name = optarg;
      break;
    case 'r':
      redis_addr_port = optarg;
      break;
    default:
      LOG_ERR("unknown option %c", c);
      exit(-1);
    }
  }
  if (!scheduler_socket_name) {
    LOG_ERR("please specify socket for incoming connections with -s switch");
    exit(-1);
  }
  char redis_addr[16] = {0};
  char redis_port[6] = {0};
  if (!redis_addr_port ||
      sscanf(redis_addr_port, "%15[0-9.]:%5[0-9]", redis_addr, redis_port) !=
          2) {
    LOG_ERR("need to specify redis address like 127.0.0.1:6379 with -r switch");
    exit(-1);
  }
  start_server(scheduler_socket_name, &redis_addr[0], atoi(redis_port));
}
