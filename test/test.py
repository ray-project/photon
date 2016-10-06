from __future__ import print_function

import os
import subprocess
import sys
import unittest
import random
import time

import common
import photon

class TestPhotonClient(unittest.TestCase):

  def setUp(self):
    # Start Redis.
    redis_executable = os.path.join(os.path.abspath(os.path.dirname(__file__)), "../common/thirdparty/redis-3.2.3/src/redis-server")
    self.p1 = subprocess.Popen([redis_executable, "--loglevel", "warning"])
    time.sleep(0.1)
    scheduler_executable = os.path.join(os.path.abspath(os.path.dirname(__file__)), "../build/photon_scheduler")
    scheduler_name = "/tmp/scheduler{}".format(random.randint(0, 10000))
    self.p2 = subprocess.Popen([scheduler_executable, "-s", scheduler_name, "-r", "127.0.0.1:6379"])
    time.sleep(0.1)
    # Connect to the scheduler.
    self.photon_client = photon.Photon(scheduler_name)

  def tearDown(self):
    # Kill the Redis server.
    self.p1.kill()
    # Kill the local scheduler.
    self.p2.kill()

  def test_submit_and_get_task(self):
    # TODO(rkn): This should be a FunctionID.
    function_id = common.ObjectID(20 * "a")
    object_ids = [common.ObjectID(20 * chr(i)) for i in range(256)]
    args_list = [
      [],
      1 * [1],
      10 * [1],
      100 * [1],
      1000 * [1],
      1 * ["a"],
      10 * ["a"],
      100 * ["a"],
      1000 * ["a"],
      [1, 1.3, 2L, 1L << 100, "hi", u"hi", [1, 2]],
      object_ids[:1],
      object_ids[:2],
      object_ids[:3],
      object_ids[:4],
      object_ids[:5],
      object_ids[:10],
      object_ids[:100],
      object_ids[:256],
      [1, object_ids[0]],
      [object_ids[0], "a"],
      [1, object_ids[0], "a"],
      [object_ids[0], 1, object_ids[1], "a"],
      object_ids[:3] + [1, "hi", 2.3] + object_ids[:5],
      object_ids + 100 * ["a"] + object_ids
    ]

    for args in args_list:
      for num_return_vals in [0, 1, 2, 3, 5, 10, 100]:
        task = common.Task(function_id, args, num_return_vals)
        # Submit a task.
        self.photon_client.submit(task)
        # Get the task.
        new_task = self.photon_client.get_task()
        self.assertEqual(task.function_id().id(), new_task.function_id().id())
        retrieved_args = new_task.arguments()
        returns = new_task.returns()
        self.assertEqual(len(args), len(retrieved_args))
        self.assertEqual(num_return_vals, len(returns))
        for i in range(len(retrieved_args)):
          if isinstance(args[i], common.ObjectID):
            self.assertEqual(args[i].id(), retrieved_args[i].id())
          else:
            self.assertEqual(args[i], retrieved_args[i])

if __name__ == "__main__":
  unittest.main(verbosity=2)
