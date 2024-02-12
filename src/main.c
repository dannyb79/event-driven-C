#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

#include "event_manager.h"
#include "events_table.h"
#include "consumer1.h"

// send event and data (if needed) to dispatcher
void broadcast_event( int event_id, int data )
{
    printf( "[ PROD  ] Broadcasting event %d data %d\n", event_id, data );
    send_event( event_id, data );
}

int main( int argc, char *argv[] ) {

    // initialize event manager module
    initialize_event_manager();

    // initialize consumers
    initialize_consumer1();
    initialize_consumer2();
    initialize_consumer3();

    // wait all threads are ready
    sleep( 1 );

#ifdef EVENT_MANAGER_DEBUG
    debug_event_group_listeners_list();
#endif

    printf( "\n\n\t Initialization complete, threads already running. Start broadcasting events...\n\n\n" );

    // we use main thread as an event producer

    // event1 (belongs to events_group_1) should be dispatched to consumer 1 and 3
    broadcast_event( ev_event1, 123 );
    sleep( 1 );

    // event3 (belongs to events_group_2) should be dispatched to consumer 1 and 2
    broadcast_event( ev_event3, 456 );
    sleep( 1 );

    // event5 (belongs to events_group_3) should be dispatched to consumer 3 only
    broadcast_event( ev_event5, 789 );
    sleep( 1 );

    printf( "\n\n\t Gently terminating...\n\n\n" );

    // terminating all thread subscribed for events_group_threads group
    broadcast_event( ev_terminate_thread, 0 );
    sleep( 1 );
    terminate_consumer1();
    terminate_consumer2();
    terminate_consumer3();

    return 0;
}
