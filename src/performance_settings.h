/*
 * performance_settings.h
 *
 *  Created on: 11.08.2011
 *   Author(s): Martin Wimmer
 *     License: Ask author
 */

#ifndef PHEET_PERFORMANCE_SETTINGS_H_
#define PHEET_PERFORMANCE_SETTINGS_H_

namespace pheet {

#ifndef NDEBUG

bool const scheduler_count_tasks_at_level = true;
bool const scheduler_count_steal_calls_per_thread = true;
bool const scheduler_count_unsuccessful_steal_calls_per_thread = true;
bool const scheduler_count_spawns = true;
bool const scheduler_count_spawns_to_call = true;
bool const scheduler_count_calls = true;
bool const scheduler_count_finishes = true;
bool const stealing_deque_count_steals = true;
bool const stealing_deque_count_steal_calls = true;
bool const stealing_deque_count_unsuccessful_steal_calls = true;
bool const stealing_deque_count_pop_cas = true;
bool const scheduler_measure_total_time = true;
bool const scheduler_measure_task_time = true;
bool const scheduler_measure_sync_time = true;
bool const scheduler_measure_idle_time = true;
bool const scheduler_measure_queue_processing_time = true;
bool const scheduler_measure_visit_partners_time = true;
bool const scheduler_measure_wait_for_finish_time = true;
bool const scheduler_measure_wait_for_coordinator_time = true;

bool const task_storage_count_steals = true;
bool const task_storage_count_unsuccessful_pops = true;
bool const task_storage_count_successful_pops = true;
bool const task_storage_count_unsuccessful_takes = true;
bool const task_storage_count_successful_takes = true;
bool const task_storage_count_unsuccessful_steals = true;
bool const task_storage_count_successful_steals = true;
bool const task_storage_count_size_pop = true;
bool const task_storage_count_size_steal = true;
bool const task_storage_measure_pop_time = true;
bool const task_storage_measure_steal_time = true;

#else

bool const scheduler_count_tasks_at_level = false;
bool const scheduler_count_steal_calls_per_thread = false;
bool const scheduler_count_unsuccessful_steal_calls_per_thread = false;
bool const scheduler_count_spawns = false;
bool const scheduler_count_spawns_to_call = false;
bool const scheduler_count_calls = false;
bool const scheduler_count_finishes = false;
bool const stealing_deque_count_steals = false;
bool const stealing_deque_count_steal_calls = false;
bool const stealing_deque_count_unsuccessful_steal_calls = false;
bool const stealing_deque_count_pop_cas = false;
bool const scheduler_measure_total_time = false;
bool const scheduler_measure_task_time = false;
bool const scheduler_measure_sync_time = false;
bool const scheduler_measure_idle_time = false;
bool const scheduler_measure_queue_processing_time = false;
bool const scheduler_measure_visit_partners_time = false;
bool const scheduler_measure_wait_for_finish_time = false;
bool const scheduler_measure_wait_for_coordinator_time = false;

bool const task_storage_count_steals = false;
bool const task_storage_count_unsuccessful_pops = false;
bool const task_storage_count_successful_pops = false;
bool const task_storage_count_unsuccessful_takes = false;
bool const task_storage_count_successful_takes = false;
bool const task_storage_count_unsuccessful_steals = false;
bool const task_storage_count_successful_steals = false;
bool const task_storage_count_size_pop = false;
bool const task_storage_count_size_steal = false;
bool const task_storage_measure_pop_time = false;
bool const task_storage_measure_steal_time = false;

#endif

}

#endif /* PHEET_PERFORMANCE_SETTINGS_H_ */
