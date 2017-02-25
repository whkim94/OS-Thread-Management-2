#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define _UTHREAD_PRIVATE
#include "preempt.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
#define HZ 100

void preempt_save(sigset_t *level)
{
	/* TODO: PART 1 - Phase 4 */
}

void preempt_restore(sigset_t *level)
{
	/* TODO: PART 1 - Phase 4 */
}

void preempt_enable(void)
{
	/* TODO: PART 1 - Phase 4 */
}

void preempt_disable(void)
{
	/* TODO: PART 1 - Phase 4 */
}

bool preempt_disabled(void)
{
	/* TODO: PART 1 - Phase 4 */
}

/*
 * timer_handler - Timer signal handler (aka interrupt handler)
 * @signo - Received signal number (can be ignored)
 */
static void timer_handler(int signo)
{
	/* TODO: PART 1 - Phase 4 */
}

void preempt_start(void)
{
	struct sigaction sa;
	struct itimerval it;

	/*
	 * Install signal handler @timer_handler for dealing with alarm signals
	 */
	sa.sa_handler = timer_handler;
	sigemptyset(&sa.sa_mask);
	/* Make functions such as read() or write() to restart instead of
	 * failing when interrupted */
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGVTALRM, &sa, NULL)) {
		perror("sigaction");
		exit(1);
	}

	/*
	 * Configure timer to fire alarm signals at a certain frequency
	 */
	it.it_value.tv_sec = 0;
	it.it_value.tv_usec = 1000000 / HZ;
	it.it_interval.tv_sec = 0;
	it.it_interval.tv_usec = 1000000 / HZ;
	if (setitimer(ITIMER_VIRTUAL, &it, NULL)) {
		perror("setitimer");
		exit(1);
	}
}

