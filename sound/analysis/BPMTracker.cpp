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

#include <math.h>
#include "BPMTracker.hpp"
#include "StrategyParameter.hpp"
#include "FFT.hpp"
#include <CoreFoundation/CoreFoundation.h>
#include "BPMTrackerEvent.hpp"

const float BPMTracker::FFT_SIZE = 1024.0f;
const int BPMTracker::SHIFT_SIZE = 128;

BPMTracker::BPMTracker(float samplingRate, float floorBPM, float ceilBPM, bool async) :
_samplingRate(samplingRate), _floorBPM(floorBPM), _ceilBPM(ceilBPM), _async(async), _completedBytes(0), _time(0), _jobs(0), _numPreRestSamples(0), _threadIsRunning(false), _needCanceling(false), _detected(0), _choosenAgent(0) {
    _frequencies = new float[8];
    float freqs[8] = {0, 125, 250, 500, 1000, 2000, 6000, 11000};
    memcpy(_frequencies, freqs, sizeof(float) * 8);
    
    _agents = new std::vector<TrackingAgent*>();
    _params = new std::vector<StrategyParameter*>();
    for (int i = 0; i < 3; i++) {
        StrategyParameter* param = new StrategyParameter();
        param->floorBPM = floorBPM;
        param->ceilBPM = ceilBPM;
        param->autoCorrelationSize = ceil(5.0f / (SHIFT_SIZE / samplingRate));
        _params->push_back(param);
    }
    _params->at(0)->watchFrequency = StrategyParameterLowFrequency;
    _params->at(0)->errorTolerance = 4;
    _params->at(1)->watchFrequency = StrategyParameterMidFrequency;
    _params->at(1)->errorTolerance = 2;
    _params->at(2)->watchFrequency = StrategyParameterAllFrequency;
    _params->at(2)->errorTolerance = 0;
    
    _agents->push_back(new TrackingAgent(SHIFT_SIZE, _params->at(0)));
    _agents->push_back(new TrackingAgent(SHIFT_SIZE, _params->at(1)));
    _agents->push_back(new TrackingAgent(SHIFT_SIZE, _params->at(2)));
    _vectorizer = new OnsetVectorizer(7, _agents);
    
    _peakDetectors = new std::vector<PeakDetector*>();
    float HSR = _samplingRate * 0.5;
    float numMagnitudes = static_cast<int>(floor(FFT_SIZE * 0.5));
    for (int i = 0; i < 7; i++) {
        int s = static_cast<int>(ceil(_frequencies[i] / HSR * numMagnitudes));
        int e = static_cast<int>(ceil(_frequencies[i + 1] / HSR * numMagnitudes));
        _peakDetectors->push_back(new PeakDetector(i, _vectorizer, s, e));
    }
    
    
    _jobs = new std::vector<TrackingJob*>();
    
    pthread_mutex_init(&_mutex, NULL);
    
    _restSamples = (float*)malloc(1);
}


BPMTracker::~BPMTracker() {
    printf("delete BPMTracker\n");
    delete[] _frequencies;
    
    while (_peakDetectors->size()) {
        delete _peakDetectors->front();
        _peakDetectors->erase(_peakDetectors->begin());
    }
    while (_agents->size()) {
        delete _agents->front();
        _agents->erase(_agents->begin());
    }
    while (_params->size()) {
        delete _params->front();
        _params->erase(_params->begin());
    }
    while (_jobs->size()) {
        free(_jobs->front()->samples);
        delete _jobs->front();
        _jobs->erase(_jobs->begin());
    }
    
    delete _peakDetectors;
    delete _agents;
    delete _params;
    delete _vectorizer;
    if (_restSamples) free(_restSamples);
    
    pthread_mutex_destroy(&_mutex);
}

void BPMTracker::deleteAfterCanceling() {
    _needCanceling = true;
    if (!_threadIsRunning) delete this;
}

void BPMTracker::addSamples(float *samples, unsigned long size) {
    //  size = 1025 * sizeof(float);
    int fftSize = static_cast<int>(BPMTracker::FFT_SIZE);
    
    // 今回フレーム数
    long newFrames = size / sizeof(float) + _numPreRestSamples;
    
    // 余り
    int rest  = newFrames % fftSize;
    
    int preRestSize = 0;
    
    int jobFrames = newFrames - rest;
    size_t jobSize = 0;
    TrackingJob* job;
    float* recentSamples = samples;
    int restIndex = 0;
    
    
    if (fftSize <= jobFrames)  {
        jobSize = newFrames * sizeof(float);
        job = new TrackingJob();
        job->numFrames = jobFrames;
        job->samples = (float*)malloc(jobSize);
        
        recentSamples = job->samples;
        restIndex = jobFrames - fftSize + SHIFT_SIZE;
        rest += fftSize - SHIFT_SIZE;
        
        if (_numPreRestSamples) {
            // 前回のあまりと今回の入りきる分をコピー
            if (_numPreRestSamples < jobFrames) {
                preRestSize = _numPreRestSamples * sizeof(float);
                memcpy(job->samples, _restSamples, preRestSize);
                memcpy(&job->samples[_numPreRestSamples], samples, jobSize - preRestSize);
            }
            else {
                preRestSize = _numPreRestSamples * sizeof(float);
                memcpy(job->samples, _restSamples, preRestSize);
                memcpy(&job->samples[_numPreRestSamples], samples, size);
            }
        } else {
            memcpy(job->samples, samples, jobSize);
        }
        
        pthread_mutex_lock(&_mutex);
        _jobs->push_back(job);
        pthread_mutex_unlock(&_mutex);
    }
    size_t restSize = rest * sizeof(float);
    free(_restSamples);
    _restSamples = (float*)malloc(rest * sizeof(float));
    memcpy(_restSamples, &recentSamples[restIndex], restSize);
    _numPreRestSamples = rest;
    
    if (_async) {
        if (!_threadIsRunning) {
            _threadIsRunning = true;
            
            pthread_create(&_thread, NULL, analyze, this);
            //        struct sched_param   param;
            //      param.sched_priority = sched_get_priority_min(SCHED_OTHER);
            //      pthread_setschedparam(_thread, SCHED_OTHER, &param);
            pthread_detach(_thread);
        }
    }
    else analyze(this);
}

void* BPMTracker::analyze(void *pthis) {
#ifdef DEBUG
    double t = CFAbsoluteTimeGetCurrent();
#endif
    BPMTracker* self = static_cast<BPMTracker*>(pthis);
    
    unsigned long& completedBytes = self->_completedBytes;
    size_t sampleBytes = sizeof(float);
    
    std::vector<PeakDetector*>* pds = self->_peakDetectors;
    
    int fftSize = static_cast<int>(BPMTracker::FFT_SIZE);
    int shiSize = static_cast<int>(BPMTracker::SHIFT_SIZE);
    float* spectrum = new float[fftSize]();
    int numPeakDetectors = self->_peakDetectors->size();
    FFT* fft = new FFT(fftSize);
    int walk = 0;
    
    BPMTrackerEvent progEvent(BPMTrackerEventProgress);
    
    std::vector<TrackingJob*>* jobs = self->_jobs;
    while (jobs->size()) {
        if (self->_needCanceling) {
            delete fft;
            delete[] spectrum;
            self->_threadIsRunning = false;
            delete self;
            return NULL;
        }
        
        TrackingJob* job = jobs->front();
        
        float* samples = job->samples;
        long bufs = job->numFrames;
        walk = 0;
        while (walk + fftSize <= bufs) {
            fft->transform(&samples[walk], spectrum);
            walk += shiSize;
            for (int i = 0; i < numPeakDetectors; i++) {
                pds->at(i)->pushMagnitudes(spectrum);
            }
        }
        
        free(samples);
        delete job;
        jobs->erase(jobs->begin());
        
        
        completedBytes += bufs * sampleBytes;
        self->invoke(progEvent);
    }
    
    float maxR = 0;
    float maxIdx = 0;
    for (int i = 0, l = self->_agents->size(); i < l; i++) {
        if (maxR < self->_agents->at(i)->getReliability()) {
            maxR = self->_agents->at(i)->getReliability();
            maxIdx = i;
        }
    }
    self->_choosenAgent = self->_agents->at(maxIdx);
#ifdef DEBUG
    printf("COMPLETE %3.2f 秒  BPM : %f  bytes : %lu\n", CFAbsoluteTimeGetCurrent() - t, self->_choosenAgent->getBPM(), completedBytes);
#endif
    self->_detected = true;
    
    delete fft;
    delete[] spectrum;
    self->_threadIsRunning = false;
    return 0;
}

const bool BPMTracker::isProcessingInBackground() const {
    return _threadIsRunning;
}

const float BPMTracker::getBPM() const {
    if (_detected)
        return _choosenAgent->getBPM();
    return 0;
}

const long BPMTracker::getPhase() const {
    if (_choosenAgent)
        return _choosenAgent->getPhase();
    return 0;
}

const unsigned long BPMTracker::getCompletedBytes() const {
    return _completedBytes;
}