#include <Python.h>
#include "node.h"

#include "common_extension.h"
#include "photon_client.h"
#include "task.h"

PyObject *PhotonError;

// clang-format off
typedef struct {
  PyObject_HEAD
  photon_conn *photon_connection;
} PyPhoton;
// clang-format on

static int PyPhoton_init(PyPhoton *self, PyObject *args,
                               PyObject *kwds) {
  char* socket_name;
  if (!PyArg_ParseTuple(args, "s", &socket_name)) {
    return -1;
  }
  self->photon_connection = photon_connect(socket_name);
  return 0;
}

static void PyPhoton_dealloc(PyPhoton *self) {
  free(((PyPhoton *) self)->photon_connection);
  Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *PyPhoton_submit(PyObject *self, PyObject *args) {
  PyObject *py_task;
  if (!PyArg_ParseTuple(args, "O", &py_task)) {
    return NULL;
  }
  photon_submit(((PyPhoton *) self)->photon_connection,
                ((PyTask *) py_task)->spec);
  Py_RETURN_NONE;
}

static PyObject *PyPhoton_get_task(PyObject *self) {
  task_spec *task_spec =
      photon_get_task(((PyPhoton *) self)->photon_connection);
  return PyTask_make(task_spec);
}

static PyMethodDef PyPhoton_methods[] = {
    {"submit", (PyCFunction) PyPhoton_submit, METH_VARARGS,
     "Submit a task to the local scheduler."},
    {"get_task", (PyCFunction) PyPhoton_get_task, METH_NOARGS,
     "Get a task from the local scheduler."},
    {NULL} /* Sentinel */
};

static PyTypeObject PyPhotonType = {
    PyObject_HEAD_INIT(NULL) 0,          /* ob_size */
    "photon.Photon",                     /* tp_name */
    sizeof(PyPhoton),                    /* tp_basicsize */
    0,                                   /* tp_itemsize */
    (destructor) PyPhoton_dealloc,       /* tp_dealloc */
    0,                                   /* tp_print */
    0,                                   /* tp_getattr */
    0,                                   /* tp_setattr */
    0,                                   /* tp_compare */
    0,                                   /* tp_repr */
    0,                                   /* tp_as_number */
    0,                                   /* tp_as_sequence */
    0,                                   /* tp_as_mapping */
    0,                                   /* tp_hash */
    0,                                   /* tp_call */
    0,                                   /* tp_str */
    0,                                   /* tp_getattro */
    0,                                   /* tp_setattro */
    0,                                   /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                  /* tp_flags */
    "Photon object",                     /* tp_doc */
    0,                                   /* tp_traverse */
    0,                                   /* tp_clear */
    0,                                   /* tp_richcompare */
    0,                                   /* tp_weaklistoffset */
    0,                                   /* tp_iter */
    0,                                   /* tp_iternext */
    PyPhoton_methods,                    /* tp_methods */
    0,                                   /* tp_members */
    0,                                   /* tp_getset */
    0,                                   /* tp_base */
    0,                                   /* tp_dict */
    0,                                   /* tp_descr_get */
    0,                                   /* tp_descr_set */
    0,                                   /* tp_dictoffset */
    (initproc) PyPhoton_init,            /* tp_init */
    0,                                   /* tp_alloc */
    PyType_GenericNew,                   /* tp_new */
};

static PyMethodDef photon_methods[] = {
    {"check_simple_value", check_simple_value, METH_VARARGS,
     "Should the object be passed by value?"},
    {NULL} /* Sentinel */
};

#ifndef PyMODINIT_FUNC /* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif

PyMODINIT_FUNC initphoton(void) {
  PyObject *m;

  if (PyType_Ready(&PyTaskType) < 0)
    return;

  if (PyType_Ready(&PyObjectIDType) < 0)
    return;

  if (PyType_Ready(&PyPhotonType) < 0)
    return;

  m = Py_InitModule3("photon", photon_methods,
                     "A module for the local scheduler.");

  Py_INCREF(&PyTaskType);
  PyModule_AddObject(m, "Task", (PyObject *) &PyTaskType);

  Py_INCREF(&PyObjectIDType);
  PyModule_AddObject(m, "ObjectID", (PyObject *) &PyObjectIDType);

  Py_INCREF(&PyPhotonType);
  PyModule_AddObject(m, "Photon", (PyObject *) &PyPhotonType);

  char photon_error[] = "photon.error";
  PhotonError = PyErr_NewException(photon_error, NULL, NULL);
  Py_INCREF(PhotonError);
  PyModule_AddObject(m, "photon_error", PhotonError);
}
