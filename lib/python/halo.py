import ctypes
import os

halo_client_library_path = os.path.join(os.path.abspath(os.path.dirname(__file__)), "../../build/halo_client.so")
halo_client_library = ctypes.cdll.LoadLibrary(halo_client_library_path)
halo_client_library.halo_connect.restype = ctypes.c_void_p

ID = ctypes.c_ubyte * 20

class UniqueID(ctypes.Structure):
  _fields_ = [("unique_id", ID)]

def make_id(string):
  if len(string) != 20:
    raise Exception("PlasmaIDs must be 20 characters long")
  unique_id = map(ord, string)
  return UniqueID(unique_id=ID(*unique_id))

class Task(object):
  def __init__(self, function_id, args):
    function_id = make_id(function_id)
    self.task_spec = ctypes.c_void_p(halo_client_library.alloc_task_spec(function_id, len(args), 1, 0))
    for arg in args:
      halo_client_library.task_args_add_ref(self.task_spec, arg)
 
  def __del__(self):
    halo_client_library.free_task_spec(self.task_spec)

class HaloClient(object):

  def __init__(self, socket_name):
    self.halo_conn = ctypes.c_void_p(halo_client_library.halo_connect(socket_name))

  def submit(self, function_id, args):
    task = Task(function_id, args)
    halo_client_library.halo_submit(self.halo_conn, task.task_spec)
