#include "halo_client.h"

#include <stdlib.h>
#include "common/io.h"
#include "common/task.h"

halo_conn *halo_connect(const char* halo_socket) {
  halo_conn *result = malloc(sizeof(halo_conn));
  result->conn = connect_ipc_sock(halo_socket);
  return result;
}

void halo_submit(halo_conn *conn, task_spec *task) {
  write_task(conn->conn, task);
}
