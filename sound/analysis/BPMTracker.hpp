/* =====================================================
Copyright (c) 2012 Satoshi Takano

The MIT License

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===================================================== */

#pragma once
#include <vector.h>
#include "EventSender.hpp"
#include "PeakDetector.hpp"
#include "TrackingAgent.hpp"
#include "OnsetVectorizer.hpp"
#include "Asynchronous.hpp"

class BPMTracker : public events::EventSender, public Asynchronous {
public:
    BPMTracker(float samplingRate, float floorBPM, float ceilBPM, bool async);
    ~BPMTracker();
    void addSamples(float* samples, unsigned long size);
    
    const bool isProcessingInBackground() const;
    const unsigned long getCompletedBytes() const;
    const float getBPM() const;
    const long getPhase() const;
    
    void deleteAfterCanceling();
    
private:
    struct TrackingJob {
        unsigned numFrames;
        float* samples;
    };
    
    static void* analyze(void* pthis);
    
    static const float FFT_SIZE;
    static const int SHIFT_SIZE;
    
    bool _async;
    float* _frequencies;
    float _floorBPM;
    float _ceilBPM;
    float _time;
    float _samplingRate;
    unsigned long _completedBytes;
    
    OnsetVectorizer* _vectorizer;
    std::vector<PeakDetector*>* _peakDetectors;
    std::vector<TrackingAgent*>* _agents;
    std::vector<StrategyParameter*>* _params;
    
    std::vector<TrackingJob*>* _jobs;
    float* _restSamples;
    unsigned _numPreRestSamples;
    
    pthread_t _thread;
    pthread_mutex_t _mutex;
    bool _threadIsRunning;
    bool _needCanceling;
    
    bool _detected;
    TrackingAgent* _choosenAgent;
};
