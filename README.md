# Cooperative Multitasking Scheduler in C

This project implements a basic cooperative multitasking scheduler in C. It simulates lightweight user-level threads using setjmp and longjmp for context switching. Threads voluntarily yield control, and scheduling is driven by a periodic alarm signal.

The goal of this project is educational. It demonstrates how scheduling, context switching, and stack management can be implemented in user space without relying on OS-level threads.


## Overview

- Implements lightweight threads in user space
- Uses setjmp and longjmp for context switching
- Uses SIGALRM to trigger scheduling decisions
- Employs a round-robin scheduling strategy
- Handles thread lifecycle and cleanup

## Project Structure

### Files

- scheduler.c  
  Contains the core scheduling logic, including thread creation, context switching, scheduling, and cleanup.

- scheduler.h  
  Declares scheduler-related types, function pointers, and public APIs.

- system.h  
  Declares system-level helpers and definitions used by the scheduler.


## Core Data Structures

### Thread Representation

Each lightweight thread is represented by a thread structure containing:

- Execution context for saving and restoring state
- Pointer to an allocated stack
- Function pointer representing the thread’s entry point
- Argument passed to the thread function
- Status indicating lifecycle state
- Link to the next thread in the scheduler list

Thread states include:
- INIT
- RUNNING
- SLEEPING
- TERMINATED

Global pointers maintain:
- The head of the thread list
- The currently running thread


## Key Functions

### scheduler_create(fnc, arg)
- Allocates and initializes a new thread
- Sets up its stack and execution function
- Inserts the thread into the scheduler’s linked list

### handler(sig)
- Signal handler for SIGALRM
- Invokes scheduler_yield to trigger a context switch

### candidate()
- Selects the next runnable thread
- Implements round-robin scheduling

### schedule()
- Chooses a runnable thread
- Marks it as RUNNING
- Transfers control using longjmp
- Starts execution if the thread is in INIT state

### scheduler_yield()
- Saves the current thread context using setjmp
- Transfers control back to the scheduler

### scheduler_execute()
- Initializes scheduling infrastructure
- Registers the alarm signal handler
- Starts the scheduling loop
- Ensures terminated threads are cleaned up

### destroy()
- Frees allocated stacks and thread structures
- Cleans up scheduler resources


## Usage Example

```c
void my_thread_function(void *arg) {
    printf("Thread running!\n");
}

int main() {
    scheduler_create(my_thread_function, NULL);
    scheduler_execute();
    return 0;
}
