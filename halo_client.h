#ifndef HALO_CLIENT_H
#define HALO_CLIENT_H

#include "halo.h"
#include "common/task.h"

typedef struct halo_conn_impl halo_conn;

/* Connect to the local scheduler. */
halo_conn *halo_connect(const char* halo_socket);

/* Submit a task to the local scheduler. */
void halo_submit(halo_conn *conn, task_spec *task);

/* Get next task for this client. */
task_spec *halo_get_task(halo_conn *conn);

/* Disconnect from the local scheduler. */
void halo_disconnect(halo_conn *conn);

#endif
