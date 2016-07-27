#include <dsp_debug_conf.h>
#if debug_task_c == 1
#define DEBUG_THIS_FILE
#endif

/* Variables declared: */
#pragma DATA_SECTION(pTask,"xdata");
#pragma DATA_SECTION(taskQueue,"xdata");
/* Functions declared: */
#pragma CODE_SECTION(add,"xprog");
#pragma CODE_SECTION(connectTaskQueue,"xprog");
#pragma CODE_SECTION(init,"xprog");
#pragma CODE_SECTION(newTask,"xprog");
#pragma CODE_SECTION(proc,"xprog");
#pragma CODE_SECTION(remove,"xprog");

/*! \file  Common - task.c */

//! Common to both DSPs 
#include <dspGlobal.h>
#include <task.h>
#include <timer.h>
#include <globals.h>
#include <dsp_stdio.h> 
#include <dsp_string.h>
#include <dsp_stdlib.h>

#define MAXTASKS 16
static Task *pTask[MAXTASKS+1];
static TaskQueue taskQueue;
                         
/*! 
\brief Add a task \n

*/
static int add(Task *task) {
	int k = 0;
	while(taskQueue.list[k]) ++k;
	if(k == MAXTASKS) return 1; // failed to add task 
	taskQueue.list[k] = task;
	dsp_printf("\nnumber of tasks becomes %d\n",(k+1));
	return 0;
}

/*! 
\brief Use malloc to make a new Task struct \n

*/
static Task *newTask(void) {
	Task *task;
	task = (Task *)dsp_malloc(sizeof(Task));
	task->fFreeTask = 1; // free memory after 
	dsp_memset(task, 0, sizeof(Task));
	task->triggerType = TriggerTypeFreewheel; // freewheel 
	task->fFreeTask = 1; // free memory after
        task->startTime=getClock(); 
	return task;
}

/*! 
\brief Remove a task \n

*/
static int remove(Task *task) {
// find the task in the queue 
        TimeStamp deltaT;
	int status, taskIndex=0, lastTask, k = 0;
	ENTRY(ID_task_remove);
	status = 1; // assume failure 
	while(taskQueue.list[k]) {
		if(taskQueue.list[k] == task) {
			taskIndex = k;
			break;
		}
	}
	deltaT=getClock()-task->startTime;
// if we didn't find it, there is something wrong 
	if(taskIndex == taskQueue.nTasks) {RETURN status;}
	
	// printf some information about removed task
	dsp_printf("task id=%d has been removed. time elapsed=",task->id);printTimeStamp(deltaT);
	dsp_printf("\n");
// find the last task in the queue 
	lastTask = taskQueue.nTasks - 1;
	while(lastTask && (taskQueue.list[lastTask] == 0))
		--lastTask;

// replace the deleted task by the last task 
	taskQueue.list[taskIndex] = taskQueue.list[lastTask];
	taskQueue.list[lastTask] = 0;	
	status = 0;

// free memory as appropriate 	
	if(task->fFreeExt) dsp_free(task->ext);
	if(task->fFreeTask) dsp_free(task);
	
	RETURN status;
}

/*! 
\brief Process task list \n

*/
static void proc(void) {
	int k = 0, t, i=0;
      	Task *task;
	
/*! go through the task list and see if there are any synchronous requests.
	these will run with priority 	*/

// Need to add an option here for 
	while(taskQueue.list[k]) {
		t = time();
		getClock(); /* ensure 64 bit timestamp doesn't overrun */
		task = taskQueue.list[k];
		if(task->state == TaskStatePaused) {
			continue;
		}
		if((task->state == TaskStateRunning) && (dataAcqEventCntr > 0)) {
			if (task->triggerType == TriggerTypeEvent) { //Priority to deal with Event-triggers
				if(task->proc)	{
					task->state = task->proc(task->ext);
				// task may return 0 or into another state via return value 
					if(task->state == 0) task->state = TaskStateRunning;
					else task->state = TaskStateComplete;
					--dataAcqEventCntr; // jsv
				} else {
					task->state = TaskStateComplete; // manually move along 
				}
			} else { 
				if((task->triggerType == TriggerTypeSynchronous) && // if synchronous trigger 
					(t >= task->tNext)) { // and the time has come... 
					task->tNext += task->tDelta; // set next trigger time 
					if(task->proc)	{
						task->state = task->proc(task->ext);
						if(task->state == 0) task->state = TaskStateComplete;						
					}
					else task->state = TaskStateComplete; // manually move along 
				}
			}
		}
		++k;
	}

// submit the task list as normal 
	k = 0;
	while(taskQueue.list[k]) {
 		t = time();
	        getClock();
		task = taskQueue.list[k];
		if(task->triggerType !=  TriggerTypeSynchronous && task->triggerType != TriggerTypeEvent){
			switch(task->state) {
			case TaskStatePaused:
				continue;
			case TaskStateConnect:
				if(task->connect) {
					task->state = task->connect(task->ext, task);
					if(task->state == 0) task->state = TaskStateInit;
				} else {
					task->state = TaskStateInit;
				}
				break;
			case TaskStateInit:
				if(task->init)	{
					task->state = task->init(task->ext);
					if(task->state == 0) task->state = TaskStateRunning;
				} else {
					task->state = TaskStateRunning; // manually move along 
				}
				break;
			case TaskStateRunning:
				if((task->triggerType != TriggerTypeFreewheel) && (t < task->tNext)) 
					break; // it's not time yet 
				if(task->triggerType == TriggerTypeSynchronous) {
					task->tNext += task->tDelta; // set next trigger time synchronously 
				} else if(task->triggerType == TriggerTypeAsynchronous) {
					task->tNext = t + task->tDelta; // set next trigger time asynchronously 
				}
				if(task->proc)	{
					task->state = task->proc(task->ext);
				// task may return 0 or into another state via return value 
					if(task->state == 0) task->state = TaskStateRunning;
				} else {
					task->state = TaskStateComplete; // manually move along 
				}
				break;
			case TaskStateComplete:
				dsp_printf("task->exit is now set to %x\n", task->exit);
				if(task->exit) task->state = task->exit(task);
				taskQueue.remove(task);
				i=0;
				while(taskQueue.list[i]) {
					dsp_printf("Id of task %d in taskQueue = %d\n", i, (taskQueue.list[i])->id);
					i++;
				}
				dsp_printf("Finished TaskStateComplete commands\n");
				break;
			default:
				dsp_printf("Task %d has an invalid state - Switching to TaskStateAbort\n", k);
				task->state = TaskStateAbort;
				break;
			}
		} //End if(trigger types)	
		++k;
	}
}

/*! 
\brief Initialize task list \n

*/
static int init(void) {
	int k = 0;
	for(k=0;k<MAXTASKS;++k)
		pTask[k] = 0;
	pTask[MAXTASKS] = 0; // always this one marks the end 
	return 0;
}

/*! 
\brief Connect a task to a queue \n

*/
void connectTaskQueue(TaskQueue **xTaskQueue) {
	*xTaskQueue = &taskQueue;
	taskQueue.nTasks = MAXTASKS;
	taskQueue.list = pTask;
	taskQueue.add = add;
	taskQueue.remove = remove;
	taskQueue.proc = proc;
	taskQueue.newTask = newTask;
	taskQueue.init = init;
}
                                                        
