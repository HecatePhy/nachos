// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!

/* @date   13 Oct 2019
 * @target lab3-exercise3
 * @brief  complete Lock with Semaphore
 * */

Lock::Lock(char* debugName) 
{
    name = debugName;
    lock = new Semaphore("lock", 1);
    owner = NULL; 
}

Lock::~Lock() 
{
    delete lock;
}

void Lock::Acquire() 
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    lock->P();
    owner = currentThread;
    (void) interrupt->SetLevel(oldLevel);
}

void Lock::Release()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    ASSERT(currentThread == owner);
    lock->V();
    owner = NULL;
    (void) interrupt->SetLevel(oldLevel);
}

/* @date   13 Oct 2019
 * @target lab3-exercise3
 * @brief  complete Condition with List and para Lock
 * */

Condition::Condition(char* debugName) 
{
    name = debugName;
    wqueue = new List;
}

Condition::~Condition() 
{
    delete wqueue;    
}

void Condition::Wait(Lock* conditionLock) 
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    ASSERT(conditionLock->getOwner() == currentThread);
    conditionLock->Release();
    wqueue->Append(currentThread);
    currentThread->Sleep();
    conditionLock->Acquire();
    (void) interrupt->SetLevel(oldLevel);
}

void Condition::Signal(Lock* conditionLock) 
{ 
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    ASSERT(conditionLock->getOwner() == currentThread);
    if(!wqueue->IsEmpty()) {
        Thread *fthread = (Thread*) wqueue->Remove();
	scheduler->ReadyToRun(fthread);
    }
    (void) interrupt->SetLevel(oldLevel);
}

void Condition::Broadcast(Lock* conditionLock) 
{ 
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    ASSERT(conditionLock->getOwner() == currentThread);
    while(!wqueue->IsEmpty()) {
        Signal(conditionLock);
    }
    (void) interrupt->SetLevel(oldLevel);
}

/* @date   13 Oct 2019
 * @target lab3-challenge1
 * @brief  implement Barrier with Condition
 * */

Barrier::Barrier(char* debugName, int thr)
{
    name = debugName;
    condition = new Condition("barriercondition");
    thread_cnt = 0;
    threshold = thr;
}

Barrier::~Barrier()
{
    delete condition;
}

void Barrier::setBarrier(Lock *barrierLock)
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    ASSERT(barrierLock->getOwner() == currentThread);
    thread_cnt++;
    if(thread_cnt >= threshold) {
    	condition->Broadcast(barrierLock);
    }
    else {
        condition->Wait(barrierLock);
    }
    (void) interrupt->SetLevel(oldLevel);
}

/* @date   13 Oct 2019
 * @target lab3-challenge2
 * @brief  implement read/write block with Semaphore
 * */

RWLock::RWLock(char* debugName)
{
    name = debugName;
    wlock = new Lock("wlock");
    rlock = new Lock("rlock");
    rcnt = 0;
}

RWLock::~RWLock()
{
    delete wlock;
    delete rlock;
}

void RWLock::racquire()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    if(wlock->getOwner() == NULL) {
	rlock->Acquire();
	rcnt++;
	rlock->Release();
    }
    else {
	printf("thread %s %d fail to read\n", currentThread->getName(), currentThread->getTid());
        currentThread->Yield();
    }
    (void) interrupt->SetLevel(oldLevel);
}

void RWLock::rrelease()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    rlock->Acquire();
    rcnt--;
    rlock->Release();
    (void) interrupt->SetLevel(oldLevel);
}

void RWLock::wacquire()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    if(rcnt == 0 && wlock->getOwner() == NULL) {
	printf("thread %s %d acquire wlock\n", currentThread->getName(), currentThread->getTid());
	wlock->Acquire();
    }
    else {
	printf("thread %s %d fail to write: ", currentThread->getName(), currentThread->getTid());
        if(wlock->getOwner() != NULL) printf("src is owned by %s %d\n", wlock->getOwner()->getName(), wlock->getOwner()->getTid());
	else printf("src is owned by reader\n");
	currentThread->Yield();
    }
    (void) interrupt->SetLevel(oldLevel);
}

void RWLock::wrelease()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    ASSERT(wlock->getOwner() == currentThread);
    wlock->Release();
    (void) interrupt->SetLevel(oldLevel);
}
