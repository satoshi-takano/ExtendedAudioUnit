//
//  Thread.cpp
//  IKKitDevelop
//
//  Created by  on 4/8/12.
//  Copyright (c) 2012 アップルジャパン株式会社. All rights reserved.
//

#include <iostream>
#include "Thread.hpp"

Thread::Thread(Runnable* runnable) 
{
  this->mRunnable = runnable;
}

Thread::~Thread()
{
  pthread_detach(mThreadID);
  pthread_cancel(mThreadID);
}

const int Thread::start()
{
  Runnable* exec = this;
  if (this->mRunnable != NULL) {
    exec = this->mRunnable;
  }
  int res = pthread_create(&mThreadID, NULL, _run, exec);
  struct sched_param   param;
  param.sched_priority = sched_get_priority_min(SCHED_FIFO);
  pthread_setschedparam(getThreadRef(), SCHED_FIFO, &param);
  return res;
}

const int Thread::join()
{
  return pthread_join(mThreadID, NULL);
}

const int Thread::detach()
{
    return pthread_detach(mThreadID);
}

void* Thread::_run(void *pthis)
{
  Runnable* r = static_cast<Runnable*>(pthis);
  r->run();
  return NULL;
}
