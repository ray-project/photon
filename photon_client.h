#ifndef PHOTON_CLIENT_H
#define PHOTON_CLIENT_H

#include "photon.h"
#include "common/task.h"

typedef struct photon_conn_impl photon_conn;

/* Connect to the local scheduler. */
photon_conn *photon_connect(const char* photon_socket);

/* Submit a task to the local scheduler. */
void photon_submit(photon_conn *conn, task_spec *task);

/* Get next task for this client. */
task_spec *photon_get_task(photon_conn *conn);

/* Disconnect from the local scheduler. */
void photon_disconnect(photon_conn *conn);

#endif
