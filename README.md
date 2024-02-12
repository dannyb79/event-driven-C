# Event driven programming in C

## Overview

A simple approach to event driven programming in C using pthread (POSIX Threads) library.

## Getting Started

### Dependencies

POSIX Threads (pthread) library. You can easily build source on any environment, either Windows or Linux.

### Build & Run

#### Linux

use gcc

\# gcc main.c event_manager.c events_table.c consumer1.c consumer2.c consumer3.c -o test

\# ./test

#### Windows

Easily build sources in a Code::Blocks project

## Description

The aim of the project is to create as easily as possible asynchronous and independent modules that react independently and asynchronously to events received from a dispatcher.
Each individual module subscribes to receive only groups of events in which it is really interested.

Since in C language there is no native way to group enumeratives we can define the necessary groups via an enumerative

```
// define events group
typedef enum {
    events_group_1,
    events_group_2,
    events_group_3,
} events_group_t;
```

and associate each event with a specific group using a table

```
events_table_item_t     events_table[ ev_max ] = {
    //  event id    group                       description
    {   ev_event1,	events_group_1,             "Event 1"                   },
    {   ev_event2,  events_group_1,             "Event 2"                   },
    {   ev_event3,  events_group_2,             "Event 3"                   },
    {   ev_event4,  events_group_2,             "Event 4"                   },
    {   ev_event5,  events_group_3,             "Event 5"                   },
    {   ev_event6,  events_group_3,             "Event 6"                   },
};
```

Each module subscribes to one or more groups of events in which it is interested using subscribe_for_events_group() function.
To group all the common operations of each module in one place each module must create and configure a thread of type event_processing_thread (the closest thing we have to a C++ class). The configuration of the process occurs by appropriately enhancing the thread_ctrl_t type structure

```
typedef struct {
    uint32_t            module_id;                  // unique id
    uint32_t            max_groups;                 // max event group interested in
    events_group_t      *groups;                    // group indexes array
    int32_t             max_event_handlers;         // total number of handlers
    handler_t           *handlers;                  // pointer to array of handlers
    int32_t             timedwait_milliseconds;     // leave 0 to wait events indefinetely
    void                (*timed_ops)( void );       // callback called every "timedwait_milliseconds" ms
} thread_ctrl_t;
```

When a module subscribes to a group of events the thread (to be woken up in the event of an event) is added to a list linked to the specific group

```
[ EVMNG ] Listeners for event group 1 -> 0x7f677f1fe9e0 0x7f677e1fc9e0
[ EVMNG ] Listeners for event group 2 -> 0x7f677f1fe9e0 0x7f677e9fd9e0
[ EVMNG ] Listeners for event group 3 -> 0x7f677e1fc9e0
```

When an event is sent, the dispatcher searches for the group it belongs to and sends the event to all the threads that are interested in it

```
	group = events_table[ event_id ].group;
	[...]
	// signal event to all listeners interested in event's group
	p = event_group_listeners[ group ];
	while( p != NULL ) {
		dispatch_event( p->thread_data, event_object );
		p = p->next;
	}
```



In the example code, 3 independent modules are created: the first module (consumer1) is interested in receiving event groups 1 and 2, the second module (consumer2) is interested in receiving only the events of group 2 and finally the third module is interested in receiving the events of groups 1 and 3. Furthermore, module 3 requires operations to be performed periodically every 200ms regardless of whether events have been received or not.
Further optimizations can be done. If event table becomes bigger and bigger search must be improved using hash table.

## Credit & License 

   Copyright 2024 Daniele Brunello daniele.brunello.dev@gmail.com

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.



