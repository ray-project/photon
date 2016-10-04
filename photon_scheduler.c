#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "common.h"
#include "event_loop.h"
#include "io.h"
#include "photon.h"
#include "state/db.h"
#include "state/task_queue.h"
#include "task.h"
#include "utarray.h"

typedef struct {
  /** The file descriptor used to communicate with the worker. */
  int client_sock;
} available_worker;

/* These are needed to define the UT_arrays. */
UT_icd task_ptr_icd = {sizeof(task_spec *), NULL, NULL, NULL};
UT_icd worker_icd = {sizeof(available_worker), NULL, NULL, NULL};

typedef struct {
  db_handle *db;
  /** This is an array of pointers to tasks that are waiting to be scheduled. */
  UT_array *task_queue;
  /** This is an array of file descriptors corresponding to clients that are
   *  waiting for tasks. */
  UT_array *available_worker_queue;
} local_scheduler_state;

void try_to_assign_task(task_spec *task, local_scheduler_state *s);
void try_to_assign_task_to_worker(int client_sock, local_scheduler_state *s);

event_loop *init_local_scheduler() { return event_loop_create(); };

void process_message(event_loop *loop, int client_sock, void *context,
                     int events) {
  local_scheduler_state *s = context;

  uint8_t *message;
  int64_t type;
  int64_t length;
  read_message(client_sock, &type, &length, &message);

  switch (type) {
  case SUBMIT_TASK: {
    task_spec *task = (task_spec *)message;
    CHECK(task_size(task) == length);
    /* Create a unique task instance ID. This is different from the task ID and
     * is used to distinguish between potentially multiple executions of the
     * task. */
    unique_id id = globally_unique_id();
    // task_queue_submit_task(s->db, id, task);
    /* Try to assign the task to a worker locally. TODO(rkn): This should
     * probably go somewhere else. */
    try_to_assign_task(task, s);
  } break;
  case TASK_DONE: {
  } break;
  case GET_TASK: {
    try_to_assign_task_to_worker(client_sock, s);
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

void try_to_assign_task(task_spec *task, local_scheduler_state *s) {
  /* Assign this task to an available worker. If there are no available workers,
   * then add this task to the local task queue. */
  if (utarray_len(s->available_worker_queue) > 0) {
    /* Get the last available worker in the available worker queue. */
    available_worker *worker =
        (available_worker *)utarray_back(s->available_worker_queue);
    /* Tell the available worker to execute the task. */
    write_message(worker->client_sock, EXECUTE_TASK, task_size(task),
                  (uint8_t *)task);
    utarray_pop_back(s->available_worker_queue);
    /* TODO: Do we need to free the available_worker struct? */
  } else {
    /* Add the task to the task queue. */
    task_spec *task_copy = malloc(task_size(task));
    memcpy(task_copy, task, task_size(task));
    utarray_push_back(s->task_queue, &task_copy);
  }
}

void try_to_assign_task_to_worker(int client_sock, local_scheduler_state *s) {
  if (utarray_len(s->task_queue) > 0) {
    /* Get the last task in the task queue. */
    task_spec **task_ptr = (task_spec **)utarray_back(s->task_queue);
    task_spec *task = *task_ptr;
    /* Send a task to the worker. */
    write_message(client_sock, EXECUTE_TASK, task_size(task), (uint8_t *)task);
    /* Update the task queue data structure and free the task. */
    utarray_pop_back(s->task_queue);
    free(task);
  } else {
    /* Check that client_sock is not already in the available workers. */
    for (available_worker *p =
             (available_worker *)utarray_front(s->available_worker_queue);
         p != NULL;
         p = (available_worker *)utarray_next(s->available_worker_queue, p)) {
      CHECK(p->client_sock != client_sock);
    }
    /* Add client_sock to a list of available workers. */
    available_worker worker_info = {.client_sock = client_sock};
    utarray_push_back(s->available_worker_queue, &worker_info);
    LOG_INFO("Adding client_sock %d to available workers.\n", client_sock);
  }
}

void new_client_connection(event_loop *loop, int listener_sock, void *context,
                           int events) {
  local_scheduler_state *s = context;
  int new_socket = accept_client(listener_sock);
  event_loop_add_file(loop, new_socket, EVENT_LOOP_READ, process_message, s);
  LOG_INFO("new connection with fd %d", new_socket);
}

void start_server(const char *socket_name, const char *redis_addr,
                  int redis_port) {
  int fd = bind_ipc_sock(socket_name);
  local_scheduler_state state;
  event_loop *loop = init_local_scheduler();

  state.db = db_connect(redis_addr, redis_port, "photon", "", -1);
  db_attach(state.db, loop);
  utarray_new(state.task_queue, &task_ptr_icd);
  utarray_new(state.available_worker_queue, &worker_icd);

  /* Run event loop. */
  event_loop_add_file(loop, fd, EVENT_LOOP_READ, new_client_connection, &state);
  event_loop_run(loop);
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
