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

#include "events_table.h"

// events data
// NOTE be careful to keep events_group_t and event_id_t consistent with this table
events_table_item_t     events_table[ ev_max ] = {
    //  event id                            group                       description
    {   ev_terminate_thread,                events_group_threads,       "Thread termination"        },
    {   ev_event1,                          events_group_1,             "Event 1"                   },
    {   ev_event2,                          events_group_1,             "Event 2"                   },
    {   ev_event3,                          events_group_2,             "Event 3"                   },
    {   ev_event4,                          events_group_2,             "Event 4"                   },
    {   ev_event5,                          events_group_3,             "Event 5"                   },
    {   ev_event6,                          events_group_3,             "Event 6"                   },
    // ...
};
