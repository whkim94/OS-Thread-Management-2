all: libuthread.a

libuthread.a: queue.o context.o uthread.o semaphore.o preempt.o bitmap.o
	ar rcs libuthread.a $^

queue.o: queue.c queue.h
	gcc -Werror -c -o $@ queue.c

context.o: context.c context.h preempt.h uthread.h
	gcc -Werror -c -o $@ context.c

uthread.o: uthread.c uthread.h context.h preempt.h queue.h
	gcc -Werror -c -o $@ uthread.c

semaphore.o: semaphore.c semaphore.h preempt.h queue.h uthread.h
	gcc -Werror -c -o $@ semaphore.c

preempt.o: preempt.c preempt.h uthread.h
	gcc -Werror -c -o $@ preempt.c

bitmap.o: bitmap.c bitmap.h
	gcc -Werror -c -o $@ bitmap.c

clean :
	rm -f queue.o context.o uthread.o semaphore.o preempt.o
