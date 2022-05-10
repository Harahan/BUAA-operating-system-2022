#include "sched.h"
#include<stdlib.h>
#include<string.h>
#include<stdio.h>

void FCFS (
    int number_of_jobs, const int job_submitted_time [],
    const int job_required_time [], int job_sched_start []) {
	int i = 0, t = 0;
	for (; i < number_of_jobs; i++) {
		if (job_submitted_time[i] > t) t = job_submitted_time[i];
		job_sched_start[i] = t;
		t += job_required_time[i];
	}
}

typedef struct x {
	int id, rt, sbt;
} work;


int cmp(const void* w1, const void* w2) {
	work* _w1 = (work*)w1; work* _w2 = (work*)w2;
	if (_w1->rt == _w2->rt) {
		if (_w1->sbt == _w2->sbt) return _w1->id - _w2->id;
		return _w1->sbt - _w2->sbt;
	}
	return _w1->rt - _w2->rt;
}

void SJF (
    int number_of_jobs, const int job_submitted_time [],
    const int job_required_time [], int job_sched_start []) {
	int i = 0;
	work wks[2500];
	for (; i < number_of_jobs; i++) {
		wks[i].id = i;
		wks[i].rt = job_required_time[i];
		wks[i].sbt = job_submitted_time[i];
	}
	work buf[2500];
	i = 0;
	int t = job_submitted_time[0], iq = 0, tot = 0;
	work queue[2500];
	while (1) {
		if ((queue[1].rt == 2147483647 || queue[1].rt == 0) && t < wks[i].sbt) t = wks[i].sbt;
		for (; wks[i].sbt <= t && i < number_of_jobs; i++) {
			memcpy(queue + iq, wks + i, sizeof(work));
			iq++;
		}
		qsort(queue, iq, sizeof(work), cmp);
		int j = 0;
		//for (; j <iq; j++) printf("[%d %d %d] ", queue[j].id, queue[j].rt, queue[j].sbt);
		//printf("\n");
		job_sched_start[queue[0].id] = t;
		t += queue[0].rt;
		queue[0].rt = 2147483647;
		if ((++tot) == number_of_jobs) break;
	}
}









