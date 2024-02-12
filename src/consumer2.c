#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "event_manager.h"
#include "events_table.h"
#include "consumer2.h"

// module's thread handle returned from pthread_create
static pthread_t            thread_id = 0;

// thread control / configuration data
static thread_ctrl_t        *thread_ctrl;

// event groups this module is interested in
#define CONSUMER2_EVENT_GROUPS      2

static events_group_t       event_group_list[ CONSUMER2_EVENT_GROUPS ] = {
    events_group_2,
    events_group_threads
};

// ------------------- event handlers (start) ---------------------------------

static void event3_handler( event_object_t event_object )
{
    printf( "[ CONS2 ] event3 handler, event data %d\n", event_object.data );
}

static void event4_handler( event_object_t event_object )
{
    printf( "[ CONS2 ] event4 handler, event data %d\n", event_object.data );
}

// ------------------- event handlers (end) ---------------------------------

// this table collects all event handlers and link them to events properly
#define CONSUMER2_EVENT_HANDLERS        4

static handler_t event_handlers_table[ CONSUMER2_EVENT_HANDLERS ] = {
    {  ev_event3,               event3_handler              },
    {  ev_event4,               event4_handler              }
};

// initialization of consumer 2
void initialize_consumer2()
{
    thread_ctrl = malloc( sizeof( thread_ctrl_t ) );
    thread_ctrl->module_id              = 2;    // unique id
    thread_ctrl->max_groups             = CONSUMER2_EVENT_GROUPS;
    thread_ctrl->groups                 = (events_group_t*)&event_group_list;
    thread_ctrl->max_event_handlers     = CONSUMER2_EVENT_HANDLERS;
    thread_ctrl->handlers               = (handler_t*)&event_handlers_table;
    thread_ctrl->timedwait_milliseconds = 0;
    thread_ctrl->timed_ops              = NULL;

    // create a thread waiting for events and set it up through thread_ctrl structure
    pthread_create( &thread_id, NULL, event_processing_thread, ( void* )( thread_ctrl ) );
}

// wait module's thread termination after sending special event ev_terminate_thread
void terminate_consumer2()
{
    if( thread_id != 0 ) {
        pthread_join( thread_id, NULL );
    }
}

