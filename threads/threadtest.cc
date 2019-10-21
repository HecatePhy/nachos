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
    default:
	printf("No test specified.\n");
	break;
    }
}

