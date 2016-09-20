struct halo_conn_impl {
  /* File descriptor of the Unix domain socket that connects to halo. */
  int conn;
};

enum conn_type {
  CONNECTION_LISTENER,
  CONNECTION_REDIS,
  CONNECTION_CLIENT
};
