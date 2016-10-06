#include <Python.h>
#include "node.h"

#include "photon_client.h"
#include "types.h"
#include "task.h"

PyObject *PhotonClientError;

// clang-format off
typedef struct {
  PyObject_HEAD
  photon_conn *photon_connection;
} PyPhotonClient;
// clang-format on

static int PyPhotonClient_init(PyPhotonClient *self, PyObject *args,
                               PyObject *kwds) {
  char* socket_name;
  if (!PyArg_ParseTuple(args, "s", &socket_name)) {
    return -1;
  }
  self->photon_connection = photon_connect(socket_name);
  return 0;
}

static void PyPhotonClient_dealloc(PyPhotonClient *self) {
  free(((PyPhotonClient *) self)->photon_connection);
  Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *PyPhotonClient_submit(PyObject *self, PyObject *args) {
  PyObject *py_task;
  if (!PyArg_ParseTuple(args, "O", &py_task)) {
    return NULL;
  }
  photon_submit(((PyPhotonClient *) self)->photon_connection,
                ((PyTask *) py_task)->spec);
  Py_RETURN_NONE;
}

static PyObject *PyPhotonClient_get_task(PyObject *self) {
  task_spec *task_spec =
      photon_get_task(((PyPhotonClient *) self)->photon_connection);
  return PyTask_make(task_spec);
}

static PyMethodDef PyPhotonClient_methods[] = {
    {"submit", (PyCFunction) PyPhotonClient_submit, METH_VARARGS,
     "Submit a task to the local scheduler."},
    {"get_task", (PyCFunction) PyPhotonClient_get_task, METH_NOARGS,
     "Get a task from the local scheduler."},
    {NULL} /* Sentinel */
};

static PyTypeObject PyPhotonClientType = {
    PyObject_HEAD_INIT(NULL) 0,  /* ob_size */
    "photon.PhotonClient",                 /* tp_name */
    sizeof(PyPhotonClient),              /* tp_basicsize */
    0,                           /* tp_itemsize */
    (destructor) PyPhotonClient_dealloc, /* tp_dealloc */
    0,                           /* tp_print */
    0,                           /* tp_getattr */
    0,                           /* tp_setattr */
    0,                           /* tp_compare */
    0,                           /* tp_repr */
    0,                           /* tp_as_number */
    0,                           /* tp_as_sequence */
    0,                           /* tp_as_mapping */
    0,                           /* tp_hash */
    0,                           /* tp_call */
    0,                           /* tp_str */
    0,                           /* tp_getattro */
    0,                           /* tp_setattro */
    0,                           /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,          /* tp_flags */
    "Photon client object",               /* tp_doc */
    0,                           /* tp_traverse */
    0,                           /* tp_clear */
    0,                           /* tp_richcompare */
    0,                           /* tp_weaklistoffset */
    0,                           /* tp_iter */
    0,                           /* tp_iternext */
    PyPhotonClient_methods,              /* tp_methods */
    0,                           /* tp_members */
    0,                           /* tp_getset */
    0,                           /* tp_base */
    0,                           /* tp_dict */
    0,                           /* tp_descr_get */
    0,                           /* tp_descr_set */
    0,                           /* tp_dictoffset */
    (initproc) PyPhotonClient_init,      /* tp_init */
    0,                           /* tp_alloc */
    PyType_GenericNew,           /* tp_new */
};

static PyMethodDef photon_client_methods[] = {
    {NULL} /* Sentinel */
};

#ifndef PyMODINIT_FUNC /* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif

PyMODINIT_FUNC initphoton_client(void) {
  PyObject *m;

  if (PyType_Ready(&PyPhotonClientType) < 0)
    return;

  m = Py_InitModule3("photon_client", photon_client_methods,
                     "Example module that creates an extension type.");

  Py_INCREF(&PyPhotonClientType);
  PyModule_AddObject(m, "PhotonClient", (PyObject *) &PyPhotonClientType);

  char photon_client_error[] = "photon_client.error";
  PhotonClientError = PyErr_NewException(photon_client_error, NULL, NULL);
  Py_INCREF(PhotonClientError);
  PyModule_AddObject(m, "photon_client_error", PhotonClientError);
}
