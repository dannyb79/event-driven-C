#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "event_manager.h"
#include "events_table.h"
#include "consumer3.h"

// module's thread handle returned from pthread_create
static pthread_t            thread_id = 0;

// thread control / configuration data
static thread_ctrl_t        *thread_ctrl;

// event groups this module is interested in
#define CONSUMER3_EVENT_GROUPS      3

static events_group_t       event_group_list[ CONSUMER3_EVENT_GROUPS ] = {
    events_group_1,
    events_group_3,
    events_group_threads
};

// ------------------- event handlers (start) ---------------------------------

static void event5_handler( event_object_t event_object )
{
    printf( "[ CONS3 ] event3 handler, event data %d\n", event_object.data );
}

static void event6_handler( event_object_t event_object )
{
    printf( "[ CONS3 ] event4 handler, event data %d\n", event_object.data );
}

// ------------------- event handlers (end) ---------------------------------

// this table collects all event handlers and link them to events properly
#define CONSUMER3_EVENT_HANDLERS        2

static handler_t event_handlers_table[ CONSUMER3_EVENT_HANDLERS ] = {
    {  ev_event5,               event5_handler              },
    {  ev_event6,               event6_handler              }
};

// this is the callback to perform operation periodically (if needed)
void consumer3_timed_operations( void )
{
    printf( "[ CONS3 ] Operations performed at regular intervals...\n" );
}

// initialization of consumer 3
void initialize_consumer3()
{
    thread_ctrl = malloc( sizeof( thread_ctrl_t ) );
    thread_ctrl->module_id              = 3;    // unique id
    thread_ctrl->max_groups             = CONSUMER3_EVENT_GROUPS;
    thread_ctrl->groups                 = (events_group_t*)&event_group_list;
    thread_ctrl->max_event_handlers     = CONSUMER3_EVENT_HANDLERS;
    thread_ctrl->handlers               = (handler_t*)&event_handlers_table;
    thread_ctrl->timedwait_milliseconds = 200;
    thread_ctrl->timed_ops              = consumer3_timed_operations;

    // create a thread waiting for events and set it up through thread_ctrl structure
    pthread_create( &thread_id, NULL, event_processing_thread, ( void* )( thread_ctrl ) );
}

// wait module's thread termination after sending special event ev_terminate_thread
void terminate_consumer3()
{
    if( thread_id != 0 ) {
        pthread_join( thread_id, NULL );
    }
}

