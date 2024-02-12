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

#ifndef EVENTS_TABLE_H_INCLUDED
#define EVENTS_TABLE_H_INCLUDED

// define events
typedef enum {
    ev_terminate_thread,
    ev_event1,
    ev_event2,
    ev_event3,
    ev_event4,
    ev_event5,
    ev_event6,
    // ... add here needed events ...
    ev_max
} event_id_t;

// define events group
typedef enum {
    events_group_threads,
    events_group_1,
    events_group_2,
    events_group_3,
    // ... add here needed group ...
    events_group_max
} events_group_t;

// define event info
typedef struct {
    event_id_t          id;             // don't use this for event data search, only for clarity in table definition
    events_group_t      group;          // group event belongs to
    char                *description;   // for event log, debug, ...
    // ... other data type relating to specific event ...
} events_table_item_t;

// export events data
extern events_table_item_t     events_table[ ev_max ];

#endif // EVENTS_TABLE_H_INCLUDED
