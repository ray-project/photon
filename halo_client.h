#ifndef HALO_CLIENT_H
#define HALO_CLIENT_H

#include "halo.h"
#include "common/task.h"

typedef struct halo_conn_impl halo_conn;

halo_conn *halo_connect(const char* halo_socket);
void halo_submit(halo_conn *conn, task_spec *task);
void halo_disconnect(halo_conn *conn);

#endif
