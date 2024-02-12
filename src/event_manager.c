/**
 * Copyright 2024 Daniele Brunello daniele.brunello.dev@gmail.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "event_manager.h"
#include "events_table.h"


// list node for listener
typedef struct event_listener_node {
    thread_data_t               *thread_data;
    struct event_listener_node  *next;
} event_listener_node_t;

// list of threads listening for specific events group
static event_listener_node_t      *event_group_listeners[ events_group_max ];

// print pointer of all thread listening for each specific group of events
void debug_event_group_listeners_list()
{
    int i = 0;

    // scan all entry for each group
    for( i = 0; i < events_group_max; i++ ) {
        printf( "[ EVMNG ] Listeners for event group %d -> ", i );
        // if null = nobody subscribed for this group of events
        if( event_group_listeners[ i ] == NULL ) {
            printf( "none\n" );
        } else {
            // start from first node
            event_listener_node_t *p;
            p = event_group_listeners[ i ];
            while( p != NULL ) {
                // write thread data pointer listening for this group
                printf( "%p ", *p );
                p = p->next;
            }
            printf( "\n" );
        }
    }
}

// initialize event manager module
void initialize_event_manager() {

    // reset thread list pointers
    memset( &event_group_listeners, 0, sizeof( event_group_listeners ) );

}

// called by a thread to subscribe to an event group
void subscribe_for_events_group( thread_data_t *thread_data, events_group_t event_group )
{
#ifdef EVENT_MANAGER_DEBUG
    printf( "[ EVMNG ] Thread %d %p subscribing for event group %d\n", thread_data->thread_id, thread_data, event_group );
#endif

    if( ( event_group < 0 ) || ( event_group >= events_group_max ) ) {
#ifdef EVENT_MANAGER_DEBUG
        printf( "[ EVMNG ] Error. Wrong subscription group %d\n", event_group );
#endif
        return;
    }

    if( event_group_listeners[ event_group ] != NULL ) {
        event_listener_node_t   *p;

        p = event_group_listeners[ event_group ];
        while( p->next != NULL ) {
            p = p->next;
        }
        p->next = ( event_listener_node_t * ) malloc( sizeof( event_listener_node_t ) );
        p->next->thread_data = thread_data;
        p->next->next = NULL;

    } else {
        event_group_listeners[ event_group ] = ( event_listener_node_t * ) malloc( sizeof( event_listener_node_t ) );
        event_group_listeners[ event_group ]->thread_data = thread_data;
        event_group_listeners[ event_group ]->next = NULL;
    }
}

// initialize thread's event queue
void initialize_thread_event_queue( event_queue *queue ) {
    queue->front = -1;
    queue->rear = -1;
}

// check if the (thread) queue is empty
int queue_is_empty( event_queue *queue ) {
    return ( queue->front == -1 && queue->rear == -1 );
}

// check if the queue is full
static int queue_is_full( event_queue *queue) {
    return ( ( queue->rear + 1 ) % THREAD_EVENT_QUEUE_SIZE == queue->front );
}

// dequeue event
event_object_t dequeue_event( event_queue *queue ) {
    event_object_t event_object = { .id = -1 };

    // check if queue is empty
    if ( queue_is_empty( queue ) ) {
#ifdef EVENT_MANAGER_DEBUG
        // commented out to not messing up log
        // printf("[ EVMNG ] Queue is empty, cannot dequeue item.\n");
#endif
        return event_object;
    }

    event_object = queue->events[ queue->front ];
    if ( queue->front == queue->rear ) {
        queue->front = -1;
        queue->rear = -1;
    } else {
        queue->front = ( queue->front + 1 ) % THREAD_EVENT_QUEUE_SIZE;
    }

    return event_object;
}

// dispatch event to specific threads
static void dispatch_event( thread_data_t *thread_data, event_object_t event_object ) {

#ifdef EVENT_MANAGER_DEBUG
    printf("[ EVMNG ] Dispatching event %d to thread %d %p\n", event_object.id, thread_data->thread_id, thread_data );
#endif

    // lock thread mutex
    pthread_mutex_lock( &thread_data->mutex );

    // enqueue the event into thread's event queue
    if ( queue_is_full( &thread_data->queue ) ) {
#ifdef EVENT_MANAGER_DEBUG
        printf("[ EVMNG ] Thread queue is full, cannot enqueue item.\n");
#endif
    } else {
        if ( queue_is_empty( &thread_data->queue ) ) {
            thread_data->queue.front = 0;
            thread_data->queue.rear = 0;
        } else {
            thread_data->queue.rear = ( thread_data->queue.rear + 1 ) % THREAD_EVENT_QUEUE_SIZE;
        }
        thread_data->queue.events[ thread_data->queue.rear ] = event_object;

#ifdef EVENT_MANAGER_DEBUG
        printf("[ EVMNG ] Event %d enqueued for thread %d\n", event_object.id, thread_data->thread_id );
#endif
    }

    // unlock mutex
    pthread_mutex_unlock( &thread_data->mutex );

    // signal condition variable to wake up thread and read the event
    pthread_cond_signal( &thread_data->cond );
}

// get timesatmp in milliseconds
static int64_t current_timestamp_millis() {
    struct timeval te;
    gettimeofday(&te, NULL); // Get current time
    int64_t milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000; // Calculate milliseconds
    return milliseconds;
}

// send event to dispachter
void send_event( event_id_t event_id, uint32_t data )
{
    event_listener_node_t   *p;
    events_group_t          group;

#ifdef EVENT_MANAGER_DEBUG
    printf( "[ EVMNG ] Dispatching event %d\n", event_id );
#endif

    if( ( event_id >= 0 ) && ( event_id < ev_max ) ) {

        group = events_table[ event_id ].group;

        event_object_t  event_object;
        event_object.id         = event_id;
        event_object.timestamp  = current_timestamp_millis();
        event_object.data       = data;

        // signal event to all listeners interested in event's group
        p = event_group_listeners[ group ];
        while( p != NULL ) {
            dispatch_event( p->thread_data, event_object );
            p = p->next;
        }
    } else {
#ifdef EVENT_MANAGER_DEBUG
        printf( "[ EVMNG ] Wrong event id %d\n", event_id );
#endif
    }
}

// get current time and add n- millisecond for pthread_cond_timedwait
static void get_wait_time( struct timespec *ts, int milliseconds_timeout )
{
    ts->tv_sec += milliseconds_timeout / 1000;
    ts->tv_nsec += (milliseconds_timeout % 1000) * 1000000;
    if (ts->tv_nsec >= 1000000000) {
        ts->tv_sec++;
        ts->tv_nsec -= 1000000000;
    }
}

// base event processing thread customizable using thread_ctrl_t structure
void* event_processing_thread( void *arg )
{
    struct timespec     ts;
    thread_data_t       thread_data;
    event_object_t      event_object;
    thread_ctrl_t       *thread_ctrl = ( thread_ctrl_t* ) arg;
    int i;

#ifdef EVENT_MANAGER_DEBUG
    printf( "[ EPT   ] Creating module %d interested in %d groups : ", thread_ctrl->module_id, thread_ctrl->max_groups );
    for( i = 0; i < thread_ctrl->max_groups; i++ ) {
        printf( " %d ", thread_ctrl->groups[ i ] );
    }
    printf( "\n" );
#endif

    // assign a unique id to this thread
    thread_data.thread_id = thread_ctrl->module_id;

    // initialize thread queue, mutex and condition variable
    initialize_thread_event_queue( &thread_data.queue );
    pthread_mutex_init( &thread_data.mutex, NULL );
    pthread_cond_init( &thread_data.cond, NULL );

    // register for event groups
    for( i = 0; i < thread_ctrl->max_groups; i++ ) {
        subscribe_for_events_group( &thread_data, thread_ctrl->groups[ i ] );
    }

#ifdef EVENT_MANAGER_DEBUG
    printf("[ EPT %d ] Initialization complete thread data @ %p\n", thread_data.thread_id, &thread_data );
#endif

    // process events indefinitely
    while ( 1 ) {

        // lock mutex
        pthread_mutex_lock( &thread_data.mutex );

        // check if event queue is empty
        if( queue_is_empty( &thread_data.queue ) ) {

            // commented out to not messing up log
            // printf("[ EPT %d ] Waiting for event...\n", thread_data.thread_id );

            // if a wait timeout is specified...
            if( thread_ctrl->timedwait_milliseconds > 0 ) {
                // wait for an event to be available until timeout expires
                clock_gettime(CLOCK_REALTIME, &ts);
                get_wait_time( &ts, thread_ctrl->timedwait_milliseconds );
                pthread_cond_timedwait( &thread_data.cond, &thread_data.mutex, &ts );
            } else {
                // wait indefinitely for an event to be available
                pthread_cond_wait( &thread_data.cond, &thread_data.mutex );
            }
        }

        // dequeue an event
        event_object = dequeue_event( &thread_data.queue );

#ifdef EVENT_MANAGER_DEBUG
        printf("[ EPT %d ] Dequeued event id: %2d data %-012d timestamp %lld\n", thread_data.thread_id, event_object.id, event_object.data, event_object.timestamp ); // Print statement for debugging
#endif
        // unlock mutex
        pthread_mutex_unlock( &thread_data.mutex );

        // terminate thread immediately
        if( event_object.id == ev_terminate_thread ) {
            break;
        }

        // if event_object.id == -1 it may be a timed wait task
        if( ( event_object.id >= 0 ) && ( event_object.id < ev_max ) ) {
            // search and call the appropriate event handler
            // NOTE in case of bigger arrays search operation should be improved with hash table
            for( i = 0; i < thread_ctrl->max_event_handlers; i++ ) {
                if( event_object.id == thread_ctrl->handlers[ i ].event_id ) {
                    thread_ctrl->handlers[ i ].handler( event_object );
                    break;
                }
            }
        }

        // perform timed operations (if needed)
        if( thread_ctrl->timed_ops != NULL ) {
            thread_ctrl->timed_ops();
        }
    }

#ifdef EVENT_MANAGER_DEBUG
    printf("[ EPT %d ] Thread terminated\n", thread_data.thread_id );
#endif

    return NULL;
}

