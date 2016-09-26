#ifndef PHOTON_H
#define PHOTON_H


struct photon_conn_impl {
  /* File descriptor of the Unix domain socket that connects to photon. */
  int conn;
};

#endif
