/**
 * Tony Givargis
 * Copyright (C), 2023
 * University of California, Irvine
 *
 * CS 238P - Operating Systems
 * scheduler.c
 */

#undef _FORTIFY_SOURCE

#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include "system.h"
#include "scheduler.h"
#include <signal.h>

/**
 * Needs:
 *   setjmp()
 *   longjmp()
 */

/* research the above Needed API and design accordingly */

#define STACK_SIZE 1048576

typedef struct thread {
    jmp_buf ctx;
    struct {
        char *memory;
        char *memory_;
    } stack;
    struct {
        void *arg;
        scheduler_fnc_t fnc;
    } code;
    enum { INIT, RUNNING, SLEEPING, TERMINATED } status;
    struct thread *link;
} thread;

thread *head = NULL;
thread *current_thread = NULL;
jmp_buf ctx;

int scheduler_create(scheduler_fnc_t fnc, void *arg) {
    thread *t = (thread *)malloc(sizeof(thread));
    if (!t) {
        return -1;
    }
    t->code.fnc = fnc;
    t->code.arg = arg;
    t->status = INIT;
    t->stack.memory_ = (char *)malloc(STACK_SIZE + page_size());
    if (!t->stack.memory_) {
        free(t);
        return -1;
    }
    t->stack.memory = memory_align(t->stack.memory_, STACK_SIZE + page_size());
    t->link = head;
    head = t;
    return 0;
}

void handler() {
    signal(SIGALRM, handler);
    alarm(1);
    scheduler_yield();
}

 void destroy(void) {
    thread *t, *t_;
    t = head;
    while (t) {
        t_ = t;
        t = t->link;
        free(t_->stack.memory_);
        free(t_);
    }
}

 static thread *candidate(void) {
    thread *t;
    if (current_thread->link != NULL)
        t = current_thread->link;
    else
        t = head;

    while (t != NULL) {
        if (t->status == INIT || t->status == SLEEPING) {
            return t;
        }
        t = t->link;
    }
    t = head;
    while (t != NULL) {
        if (t->status != TERMINATED) {
            return t;
        }
        t = t->link;
    }
    return NULL;
}

 static void schedule(void) {
    unsigned long stack_pointer;
    thread *t;
    t = candidate();
    if (t == NULL) {
        return;
    }

    current_thread = t;
    if (t->status == INIT) {
        stack_pointer = (unsigned long)t->stack.memory;
        __asm__ volatile (
            "movq %0, %%rsp\n"
            :
            : "r"(stack_pointer)
            : 
        );
        t->status = RUNNING;
        t->code.fnc(t->code.arg);
        t->status = TERMINATED;
        longjmp(ctx, 1);
    } else {
        t->status = RUNNING;
        longjmp(t->ctx, 1);
    }
}

void scheduler_execute(void) {
    current_thread = head;
    signal(SIGALRM, handler);
    alarm(1);
    setjmp(ctx);
    schedule();
    destroy();
}

void scheduler_yield(void) {
    if (!setjmp(current_thread->ctx)) {
        current_thread->status = SLEEPING;
        longjmp(ctx, 1);
    }
}
