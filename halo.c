#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <inttypes.h>

#include "utarray.h"
#include "common.h"
/* TODO(pcm): Get rid of this by changing the API of
 * db_connect to return a db_conn */
#include "state/redis.h"
#include "state/task_queue.h"
#include "state/db.h"
#include "event_loop.h"
#include "task.h"
#include "halo.h"

typedef struct {
  event_loop *loop;
  db_conn *db;
  UT_array *task_queue;
} local_scheduler_state;

void init_local_scheduler(local_scheduler_state *s) {
  s->loop = malloc(sizeof(event_loop));
  event_loop_init(s->loop);
};

void new_client_connection(int listener_sock, local_scheduler_state *s) {
  int new_socket = accept(listener_sock, NULL, NULL);
  if (new_socket < 0) {
    if (errno != EWOULDBLOCK) {
      LOG_ERR("accept failed");
      exit(-1);
    }
    return;
  }
  event_loop_attach(s->loop, 0, NULL, new_socket, POLLIN);
  LOG_INFO("new connection with index %" PRId64, event_loop_size(s->loop));
}

void read_task_from_socket(local_scheduler_state *s, int client_sock) {
  task_spec *task = read_task(client_sock);
  task_queue_submit_task(s->db, globally_unique_id(), task);
  free(task);
}

void run_event_loop(int sock, local_scheduler_state *s) {
  while (1) {
    int num_ready = event_loop_poll(s->loop);
    if (num_ready < 0) {
      LOG_ERR("poll failed");
      exit(-1);
    }
    for (int i = 0; i < event_loop_size(s->loop); ++i) {
      printf("%d event_loop_type: %d\n", i, event_loop_type(s->loop, i));
      struct pollfd *waiting = event_loop_get(s->loop, i);
      if (waiting->revents == 0)
        continue;
      if (waiting->fd == sock) {
        new_client_connection(sock, s);
      } else if (event_loop_type(s->loop, i) == CONNECTION_REDIS) {
        printf("redis event %d\n", i);
        db_event(s->db);
      } else {
        read_task_from_socket(s, waiting->fd);
      }
    }
  }
}

void start_server(const char* socket_name, const char* redis_addr, int redis_port) {
  int fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd == -1) {
    LOG_ERR("socket error");
    exit(-1);
  }
  int on = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*) &on, sizeof(on)) < 0) {
    LOG_ERR("setsockopt failed");
    close(fd);
    exit(-1);
  }
  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, socket_name, sizeof(addr.sun_path) - 1);
  unlink(socket_name);
  bind(fd, (struct sockaddr*) &addr, sizeof(addr));
  listen(fd, 5);
  local_scheduler_state state;
  init_local_scheduler(&state);
  /* Add listening socket. */
  event_loop_attach(state.loop, CONNECTION_LISTENER, NULL, fd, POLLIN);
  state.db = malloc(sizeof(db_conn));
  db_connect(redis_addr, redis_port, "halo", "", -1, state.db);
  db_attach(state.db, state.loop, CONNECTION_REDIS);
  run_event_loop(fd, &state);
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
  char redis_addr[16] = { 0 };
  char redis_port[6] = { 0 };
  if(!redis_addr_port || sscanf(redis_addr_port, "%15[0-9.]:%5[0-9]", redis_addr, redis_port) != 2) {
    LOG_ERR("need to specify redis address like 127.0.0.1:6379 with -r switch");
    exit(-1);
  }
  start_server(scheduler_socket_name, &redis_addr[0], atoi(redis_port));
}
