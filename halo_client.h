#ifndef HALO_CLIENT_H
#define HALO_CLIENT_H

#include "halo.h"
#include "common/task.h"

typedef struct halo_conn_impl halo_conn;

typedef void (*execute_callback)(task_iid task_iid,
                                 task_spec *task);

/* Connect to the local scheduler. */
halo_conn *halo_connect(const char* halo_socket);
/* Submit a task to the local scheduler. */
void halo_submit(halo_conn *conn, task_spec *task);
/* Register callback that will be called when a task is scheduled
 * to execute on this client by the local scheduler. */
void halo_register_execute_callback(halo_conn *conn, execute_callback callback);
/* Disconnect from the local scheduler. */
void halo_disconnect(halo_conn *conn);

#endif
