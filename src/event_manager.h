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

#ifndef __EVENT_MANAGER_H__
#define __EVENT_MANAGER_H__

#include <stdint.h>
#include <pthread.h>
#include "events_table.h"

// uncomment/comment for enable/disable debug
#define EVENT_MANAGER_DEBUG


// thread's event queue size
#define THREAD_EVENT_QUEUE_SIZE       64



// define event structure
typedef struct {
    int             id;             // unique identifier value of event
    uint32_t        data;           // extra data (if any)
    uint64_t        timestamp;      // timestamp in milliseconds when event is signaled
} event_object_t;

// thread's event queue data
typedef struct {
    event_object_t  events[ THREAD_EVENT_QUEUE_SIZE ];
    int             front;
    int             rear;
} event_queue;

// thread's data
typedef struct {
    uint32_t            thread_id;
    pthread_mutex_t     mutex;
    pthread_cond_t      cond;
    event_queue         queue;
} thread_data_t;

// event / handler relation structure
typedef struct {
    event_id_t          event_id;
    void                ( *handler )( event_object_t );
} handler_t;

/*
    thread control data

    this structure must be allocated inside module and allow to customize thread behaviour:
    module_id
    you can assign inside module a unique identifier for the thread, an integer value
    groups / max_groups
    module must tell the thread which event groups is interested in
    max_event_handlers / handlers
    module must tell the thread how to deal with event received
    timedwait_milliseconds / timed_ops
    if you leave timedwait_milliseconds zero valued the thread wait indefinitely for events, else
    if you set a value in milliseconds the thread stop waiting events and can perform additional
    operations through timed_ops callback
*/
typedef struct {
    uint32_t            module_id;                  // unique id
    uint32_t            max_groups;                 // max event group interested in
    events_group_t      *groups;                    // group indexes array
    int32_t             max_event_handlers;         // total number of handlers
    handler_t           *handlers;                  // pointer to array of handlers
    int32_t             timedwait_milliseconds;     // leave 0 to wait events indefinetely
    void                (*timed_ops)( void );       // callback called every "timedwait_milliseconds" ms
} thread_ctrl_t;


// initialize event manager module
void initialize_event_manager();

// send event to dispachter
void send_event( event_id_t event_id, uint32_t data );

// base event processing thread (you can define your custom thread but this is the base)
void* event_processing_thread( void *arg );


#endif
