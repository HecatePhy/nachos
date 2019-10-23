// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "elevatortest.h"

#include "synch.h" // HecAtePhy: add sync header

// testnum is set in main.cc
int testnum = 1;

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread", 0);

    t->Fork(SimpleThread, (void*)1);
    SimpleThread(0);
}

//----------------------------------------------------------------------
// ThreadTest2
//      test thread control by invoke ThreadTest1
//      test TS command by output ttable
// @date   3 Oct 2019
// @target lab1-exercise4
// @brief  new TS threadtest with testnum 3
//----------------------------------------------------------------------

void
TS()
{
    printf("---TS---\n");
    for(int i=1; i<=MAX_TID; i++){
        if(tid_mask[i]){
	    printf("thread tid: %d user: %d name: %s\n", ttable[i]->getTid(), ttable[i]->getUid(), ttable[i]->getName());
	}
    }
}

void
ThreadTest2()
{
    DEBUG('t', "Entering ThreadTest2");

    for(int i=0; i<=100; i++){
        Thread *t = new Thread("forked thread", 0);
    }

    TS();
}

//----------------------------------------------------------------------
// ThreadTest4
//      test priority & preemptive
// @date   3 Oct 2019
// @target lab2-exercise3
// @brief  scheduler test by SimpleThread
//----------------------------------------------------------------------

void
ThreadTest4()
{
    DEBUG('t', "Entering ThreadTest4");

    // 
}

//----------------------------------------------------------------------
// ThreadTest5
//      test round-robin with TIME_SLICE 100
//      timer ticks advance by manual setlevel
// @date   3 Oct 2019
// @target lab2-challenge1
// @brief  new thread which loop and setlevel
//----------------------------------------------------------------------

void
TickThread(int n)
{
    for (int i = 1; i <= n; i++) {
        printf("*** thread %d with priority %d looped %d times\n", currentThread->getTid(), currentThread->getPriority(), i);
	interrupt->SetLevel(IntOn);
	interrupt->SetLevel(IntOff);
    }
}

void
ThreadTest5()
{
    DEBUG('t', "Entering ThreadTest5");

    Thread *t1 = new Thread("tickthread1", 2);
    Thread *t2 = new Thread("tickthread2", 4);
    Thread *t3 = new Thread("tickthread3", 8);

    t1->Fork(TickThread, 120);
    t2->Fork(TickThread, 120);
    t3->Fork(TickThread, 120);
}

//----------------------------------------------------------------------
// ThreadTest6
//      Lock solution to producer-consumer
// @date   13 Oct 2019
// @target lab3-exercise3&4
// @brief  realize producer-consumer by Ac&Rl when produce/consume
//----------------------------------------------------------------------

void 
ThreadTest6()
{}

//----------------------------------------------------------------------
// ThreadTest7
//      Semaphore solution to producer-consumer
// @date   13 Oct 2019
// @target lab3-exercise3&4
// @brief  use Semaphore to represent slot
//----------------------------------------------------------------------

int slot = 24;
int slotcnt = 0;
Semaphore *sema = new Semaphore("empty", 0);
Semaphore *nsema = new Semaphore("full", slot);

void 
sproducer(int val) 
{
    for(int i = 0; i < val; i++) {
        if(slotcnt >= slot) {
			printf("slot is full: thread %s %d cannot produce now\n", currentThread->getName(), currentThread->getTid());
        }
	    nsema->P();
	    sema->V();
        slotcnt++;
	    printf("thread %s %d produce production\n", currentThread->getName(), currentThread->getTid());
    }
}

void 
sconsumer(int val)
{
    for(int i = 0; i < val; i++) {
		if(slotcnt <= 0) {
			printf("slot is empty: thread %s %d cannot consume now\n", currentThread->getName(), currentThread->getTid());
		}
	    sema->P();
		slotcnt--;
	    nsema->V();
        printf("thread %s %d consume production\n", currentThread->getName(), currentThread->getTid());
    }
}

void 
ThreadTest7()
{
    DEBUG('t', "Entering ThreadTest7");
    Thread *t1 = new Thread("producer1", 4);
    Thread *t2 = new Thread("consumer1", 4);
    Thread *t3 = new Thread("producer2", 4);
    Thread *t4 = new Thread("consumer2", 4);
    Thread *t5 = new Thread("consumer3", 4);

    t1->Fork(sproducer, 32);
    t2->Fork(sconsumer, 32);
    t3->Fork(sproducer, 32);
    t4->Fork(sconsumer, 32);
    t5->Fork(sconsumer, 32);
}

//----------------------------------------------------------------------
// ThreadTest8
//      Condition solution to producer-consumer
// @date   13 Oct 2019
// @target lab3-exercise3&4
// @brief  
//----------------------------------------------------------------------

Condition *ccond = new Condition("consume");
Condition *pcond = new Condition("producer");
Lock *pclock = new Lock("pclock");
int srccnt = 0;

void 
cproducer(int val)
{
    for(int i = 0; i < val; i++) {
	pclock->Acquire();
	while(srccnt >= slot) {
	    printf("slot is full: producer %s %d cannot produce\n", currentThread->getName(), currentThread->getTid());
	    pcond->Wait(pclock);
	}
	printf("thread %s %d produce production\n", currentThread->getName(), currentThread->getTid());
	srccnt++;
	ccond->Signal(pclock);
	pclock->Release();
    }
}

void 
cconsumer(int val)
{
    for(int i = 0; i < val; i++) {
        pclock->Acquire();
	while(srccnt <= 0) {
	    printf("slot is empty: consumer %s %d cannot consume\n", currentThread->getName(), currentThread->getTid());
	    ccond->Wait(pclock);
	}
	printf("thread %s %d consume production\n", currentThread->getName(), currentThread->getTid());
	srccnt--;
	pcond->Signal(pclock);
	pclock->Release();
    }
}

void 
ThreadTest8()
{
    DEBUG('t', "Entering ThreadTest8");

    Thread *t1 = new Thread("producer1", 4);
    Thread *t2 = new Thread("consumer1", 4);
    Thread *t3 = new Thread("producer2", 4);
    Thread *t4 = new Thread("consumer2", 4);

    t1->Fork(cproducer, 32);
    t2->Fork(cconsumer, 32);
    t3->Fork(cproducer, 32);
    t4->Fork(cconsumer, 32);
}

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    switch (testnum) {
    case 1:
	ThreadTest1();
	break;
    case 3:
	ThreadTest2();
	break;
    case 4:
	ThreadTest4();
	break;
    case 5:
	ThreadTest5();
	break;
    case 6:
	ThreadTest6();
	break;
    case 7:
	ThreadTest7();
	break;
    case 8:
	ThreadTest8();
	break;
    default:
	printf("No test specified.\n");
	break;
    }
}

