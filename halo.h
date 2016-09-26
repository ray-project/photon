#ifndef HALO_H
#define HALO_H


struct halo_conn_impl {
  /* File descriptor of the Unix domain socket that connects to halo. */
  int conn;
};

#endif
