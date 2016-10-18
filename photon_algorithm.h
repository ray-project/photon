#ifndef PHOTON_ALGORITHM_H
#define PHOTON_ALGORITHM_H

#include "photon.h"
#include "common/task.h"

/* ==== The scheduling algorithm ====
 *
 * This file contains declaration for all functions and data structures that need
 * to be provided if you want to implement a new algorithms for the local scheduler.
 */

/** Internal state of the scheduling algorithm. */
typedef struct scheduler_state scheduler_state;

/** Initialize the scheduler state. */
scheduler_state *make_scheduler_state();

/** Free the scheduler state. */
void free_scheduler_state(scheduler_state *state);

/** This function will be called when a new task is submitted by a worker for execution.
 *  @param info Info about resources exposed by photon to the scheduling algorithm.
 *  @param state State of the scheduling algorithm.
 *  @param task Task that is submitted by the worker.
 */
void handle_task_submitted(scheduler_info *info, scheduler_state *state, task_spec *task);

/** This function will be called when a task is assigned by the global scheduler for
 *  execution on this local scheduler.
 *
 *  @param info Info about resources exposed by photon to the scheduling algorithm.
 *  @param state State of the scheduling algorithm.
 *  @param task Task that is assigned by the global scheduler.
 */
void handle_task_assigned(scheduler_info *info, scheduler_state *state, task_spec *task);

/** This function is called if a new object becomes available in the local plasma store.
 *
 *  @param info Info about resources exposed by photon to the scheduling algorithm.
 *  @param state State of the scheduling algorithm.
 *  @param object_id ID of the object that became available.
 */
void handle_object_available(scheduler_info *info, scheduler_state *state, object_id object_id);

/** This function is called when a new worker becomes available
 *
 *  @param info Info about resources exposed by photon to the scheduling algorithm.
 *  @param state State of the scheduling algorithm.
 *  @param worker_index The index of the worker that becomes available.
 */
void handle_worker_available(scheduler_info *info, scheduler_state *state, int worker_index);

#endif /* PHOTON_ALGORITHM_H */
