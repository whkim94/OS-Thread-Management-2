#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

//#define _UTHREAD_PRIVATE
#include "preempt.h"
#include "queue.h"
#include "semaphore.h"
#include "uthread.h"

struct semaphore {
	/* TODO: PART 1 - Phase 3 */
};

sem_t sem_create(size_t count)
{
	/* TODO: PART 1 - Phase 3 */
}

int sem_destroy(sem_t sem)
{
	/* TODO: PART 1 - Phase 3 */
}

int sem_down(sem_t sem)
{
	/* TODO: PART 1 - Phase 3 */
}

int sem_up(sem_t sem)
{
	/* TODO: PART 1 - Phase 3 */
}

