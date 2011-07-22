/*
 * BasicMixedModeSchedulerTaskExecutionContext.h
 *
 *  Created on: 16.06.2011
 *   Author(s): Martin Wimmer
 *     License: Ask author
 */

#ifndef BASICMIXEDMODESCHEDULERTASKEXECUTIONCONTEXT_H_
#define BASICMIXEDMODESCHEDULERTASKEXECUTIONCONTEXT_H_

#include "../../../../settings.h"
#include "../../common/CPUThreadExecutor.h"
#include "../../../misc/atomics.h"
#include "../../../misc/bitops.h"
#include "../../../misc/type_traits.h"

#include <vector>
#include <queue>
#include <assert.h>
#include <iostream>

/*
 *
 */
namespace pheet {

union BasicMixedModeSchedulerTaskExecutionContextRegistration {
	uint64_t complete;
	struct {
	//	uint32_t c;	// counter
		uint32_t r;	// required threads
		uint32_t a;	// acquired threads
	} parts;
};

struct BasicMixedModeSchedulerTaskExecutionContextFinishStackElement {
	// Modified by local thread. Incremented when task is spawned, decremented when finished
	size_t num_spawned;

	// Only modified by other threads. Stolen tasks that were finished (including spawned tasks)
	size_t num_finished_remote;

	// Pointer to num_finished_remote of another thread (the one we stole tasks from)
	BasicMixedModeSchedulerTaskExecutionContextFinishStackElement* parent;
};

/*
 * Information about the next task to execute
 *
 * after retrieving the TeamTaskData element, countdown has to be atomically decremented
 * the last thread to decrement countdown has to either:
 * - set the pointer on the team-stack to NULL, signaling, that all threads in the team have started execution, or
 * - delete the previous task and decrement num_finished_remote in the parent stack element
 *
 * As long as the next pointer is NULL, there is more to come. (Other threads just have to wait.
 * 	If out of sync they can execute other tasks in the meantime)
 * A TeamTaskData element with a task pointer set to NULL is the last in the series. There, countdown
 * 	also has to be decremented, to make sure the previous task is finished correctly
 */
//
template <class Task, class StackElement>
struct BasicMixedModeSchedulerTaskExecutionContextTeamTaskData {
	// Task to execute
	Task* task;

	// Pointer to the next task to execute. set by coordinator
	BasicMixedModeSchedulerTaskExecutionContextTeamTaskData<Task, StackElement>* next;

	// Parent stack element needed for signaling finish
	StackElement* parent;

	// Threads count down as soon as they finish executing the task.
	// The last thread to count down is responsible for cleanup of the task (signal finish, delete task)
	// (Warning: after the last countdown it is not guaranteed that the TeamData hasn't been reused,
	// so even the deleting thread should get the task beforehand)
	// TODO: How do we know when all threads have read the next pointer? There is no real signal for that
	procs_t executed_countdown;

	// Size of the team to execute the task
	procs_t team_size;

	// Level of the team to execute the task
	procs_t team_level;

	// Whether a sync_team is required after execution of the task
	bool sync_required;
};

template <class TaskExecutionContext>
struct BasicMixedModeSchedulerTaskExecutionContextTeamAnnouncement {
	typename TaskExecutionContext::TeamTaskData* first_task;
	typename TaskExecutionContext::TeamTaskData* last_task;

	TaskExecutionContext* coordinator;
	BasicMixedModeSchedulerTaskExecutionContextTeamAnnouncement<TaskExecutionContext>* next_team;

	typename TaskExecutionContext::Registration reg;
	procs_t level;
};


struct BasicMixedModeSchedulerTaskExecutionContextTeamInfo {
	procs_t team_size;
	procs_t local_id;
	procs_t coordinator_id;
	procs_t team_level;

	// Only for non coordinating threads
	procs_t max_team_level;
};

template <class TaskExecutionContext>
struct BasicMixedModeSchedulerTaskExecutionContextLevelDescription {
	TaskExecutionContext** partners;
	procs_t num_partners;
	procs_t local_id;
	procs_t total_size;
	bool reverse_ids;
};

template <class TaskExecutionContext>
struct BasicMixedModeSchedulerTaskExecutionContextDequeItem {
	BasicMixedModeSchedulerTaskExecutionContextDequeItem();

	procs_t team_size;
	typename TaskExecutionContext::Task* task;
	typename TaskExecutionContext::StackElement* stack_element;

	bool operator==(BasicMixedModeSchedulerTaskExecutionContextDequeItem<TaskExecutionContext> const& other) const;
	bool operator!=(BasicMixedModeSchedulerTaskExecutionContextDequeItem<TaskExecutionContext> const& other) const;
};

template <class TaskExecutionContext>
BasicMixedModeSchedulerTaskExecutionContextDequeItem<TaskExecutionContext>::BasicMixedModeSchedulerTaskExecutionContextDequeItem()
: task(NULL), stack_element(NULL) {

}

template <class TaskExecutionContext>
bool BasicMixedModeSchedulerTaskExecutionContextDequeItem<TaskExecutionContext>::operator==(BasicMixedModeSchedulerTaskExecutionContextDequeItem<TaskExecutionContext> const& other) const {
	return other.task == task;
}

template <class TaskExecutionContext>
bool BasicMixedModeSchedulerTaskExecutionContextDequeItem<TaskExecutionContext>::operator!=(BasicMixedModeSchedulerTaskExecutionContextDequeItem<TaskExecutionContext> const& other) const {
	return other.task != task;
}


template <class TaskExecutionContext>
class nullable_traits<BasicMixedModeSchedulerTaskExecutionContextDequeItem<TaskExecutionContext> > {
public:
	static BasicMixedModeSchedulerTaskExecutionContextDequeItem<TaskExecutionContext> const null_value;
};

template <class TaskExecutionContext>
BasicMixedModeSchedulerTaskExecutionContextDequeItem<TaskExecutionContext> const nullable_traits<BasicMixedModeSchedulerTaskExecutionContextDequeItem<TaskExecutionContext> >::null_value;

template <class Scheduler, template <typename T> class StealingDeque>
class BasicMixedModeSchedulerTaskExecutionContext {
public:
	typedef BasicMixedModeSchedulerTaskExecutionContextRegistration Registration;
	typedef BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque> TaskExecutionContext;
	typedef BasicMixedModeSchedulerTaskExecutionContextLevelDescription<BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque> > LevelDescription;
	typedef typename Scheduler::Backoff Backoff;
	typedef typename Scheduler::CPUHierarchy CPUHierarchy;
	typedef typename Scheduler::Task Task;
	typedef BasicMixedModeSchedulerTaskExecutionContextFinishStackElement FinishStackElement;
	typedef BasicMixedModeSchedulerTaskExecutionContextDequeItem<BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque> > DequeItem;
	typedef BasicMixedModeSchedulerTaskExecutionContextTeamTaskData<Task, StackElement> TeamTaskData;
	typedef BasicMixedModeSchedulerTaskExecutionContextTeamInfo TeamInfo;
	typedef BasicMixedModeSchedulerTaskExecutionContextTeamAnnouncement<TaskExecutionContext> TeamAnnouncement;

	BasicMixedModeSchedulerTaskExecutionContext(vector<LevelDescription*> const* levels, vector<typename CPUHierarchy::CPUDescriptor*> const* cpus, typename Scheduler::State* scheduler_state);
	~BasicMixedModeSchedulerTaskExecutionContext();

	void join();

	template<class CallTaskType, typename ... TaskParams>
		void finish(TaskParams ... params);

	template<class CallTaskType, typename ... TaskParams>
		void call(TaskParams ... params);

	template<class CallTaskType, typename ... TaskParams>
		void spawn(TaskParams ... params);

	template<class CallTaskType, typename ... TaskParams>
		void finish_nt(procs_t nt_size, TaskParams ... params);

	template<class CallTaskType, typename ... TaskParams>
		void call_nt(procs_t nt_size, TaskParams ... params);

	template<class CallTaskType, typename ... TaskParams>
		void spawn_nt(procs_t nt_size, TaskParams ... params);

	// If team is out of sync
	void sync_team();
	void team_barrier();

	procs_t get_global_id();
	procs_t get_local_id();

private:
	void run();
	void execute_task(Task* task, StackElement* parent);
	void main_loop();
	void process_queue();
	void empty_stack(size_t limit);

	void build_team(procs_t nt_size);

	void visit_partners(procs_t level_limit);

	void follow_coordinator();

	procs_t get_level_for_num_threads(procs_t num_threads);

	// Stack is only used by the coordinator
	static size_t const stack_size;
	FinishStackElement* stack;
	size_t finish_stack_filled;
	// Block the finish stack from being emptied below this point (needed for blocking finish)
	size_t finish_stack_block;

	// machine hierarchy
	LevelDescription* levels;
	procs_t num_levels;
	procs_t* level_map;

	// Team task information
	TeamTaskData* current_team_task;
	TeamAnnouncement* current_team;
	TeamAnnouncement** announced_teams;

	procs_t team_announcement_index;

	// Information needed for task reclamation scheme
	queue<TeamTaskData*> team_task_reclamation_queue;
	TeamTaskData** team_task_reuse;

	// Information needed for team reclamation scheme
	queue<TeamAnnouncement*> team_announcement_reclamation_queue;
	TeamAnnouncement** team_announcement_reuse;

	// for finishing
	TeamTaskData const* waiting_for_finish;

	// Information about the team (calculated by all threads)
	TeamInfo* team_info;

	// Maximum level at which we stay in the team (for current coordinator)
	procs_t max_team_level;

	// depth of the stack (including the coordinators stacks) - team tasks only
	size_t depth;

	CPUThreadExecutor<typename CPUHierarchy::CPUDescriptor, BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque> > thread_executor;

	typename Scheduler::State* scheduler_state;

	StealingDeque<DequeItem> stealing_deque;

	friend class CPUThreadExecutor<typename CPUHierarchy::CPUDescriptor, BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>>;
};

template <class Scheduler, template <typename T> class StealingDeque>
size_t const BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::stack_size = 256;

template <class Scheduler, template <typename T> class StealingDeque>
size_t const BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::team_stack_size = 256;

template <class Scheduler, template <typename T> class StealingDeque>
size_t const BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::local_team_stack_size = 256;

template <class Scheduler, template <typename T> class StealingDeque>
BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::BasicMixedModeSchedulerTaskExecutionContext(vector<LevelDescription*> const* levels, vector<typename CPUHierarchy::CPUDescriptor*> const* cpus, typename Scheduler::State* scheduler_state)
: finish_stack_filled(0), finish_stack_block(0), num_levels(levels->size()), thread_executor(cpus, this), scheduler_state(scheduler_state), stealing_deque(find_last_bit_set((*levels)[0]->total_size - 1) << 4), team_announcement_index(0), depth(0), in_sync(true), all_joined(true), waiting_for_finish(NULL), current_team_task(NULL), current_team(NULL) {
	stack = new StackElement[stack_size];
	this->levels = new LevelDescription[num_levels];
	procs_t local_id = 0;
	for(ptrdiff_t i = num_levels - 1; i >= 0; i--) {
		this->levels[i].partners = (*levels)[i]->partners;
		this->levels[i].num_partners = (*levels)[i]->num_partners;
		local_id += (*levels)[i]->local_id;
		this->levels[i].local_id = local_id;
		this->levels[i].total_size = (*levels)[i]->total_size;
		this->levels[i].reverse_ids = (*levels)[i]->reverse_ids;
	}

	// Create map for simple lookup of levels in the hierarchy (if we have a number of threads)
	procs_t max_levels = find_last_bit_set(this->levels[0].total_size - 1) + 1;
	level_map = new procs_t[max_levels];
	procs_t lvl = num_levels - 1;
	level_map[0] = num_levels - 1;
	for(procs_t i = 1; i < max_levels - 1; i++) {
		procs_t min_size = (1 << (i - 1)) + 1;
		procs_t lvl_size = this->levels[lvl].total_size;
		while(min_size > lvl_size) {
			--lvl;
			lvl_size = this->levels[lvl].total_size;
		}
		level_map[i] = lvl;
	}
	level_map[max_levels - 1] = 0;

	// Initialize stuff for memory reuse
	announced_teams = new TeamAnnouncement*[num_levels];
	team_task_reuse = new TeamTaskData*[num_levels - 1];
	team_announcement_reuse = new TeamAnnouncement*[num_levels];
	for(procs_t i = 0; i < num_levels; ++i) {
		announced_teams[i] = NULL;

		team_task_reuse[i] = new TeamTaskData();
		team_announcement_reuse[i] = new TeamAnnouncement();
	}
	team_announcement_reuse[num_levels - 1]->level = num_levels - 1;

	// Initialize team_info structure
	team_info = new TeamInfo();

	// Run thread
	thread_executor.run();
}

template <class Scheduler, template <typename T> class StealingDeque>
BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::~BasicMixedModeSchedulerTaskExecutionContext() {
	delete[] stack;
	delete[] levels;
	delete[] announced_team_tasks;
	delete[] announced_coordinators;
	delete[] level_map;

	delete team_info;

	TeamTaskData* tmp;

	while((tmp = reclamation_queue.pop()) != NULL) {
		delete tmp;
	}

	for(procs_t i = 0; i < num_levels; ++i) {
		delete team_task_reuse[i];
	}
	delete[] team_task_reuse;
}

template <class Scheduler, template <typename T> class StealingDeque>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::join() {
	thread_executor.join();
}


template <class Scheduler, template <typename T> class StealingDeque>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::run() {
	scheduler_state->state_barrier.wait(0, 1);

	Task* startup_task = scheduler_state->startup_task;
	if(startup_task != NULL && PTR_CAS(&(scheduler_state->startup_task), startup_task, NULL)) {
		FinishStackElement* root = start_finish_region(NULL);

		DequeItem di;
		di.stack_element = NULL;
		di.task = startup_task;
		di.team_size = scheduler_state->team_size;

		create_team(di.team_size);

		if(di.team_size == 1) {
			execute_solo_queue_task(di);
		}
		else {
			execute_queue_task(di);
		}

		coordinate_team();

		wait_for_finish(root);

		scheduler_state->current_state = 2; // finished
	}
	else {
		wait_for_shutdown();
	}

	scheduler_state->state_barrier.barrier(1, levels[0].total_size);

	// Now we can safely finish execution
}

/*
 * Do work until the scheduler shuts down
 */
template <class Scheduler, template <typename T> class StealingDeque>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::wait_for_shutdown() {
	// pre-condition: queue must be empty
	assert(stealing_deque.is_empty());

	while(scheduler_state->current_state != 2) {
		// Try to join teams or steal tasks
		visit_partners();

		// Coordinate the current team if existing until it's empty
		coordinate_team();

		while(has_local_work()) {
			// Execute a single task. This will create a team if there was a task
			if(execute_next_queue_task()) {
				// Coordinate the current team if existing until it's empty
				coordinate_team();
			}
		}
	}
}

/*
 * Do work until the task has been finished
 */
template <class Scheduler, template <typename T> class StealingDeque>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::wait_for_finish(FinishStackElement* parent) {
	size_t prev_finish_stack_block = finish_stack_block;

	// Make sure this finish_stack_element isn't removed until we notice the finish
	finish_stack_block = (parent - finish_stack) + 1;

	while(parent->num_finished_remote != parent->num_spawned) {
		// TODO: try a policy where we do not need to empty our queues before we notice the finish
		// (currently not implemented for simplicity)

		while(has_local_work()) {
			// Execute a single task. This will create a team if there was a task
			if(execute_next_queue_task()) {
				// Coordinate the current team if existing until it's empty
				coordinate_team();
			}
		}

		// Try to join teams or steal tasks
		visit_partners_until_finished(parent);

		// Coordinate the current team if existing until it's empty
		coordinate_team();
	}

	finish_stack_block = prev_finish_stack_block;
}

/*
 * Do work until the current team is synchronized
 */
template <class Scheduler, template <typename T> class StealingDeque>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::wait_for_sync() {
	// TODO
}

/*
 * Do work until the task has been finished
 */
template <class Scheduler, template <typename T> class StealingDeque>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::wait_for_coordinator_finish(TeamTaskData const* parent_task) {
	TeamTaskData* prev_waiting_for_finish = waiting_for_finish;
	waiting_for_finish = parent_task;

	follow_team();

	waiting_for_finish = prev_waiting_for_finish;
}

/*
 * Coordinates a team until we run out of work for the team
 */
template <class Scheduler, template <typename T> class StealingDeque>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::coordinate_team() {
	if(current_team != NULL) {
		if(current_team->level == num_levels - 1) {
			// Solo team
			DequeItem di = get_next_team_task();
			while(di.task != NULL) {
				// Execute
				execute_solo_queue_task(di);

				// Try to get a same-size task
				di = get_next_team_task();
			}
			current_team = NULL;
		}
		else {
			DequeItem di = get_next_team_task();
			while(di.task != NULL) {
				// This method is responsible for creating the team task data and finish stack elements, etc...
				TeamTaskData* tt = create_team_task(di);

				// Show it to the rest of the team
				announce_next_team_task(tt);

				// Execute (same for coor and client!)
				execute_team_task(di);

				// Try to get a same-size task
				di = get_next_team_task();
			}
			disband_team();

	}
}

template <class Scheduler, template <typename T> class StealingDeque>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::disband_team() {
	assert(current_team != NULL);
	assert(current_team->level != num_levels - 1);
	assert(current_team->coordinator == this);

	// Put the old team into memory reclamation (as soon as it will be retrieved from reclamation,
	// it is guaranteed that no other (relevant) threads still have a reference to this
	team_announcement_reclamation_queue.push(current_team);

	// Create a dummy team with a single thread - Other threads will see this team and exit
	create_team(1);

	// Finally, drop out of this team
	current_team = NULL;
}

/*
 * Finds a single task, creates a team for it and executes the task
 */
template <class Scheduler, template <typename T> class StealingDeque>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::execute_next_queue_task() {
	DequeItem di = get_next_queue_task();

	if(di.task != NULL) {
		create_team(di.team_size);

		if(di.team_size == 1) {
			execute_solo_queue_task(di);
		}
		else {
			execute_queue_task(di);
		}
		return true;
	}
	return false;
}

/*
 * executes the given task (if it is a team task, it is announced for the team
 */
template <class Scheduler, template <typename T> class StealingDeque>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::execute_queue_task(DequeItem const& di) {
/*	if(di.team_size == 1) {
		execute_solo_task(di);
	}
	else {*/
	assert(di.team_size > 1);

	// This method is responsible for creating the team task data and finish stack elements, etc...
	TeamTaskData* tt = create_team_task(di);

	// Show it to the rest of the team
	announce_first_team_task(tt);

	// Execute (same for coor and client!)
	execute_team_task(tt);

	// Send task to memory reclamation
	reclamation_queue.push(current_team_task);
//	}
}

/*
 * executes the given task (if it is a team task, it is announced for the team
 */
template <class Scheduler, template <typename T> class StealingDeque>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::execute_solo_queue_task(DequeItem const& di) {
	assert(di.team_size == 1);

	// Execute task
	(*di.task)(*this);

	// signal that we finished execution of the task
	signal_task_completion(di.stack_element);
}

template <class Scheduler, template <typename T> class StealingDeque>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::execute_team_task(TeamTaskData const* team_task) {
	// Execute task
	(*task)(*this);

	// signal that we finished execution of the task
	signal_task_completion(team_task->parent);
}

template <class Scheduler, template <typename T> class StealingDeque>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::create_team(procs_t team_size) {
	if(team_size == 1) {
		TeamAnnouncement* team = team_announcement_reuse[num_levels - 1];
		prepare_team_info(team);
		if(current_team != NULL) {
			// We have to make sure the team is synced first
			// New team smaller: make sure all threads joined the team (last chance for getting those threads in)
			sync_team();

			current_team->next_team = team;
			announced_teams[team_announcement_index] = NULL;

			current_team = team;
		}
		return team;
	}
	else {
		procs_t level = get_level_for_num_threads(team_size);
		TeamAnnouncement* team;

		if(!team_announcement_reclamation_queue.empty() && team_announcement_reclamation_queue.front()->reg.r == team_announcement_reclamation_queue.front()->reg.a) {
			TeamAnnouncement* tmp = team_announcement_reclamation_queue.pop();
			team = team_task_reuse[level];
			team_announcement_reuse[level] = tmp;
		}
		else {
			team = new TeamAnnouncement();
		}

		team->coordinator = this;
		team->first_task = NULL;
		team->last_task = NULL;
		team->level = level;
		team->next_team = NULL;
		team->reg.r = team_size;
		if(current_team == NULL) {
			team->reg.a = 1;
		}
		else {
			// We have to make sure the team is synced first
			// New team smaller: make sure all threads joined the team (last chance for getting those threads in)
			// New team larger: Make sure all threads saw the announcement
			sync_team();

			if(current_team->level > team_level) {
				team->reg.a = current_team->reg.r;
			}
			else {
				// Synchronized from the start - perfect
				team->reg.a = team_size;
			}
		}

		// TODO: If we have to sync the team, we can drop the fence
		MEMORY_FENCE();

		if(current_team != NULL) {

			if(current_team->level > team_level) {
				// We now need more threads so we need to reannounce.
				announced_teams[team_announcement_index] = team;
			}
			else {
				assert(current_team->level != team_level);
				// This team is smaller so no need for reannouncement
				announced_teams[team_announcement_index] = NULL;
			}
			current_team->next_team = team;
		}
		else {
			announced_teams[team_announcement_index] = team;
		}
		current_team = team;

		prepare_team_info(team);
	}
}

/*
 * Creates a new team task with the given parameters.
 * No memory_fence is provided, so if you want to announce it directly afterwards you need to use a fence
 *
 * Do not create single-threaded tasks with this
 */
template <class Scheduler, template <typename T> class StealingDeque>
TeamTaskData* BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::create_team_task(DequeItem di) {
	TeamTaskData* team_task;

	assert(nt_size != 1);

	if(!reclamation_queue.empty() && reclamation_queue.front()->executed_countdown == 0) {
		TeamTaskData* tmp = reclamation_queue.pop();
		team_task = team_task_reuse[tmp->team_level];
		team_task_reuse[tmp->team_level] = tmp;
	}
	else {
		team_task = new TeamTaskData();
	}

	FinishStackElement* parent = di.stack_element;
	if(parent >= stack && (parent < (stack + stack_size))) {
		// to prevent thrashing on the parent finish block, we create a new finish block local to the coordinator
		// TODO: evaluate whether we can let the coordinator update the finish block and just do perform some checks
		// when it should be completed

		// Perform cleanup on finish stack
		empty_finish_stack();

		// Create new stack element for finish
		parent = start_finish_region(parent);
	}

	team_task->team_level = current_team->level;
	team_task->team_size = levels[current_team->level].total_size; //nt_size;
	team_task->task = di.task;
	team_task->parent = parent;
	team_task->next = NULL;
	team_task->executed_countdown = levels[team_level].total_size;

	// Make sure all changes are written before it will be published
	MEMORY_FENCE();

	return team_task;
}

template <class Scheduler, template <typename T> class StealingDeque>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::announce_first_team_task(TeamTaskData* team_task) {
	current_team->first_task = team_task;
}

template <class Scheduler, template <typename T> class StealingDeque>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::announce_next_team_task(TeamTaskData* team_task) {
	current_team_task->next = team_task;
}

template <class Scheduler, template <typename T> class StealingDeque>
FinishStackElement* BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::start_finish_region(FinishStackElement* parent) {
	assert(finish_stack_filled < finish_stack_size);

	finish_stack[finish_stack_filled].num_finished_remote = 0;
	// As we create it out of a parent region that is waiting for completion of a single task, we can already add this one task here
	finish_stack[finish_stack_filled].num_spawned = 1;
	finish_stack[finish_stack_filled].parent = parent;

	++finish_stack_filled;

	return &(finish_stack[finish_stack_filled - 1]);
}

/*
 * empty stack but not below limit
 */
template <class Scheduler, template <typename T> class StealingDeque>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::empty_finish_stack() {
	while(finish_stack_filled > finish_stack_block) {
		size_t se = stack_filled - 1;
		if(finish_stack[se].num_spawned == finish_stack[se].num_finished_remote) {
			finalize_finish_stack_element(se);

			finish_stack_filled = se;

			// When parent is set to NULL, some thread is finalizing/has finalized this stack element (otherwise we would have an error)
			assert(finish_stack[finish_stack_filled].parent == NULL);
		}
		else {
			break;
		}
	}
}

template <class Scheduler, template <typename T> class StealingDeque>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::signal_task_completion(FinishStackElement* stack_element) {
	FinishStackElement* parent = stack_element->parent;

	if(stack_element >= stack && (stack_element < (stack + stack_size))) {
		--(stack_element->num_spawned);

		// Memory fence is unfortunately required to guarantee that some thread finalizes the finish_stack_element
		// TODO: prove that the fence is enough
		MEMORY_FENCE();
	}
	else {
		// Increment num_finished_remote of parent
		SIZET_ATOMIC_ADD(&(stack_element->num_finished_remote), 1);
	}
	if(stack_element->num_spawned == stack_element->num_finished_remote) {
		finalize_finish_stack_element(stack_element, parent);
	}
}

template <class Scheduler, template <typename T> class StealingDeque>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::finalize_finish_stack_element(FinishStackElement* element, FinishStackElement* parent) {
	if(parent != NULL) {
		if(element->num_spawned == 0) {
			// No tasks processed remotely - no need for atomic ops
			element->parent = NULL;
			signal_task_completion(parent);
		}
		else {
			if(PTR_CAS(&(element->parent), parent, NULL)) {
				signal_task_completion(parent);
			}
		}
	}
}

/*
 * Stealing routine for idle threads
 */
template <class Scheduler, template <typename T> class StealingDeque>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::visit_partners() {
	Backoff bo;
	DequeItem di;
	while(true) {
		procs_t next_rand = random();

		// We do not steal from the last level as there are no partners
		procs_t level = num_levels - 1;
		while(level > 0) {
			--level;
			// For all except the last level we assume num_partners > 0
			assert(levels[level].num_partners > 0);
			ThreadExecutionContext* partner = levels[level].partners[next_rand % levels[level].num_partners];
			assert(partner != this);

			TeamAnnouncement* team = find_partner_team(partner, level);
			if(team != NULL) {
				// Joins the team and executes all tasks. Only returns after the team has been disbanded
				join_team(team);
				return;
			}

			di = steal_tasks_from_partner(partner, level + 1);
		//	di = levels[level].partners[next_rand % levels[level].num_partners]->stealing_deque.steal();

			if(di.task != NULL) {
				create_team(di.team_size);

				execute_queue_task(di);
				return;
			}
		}
		if(scheduler_state->current_state >= 2) {
			return;
		}
		bo.backoff();
	}
}

/*
 * Stealing routine for (coordinating) threads waiting for a finish
 */
template <class Scheduler, template <typename T> class StealingDeque>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::visit_partners_until_finished(FinishStackElement* parent) {
	Backoff bo;
	DequeItem di;
	while(true) {
		procs_t next_rand = random();

		// We do not steal from the last level as there are no partners
		procs_t level = num_levels - 1;
		while(level > 0) {
			--level;
			// For all except the last level we assume num_partners > 0
			assert(levels[level].num_partners > 0);
			ThreadExecutionContext* partner = levels[level].partners[next_rand % levels[level].num_partners];
			assert(partner != this);

			TeamAnnouncement* team = find_partner_team(partner, level);
			if(team != NULL) {
				// Joins the team and executes all tasks. Only returns after the team has been disbanded
				join_team(team);
				return;
			}

			di = steal_tasks_from_partner(partner, level + 1);
		//	di = levels[level].partners[next_rand % levels[level].num_partners]->stealing_deque.steal();

			if(di.task != NULL) {
				create_team(di.team_size);

				execute_queue_task(di);
				return;
			}
		}
		if(parent->num_finished_remote == parent->num_spawned) {
			return;
		}
		bo.backoff();
	}
}

/*
 * Checks if the partner has a relevant team for this thread
 * This method assumes we are not bound to a team, another more complex method (with tie-breaking, etc.) is used during team sync
 */
template <class Scheduler, template <typename T> class StealingDeque>
TeamAnnouncement* BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::find_partner_team(ThreadExecutionContext* partner, procs_t level) {
	TeamAnnouncement* team = partner->announced_teams[team_announcement_index];
	if(team != NULL && team->level <= level) {
		// Make sure we haven't found an old announcement (in that case the team must already be fully synced)
		if(team->reg.a == team->reg.r) {
			// Old announcement - leave it
			return NULL;
		}
		return team;
	}
	return NULL;
}

/*
 * Joins the team and executes all tasks. Only returns after the team has been disbanded
 *
 * TODO: Does not take into account conflicting team announcements, etc.
 */
template <class Scheduler, template <typename T> class StealingDeque>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::join_team(TeamAnnouncement* team) {
	// Announce the team so it is visible to others
	announced_teams[team_announcement_index] = team;

	// Registration
	{
		Registration reg, old_reg;
		Backoff bo;
		reg.complete = team->reg.complete;
		old_reg.complete = reg.complete;
		++reg.parts.a;
		while(!UINT64_CAS(&(team->reg.complete), old_reg.complete, reg.complete)) {
			bo.backoff();
			reg.complete = team->reg.complete;
			old_reg.complete = reg.complete;
			++reg.parts.a;
		}
	}

	// Calculate the team_level at which we have to drop out
	{
		assert(levels[team->level].local_id != coor->levels[team->level].local_id);

		TaskExecutionContext* smaller;
		TaskExecutionContext* larger;
		if(levels[team->level].local_id < coor->levels[team->level].local_id) {
			smaller = this;
			larger = coor;
		}
		else {
			smaller = coor;
			larger = this;
		}
		procs_t diff = larger->levels[team->level].local_id - smaller->levels[team->level].local_id;
		procs_t lvl = ti.team_level + 1;
		while(smaller->levels[lvl].local_id + diff == larger->levels[lvl].local_id) {
			++lvl;
		}
		max_team_level = lvl - 1;
	}

	current_team = team;
	assert(current_team->level <= max_team_level);

	follow_team();
}

/*
 * Joins the team and executes all tasks. Only returns after the team has been disbanded
 *
 * TODO: Does not take into account conflicting team announcements, etc.
 */
template <class Scheduler, template <typename T> class StealingDeque>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::follow_team() {
	while(current_team->level <= max_team_level) {
		{
			Backoff bo;
			while(current_team->first_task == NULL) {
				bo.backoff();
			}
			current_team_task = current_team->first_task;

			execute_team_task(current_team_task);
		}

		while(current_team->last_task != current_team_task) {
			Backoff bo;
			while(current_team->next == NULL && current_team->last_task != current_team_task) {
				bo.backoff();
			}
			if(current_team->next == NULL) {
				break;
			}
			current_team_task = current_team_task->next;
			if(current_team_task == to_finish) {
				return;
			}

			execute_team_task(current_team_task);
		}

		{
			// We have to make sure the team is synced first
			// New team smaller: Make sure all threads joined the previous team (last chance for getting those threads in)
			// New team larger: Make sure all threads saw the previous announcement
			sync_team();

			// Wait for next team to appear
			Backoff bo;
			while(current_team->next_team == NULL) {
				bo.backoff();
			}
			// Announce new team if necessary
			if(current_team->level > current_team->next_team->level) {
				team_announcements[team_announcement_index] = current_team->next_team;
			}
			else {
				team_announcements[team_announcement_index] = NULL;
			}

			// Put the old team into memory reclamation (as soon as it will be retrieved from reclamation,
			// it is guaranteed that no other (relevant) threads still have a reference to this
			team_announcement_reclamation_queue.push(current_team);

			// Update team information
			current_team = current_team->next_team;
			prepare_team_info(current_team);
		}
	}
}

/*
 * Calculates all information needed for the team
 */
template <class Scheduler, template <typename T> class StealingDeque>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::prepare_team_info(TeamAnnouncement* team) {
	team_info->team_level = team->level;
	if(coor->levels[level].reverse_ids) {
		procs_t offset = coor->levels[level].total_size - 1;
		team_info->coordinator_id = offset - coor->levels[level].coordinator_id;
		team_info->local_id = offset - levels[level].local_id;
	}
	else {
		team_info->coordinator_id = coor->levels[level].coordinator_id;
		team_info->local_id = levels[level].local_id;
	}
}

/*
 * Performs a synchronization of the team
 * After sync_team it is ensured, that all threads necessary for the team are working in the team
 */
template <class Scheduler, template <typename T> class StealingDeque>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::sync_team() {
	// TODO
}

/**
 * translate a number of threads to a level in the CPU hierarchy
 */
template <class Scheduler, template <typename T> class StealingDeque>
procs_t BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::get_level_for_num_threads(procs_t num_threads) {
	assert(num_threads > 0);

	if(num_threads > levels[0].total_size) {
		return 0;
	}
	procs_t candidate = level_map[find_last_bit_set(num_threads - 1)];
	// With perfect binary trees (e.g. if number of processors is a power of two) this loop is never executed
	// On sane hierarchies, this loop is executed at most once
	while(levels[candidate].total_size < num_threads) {
		--candidate;
	}
	return candidate;
}

template <class Scheduler, template <typename T> class StealingDeque>
bool BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::is_coordinator() {
	return team_info->coordinator_id == team_info->local_id;
}

template <class Scheduler, template <typename T> class StealingDeque>
procs_t BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::get_local_id() {
	return team_info->local_id;
}

template <class Scheduler, template <typename T> class StealingDeque>
procs_t BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::get_coordinator_id() {
	return team_info->coordinator_id;
}

template <class Scheduler, template <typename T> class StealingDeque>
procs_t BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::get_global_id() {
	return levels[0].local_id;
}

template <class Scheduler, template <typename T> class StealingDeque>
procs_t BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::get_team_size() {
	return team_info->team_size;
}

template <class Scheduler, template <typename T> class StealingDeque>
template<class CallTaskType, typename ... TaskParams>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::spawn(TaskParams ... params) {
	// TODO: optimization to use call in some cases to prevent the stack from growing too large

	// TODO: let tasks be spawned by multiple threads

	if(is_coordinator()) {
		CallTaskType* task = new CallTaskType(params ...);
		assert(stack_filled > 0);
		current_task->parent.num_spawned++;
		DequeItem di;
		di.task = task;
		di.stack_element = current_task->parent;
		di.team_size = team_info->team_size;
		store_item_in_queue(di, team_info->team_level);
	}
}

template <class Scheduler, template <typename T> class StealingDeque>
template<class CallTaskType, typename ... TaskParams>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::call(TaskParams ... params) {
	// TODO
	/*
	if(is_coordinator) {
		assert(stack_filled > 0);

		if(team_size == 1) {
			CallTaskType task(params ...);
			call_team_task(&task);
		}
		else {
			CallTaskType* task = new CallTaskType(params ...);
			call_task(team_size, task, &(stack[stack_filled - 1]));
		}
	}
	else {
		join_coordinator_subteam();
	}*/
}

template <class Scheduler, template <typename T> class StealingDeque>
template<class CallTaskType, typename ... TaskParams>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::finish(TaskParams ... params) {
	// TODO

/*	if(is_coordinator()) {
		assert(stack_filled > 0);

		if(team_size == 1) {
			// Create task
			CallTaskType task(params ...);

			// Create a new stack element for new task
			// num_finished_remote is not required as this stack element blocks lower ones from finishing anyway
			finish_team_task(&task, NULL);
		}
		else {
			CallTaskType* task = new CallTaskType(params ...);

			// Create a new stack element for new task
			// num_finished_remote is not required as this stack element blocks lower ones from finishing anyway
			finish_task(team_size, task, NULL);
		}
	}
	else {
		join_coordinator_subteam();
	}*/
}

template <class Scheduler, template <typename T> class StealingDeque>
template<class CallTaskType, typename ... TaskParams>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::spawn_nt(procs_t nt_size, TaskParams ... params) {
	// TODO: optimization to use call in some cases to prevent the stack from growing too large

	// TODO: let tasks be spawned by multiple threads

	if(is_coordinator()) {
		CallTaskType* task = new CallTaskType(params ...);
		assert(stack_filled > 0);
		current_task->parent.num_spawned++;
		DequeItem di;
		di.task = task;
		di.stack_element = current_task->parent;
		di.team_size = nt_size;
		store_item_in_queue(di, get_level_for_num_threads(nt_size));
	}
}

template <class Scheduler, template <typename T> class StealingDeque>
template<class CallTaskType, typename ... TaskParams>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::call_nt(procs_t nt_size, TaskParams ... params) {
	// TODO
	/*
	if(is_coordinator()) {
		assert(stack_filled > 0);

		if(team_size == 1) {
			CallTaskType task(params ...);
			call_team_task(&task);
		}
		else {
			CallTaskType* task = new CallTaskType(params ...);
			call_task(team_size, task, &(stack[stack_filled - 1]));
		}
	}
	else {
		join_coordinator_subteam(nt_size);
	}*/
}

template <class Scheduler, template <typename T> class StealingDeque>
template<class CallTaskType, typename ... TaskParams>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::finish_nt(procs_t nt_size, TaskParams ... params) {
	// TODO
		/*
	if(is_coordinator()) {
		assert(stack_filled > 0);

		if(nt_size == 1) {
			// Create task
			CallTaskType task(params ...);

			// Create a new stack element for new task
			// num_finished_remote is not required as this stack element blocks lower ones from finishing anyway
			finish_team_task(&task, NULL);
		}
		else {
			CallTaskType* task = new CallTaskType(params ...);

			// Create a new stack element for new task
			// num_finished_remote is not required as this stack element blocks lower ones from finishing anyway
			finish_task(nt_size, task, NULL);
		}
	}
	else {
		join_coordinator_subteam(nt_size);
	}*/
}

/*
 * Checks whether there is still some local work that we can execute
 */
template <class Scheduler, template <typename T> class StealingDeque>
bool BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::has_local_work() {
	// TODO
}

/*
 * Get a task from the local queues that is suited for the current team
 */
template <class Scheduler, template <typename T> class StealingDeque>
DequeItem BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::get_next_team_task() {
	// TODO
}

/*
 * Get any task from the local queues
 */
template <class Scheduler, template <typename T> class StealingDeque>
DequeItem BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::get_next_queue_task() {
	// TODO
}

/*
 * Steal tasks from the given partner, but only those where the level is smaller than the given level
 */
template <class Scheduler, template <typename T> class StealingDeque>
DequeItem BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::steal_tasks_from_partner(TaskExecutionContext* partner, procs_t min_level) {
	// TODO
}

// -------------------------------- old code -----------------------------------------------





/*
template <class Scheduler, template <typename T> class StealingDeque>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::build_team(SyncStackElement* sync, uint32_t c) {
	Registration reg;
	Backoff bo;
	reg.complete = sync->reg.complete;
	procs_t level = local_team_info->team_level;
	while(reg.parts.c == c && reg.parts.r != reg.parts.a) {
		if(!process_queue_task(level + 1, sync, c)) {
			if(!visit_team_partners(level, sync, c)) {
				bo.backoff();
			}
		}

		reg.complete = sync.reg.complete;
	}
}

template <class Scheduler, template <typename T> class StealingDeque>
bool BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::process_queue_task(procs_t level_limit, SyncStackElement* sync, uint32_t c) {
	if(has_local_task(level_limit)) {
		if(!deregister_from_team(sync, c)) {
			return true;
		}

		DequeItem di = get_next_local_task(level_limit);
		if(di.task != NULL) {
			// Warning, no distinction between locally spawned tasks and remote tasks
			// But this makes it easier with the finish construct, etc.
			// Otherwise we would have to empty our deque on the next finish call
			// which is bad for balancing
			execute_queue_task(di.team_size, di.task, di.stack_element);

			register_for_team(sync);
			return true;
		}
		register_for_team(sync);
	}
	return false;
}

template <class Scheduler, template <typename T> class StealingDeque>
bool BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::visit_team_partners(procs_t level_limit, SyncStackElement* sync, uint32_t c) {
	procs_t next_rand = random();

	// We do not steal from the last level as there are no partners
	procs_t level = num_levels - 1;
	while(level > level_limit) {
		--level;
		// For all except the last level we assume num_partners > 0
		assert(levels[level].num_partners > 0);
		ThreadExecutionContext* partner = levels[level].partners[next_rand % levels[level].num_partners];
		assert(partner != this);

		TeamInfo* team = find_partner_team(partner, level);
		if(team != NULL) {
			if(!deregister_from_team(sync, c)) {
				return true;
			}
			join_partner_team(team);
			follow_coordinator();

			register_for_team(sync);
			return true;
		}

		if(partner->has_local_task(level_limit)) {
			if(!deregister_from_team(sync, c)) {
				return true;
			}

			di = steal_tasks_from_partner(partner, level_limit);
		//	di = levels[level].partners[next_rand % levels[level].num_partners]->stealing_deque.steal();

			if(di.task != NULL) {
				execute_queue_task(di.team_size, di.task, di.stack_element);

				register_for_team(sync);
				return true;
			}

			register_for_team(sync);
		}
	}
	return false;
}

template <class Scheduler, template <typename T> class StealingDeque>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::register_for_team(SyncStackElement* sync) {
	// We can assume that we only get to register for teams we are definitely eligible for and that those teams are still
	// being built (because without this thread the team can't be built, and team building is never cancelled)
	Registration reg;
	Registration old_reg;
	Backoff bo;

	reg.complete = sync->reg.complete;
	old_reg.complete = reg.complete;

	++reg.parts.a;
	if(reg.parts.a == reg.parts.r) {
		++reg.parts.c;
	}

	while(!UINT64_CAS(&(sync->reg.complete), old_reg.complete, reg.complete)) {
		bo.backoff();

		reg.complete = sync->reg.complete;
		old_reg.complete = reg.complete;

		++reg.parts.a;

		if(reg.parts.a == reg.parts.r) {
			++reg.parts.c;
		}
	}
}

template <class Scheduler, template <typename T> class StealingDeque>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::deregister_from_team(SyncStackElement* sync, uint32_t c) {
	// If c hasn't changed and team isn't jetzt built we can deregister

	Registration reg;
	Registration old_reg;
	Backoff bo;

	reg.complete = sync->reg.complete;
	if(reg.parts.c != c) {
		// Team has already been built
		return false;
	}
	old_reg.complete = reg.complete;

	--reg.parts.a;

	while(!UINT64_CAS(&(sync->reg.complete), old_reg.complete, reg.complete)) {
		bo.backoff();

		reg.complete = sync->reg.complete;
		if(reg.parts.c != c) {
			// Team has already been built
			return false;
		}
		old_reg.complete = reg.complete;

		--reg.parts.a;
	}
	return true;
}
*/
/*
template <class Scheduler, template <typename T> class StealingDeque>
template<class CallTaskType, typename ... TaskParams>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::finish(TaskParams ... params) {
	if(is_coordinator()) {
		assert(stack_filled > 0);

		if(team_size == 1) {
			// Create task
			CallTaskType task(params ...);

			// Create a new stack element for new task
			// num_finished_remote is not required as this stack element blocks lower ones from finishing anyway
			finish_team_task(&task, NULL);
		}
		else {
			CallTaskType* task = new CallTaskType(params ...);

			// Create a new stack element for new task
			// num_finished_remote is not required as this stack element blocks lower ones from finishing anyway
			finish_task(team_size, task, NULL);
		}
	}
	else {
		join_coordinator_subteam();
	}
}

template <class Scheduler, template <typename T> class StealingDeque>
template<class CallTaskType, typename ... TaskParams>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::spawn(TaskParams ... params) {
	// TODO: optimization to use call in some cases to prevent the stack from growing too large

	// TODO: let tasks be spawned by multiple threads

	if(is_coordinator()) {
		CallTaskType* task = new CallTaskType(params ...);
		assert(stack_filled > 0);
		stack[stack_filled - 1].num_spawned++;
		DequeItem di;
		di.task = task;
		di.stack_element = &(stack[stack_filled - 1]);
		di.team_size = team_size;
		stealing_deque.push(di);
	}
}

template <class Scheduler, template <typename T> class StealingDeque>
template<class CallTaskType, typename ... TaskParams>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::call(TaskParams ... params) {
	if(is_coordinator) {
		assert(stack_filled > 0);

		if(team_size == 1) {
			CallTaskType task(params ...);
			call_team_task(&task);
		}
		else {
			CallTaskType* task = new CallTaskType(params ...);
			call_task(team_size, task, &(stack[stack_filled - 1]));
		}
	}
	else {
		join_coordinator_subteam();
	}
}

template <class Scheduler, template <typename T> class StealingDeque>
template<class CallTaskType, typename ... TaskParams>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::finish_nt(procs_t nt_size, TaskParams ... params) {
	if(is_coordinator()) {
		assert(stack_filled > 0);

		if(nt_size == 1) {
			// Create task
			CallTaskType task(params ...);

			// Create a new stack element for new task
			// num_finished_remote is not required as this stack element blocks lower ones from finishing anyway
			finish_team_task(&task, NULL);
		}
		else {
			CallTaskType* task = new CallTaskType(params ...);

			// Create a new stack element for new task
			// num_finished_remote is not required as this stack element blocks lower ones from finishing anyway
			finish_task(nt_size, task, NULL);
		}
	}
	else {
		join_coordinator_subteam(nt_size);
	}
}

template <class Scheduler, template <typename T> class StealingDeque>
template<class CallTaskType, typename ... TaskParams>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::spawn_nt(procs_t nt_size, TaskParams ... params) {
	// TODO: optimization to use call in some cases to prevent the stack from growing too large

	// TODO: let tasks be spawned by multiple threads

	if(is_coordinator()) {
		CallTaskType* task = new CallTaskType(params ...);
		assert(stack_filled > 0);
		stack[stack_filled - 1].num_spawned++;
		DequeItem di;
		di.task = task;
		di.stack_element = &(stack[stack_filled - 1]);
		di.team_size = nt_size;
		stealing_deque.push(di);
	}
}

template <class Scheduler, template <typename T> class StealingDeque>
template<class CallTaskType, typename ... TaskParams>
void BasicMixedModeSchedulerTaskExecutionContext<Scheduler, StealingDeque>::call_nt(procs_t nt_size, TaskParams ... params) {
	if(is_coordinator()) {
		assert(stack_filled > 0);

		if(team_size == 1) {
			CallTaskType task(params ...);
			call_team_task(&task);
		}
		else {
			CallTaskType* task = new CallTaskType(params ...);
			call_task(team_size, task, &(stack[stack_filled - 1]));
		}
	}
	else {
		join_coordinator_subteam(nt_size);
	}
}

*/


}

#endif /* BASICMIXEDMODESCHEDULERTASKEXECUTIONCONTEXT_H_ */