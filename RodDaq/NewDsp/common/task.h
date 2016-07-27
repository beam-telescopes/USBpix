/* Common Header */

#ifndef TASK_H
#define TASK_H

#include <dsp_types.h>
#include <timer.h>

/* task states */
enum {
	TaskStateIdle,
	TaskStateConnect,
	TaskStateInit,
	TaskStateRunning,
	TaskStatePaused,
	TaskStateComplete,
	TaskStateAbort
};

enum {
	TriggerTypeEvent,
	TriggerTypeFreewheel,
	TriggerTypeSynchronous,
	TriggerTypeAsynchronous
};

typedef struct {
	int state, triggerType, tDelta, tNext, optionsSize, id;
	int (*connect)(void *ext, void *task);
	int (*init)(void *ext);
	int (*proc)(void *ext);
	void *(*talk)(void *ext, int topic, UINT32 item);
	int (*exit)(void *task);
	void *ext;
	void *options;
	int fFreeExt, fFreeTask;
        TimeStamp startTime;
} Task;

typedef struct {
	Task **list;
	int nTasks;
	void (*proc)(void);
	Task *(*newTask)(void);
	int (*add)(Task *task);
	int (*remove)(Task *task);
	int (*init)(void);
} TaskQueue;

void connectTaskQueue(TaskQueue **taskQueue);

#endif
