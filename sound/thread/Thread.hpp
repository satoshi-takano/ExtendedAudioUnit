//
//  Thread.h
//  IKKitDevelop
//
//  Created by  on 4/8/12.
//  Copyright (c) 2012 アップルジャパン株式会社. All rights reserved.
//

#pragma once

#include "Runnable.hpp"
#include <pthread.h>
#include "Asynchronous.hpp"

class Thread : public Runnable, public Asynchronous {
    Runnable* mRunnable;
    static void* _run(void* pthis);
    
public:
    Thread() : mRunnable(NULL) {}
    Thread(Runnable* runnable);
    virtual ~Thread();
    
    const int start();
    const int join();
    const int detach();
    
    virtual void deleteAfterCanceling() = 0;
    
    pthread_t& getThreadRef() {return mThreadID;}
    
protected:
    pthread_t mThreadID;
};