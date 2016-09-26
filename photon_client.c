#include "photon_client.h"

#include <stdlib.h>
#include "common/io.h"
#include "common/task.h"

photon_conn *photon_connect(const char* photon_socket) {
  photon_conn *result = malloc(sizeof(photon_conn));
  result->conn = connect_ipc_sock(photon_socket);
  return result;
}

void photon_submit(photon_conn *conn, task_spec *task) {
  write_task(conn->conn, task);
}
