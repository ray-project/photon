#ifndef PHOTON_SCHEDULER_H
#define PHOTON_SCHEDULER_H

#include "task.h"

typedef struct local_scheduler_state local_scheduler_state;

/**
 * Establish a connection to a new client.
 *
 * @param loop Event loop of the local scheduler.
 * @param listener_socket Socket the local scheduler is listening on for new
 *        client requests.
 * @param context State of the local scheduler.
 * @param events Flag for events that are available on the listener socket.
 * @return Void.
 */
void new_client_connection(event_loop *loop, int listener_sock, void *context,
                           int events);

/**
 * Check if all of the remote object arguments for a task are available in the
 * local object store.
 *
 * @param s The state of the local scheduler.
 * @param task Task specification of the task to check.
 * @return This returns 1 if all of the remote object arguments for the task are
 *         present in the local object store, otherwise it returns 0.
 */
int can_run(local_scheduler_state *s, task_spec *task);

/**
 * Assign a task to a worker.
 *
 * @param client_sock The client socket to use to send the task to the worker.
 * @param task The task to assign to the worker.
 * @return Void.
 */
void assign_task_to_worker(int client_sock, task_spec *task);

/**
 * If there is a task whose dependencies are available locally, assign it to the
 * worker. This does not remove the worker from the available worker queue.
 *
 * @param s The state of the local scheduler.
 * @param client_sock The client socket to use to send the task to the worker.
 * @return This returns 1 if it successfully assigned a task to the worker,
 *         otherwise it returns 0.
 */
int find_and_schedule_task_if_possible(local_scheduler_state *s,
                                       int client_sock);

/**
 * Assign a task to a worker.
 *
 * @param s State of the local scheduler.
 * @param client_sock Socket by which the worker is connected.
 * @return Void.
 */
void handle_get_task(local_scheduler_state *s, int client_sock);

/**
 * Handle incoming submit request by a worker.
 *
 * @param s State of the local scheduler.
 * @param task Task specification of the task to be submitted.
 * @return Void.
 */
void handle_submit_task(local_scheduler_state *s, task_spec *task);

/**
 * This is the callback that is used to process a notification from the Plasma
 * store that an object has been sealed.
 *
 * @param loop The local scheduler's event loop.
 * @param client_sock The file descriptor to read the notification from.
 * @param context The local scheduler state.
 * @param events
 * @return Void.
 */
void process_plasma_notification(event_loop *loop, int client_sock,
                                 void *context, int events);

#endif /* PHOTON_SCHEDULER_H */
