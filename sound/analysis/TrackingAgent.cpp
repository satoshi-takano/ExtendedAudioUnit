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
#include <hash_map.h>
#include "TrackingAgent.hpp"
#include "OnsetVectorizer.hpp"

#define START_TRACKING_THRESHOLD 6.0

TrackingAgent::TrackingAgent(int timeIntervalFrames, StrategyParameter* param) :
_timeIntervalFrames(timeIntervalFrames), _parameter(param), _watchAt(0), _watchLength(0), _phase(0),
_doTrack(false), _reduce(floor(1.0f / (timeIntervalFrames / 44100.0f))), _walk(0), _acRangeExpander(1.0f), _acNoCorrelationCount(0) {
  
  _intervalHistory = new std::vector<float>();
  _excludedIntervals = new std::vector<float>();
  
  _beatTracker = 0;
  _backBeatTracker = 0;
}

TrackingAgent::~TrackingAgent() {
  delete _intervalHistory;
  delete _excludedIntervals;
  if (_beatTracker ) {
    delete _beatTracker;
    delete _backBeatTracker;
  }
}

void TrackingAgent::updateOnsetVector() {
  const std::vector<float*>* onsetVectors = _vectorizer->getOnsetVectors();
  
  unsigned now = onsetVectors->size() - 1;
  long errorTolerance = _parameter->errorTolerance;
  float power = 0;
  
  if (_doTrack) {
    power = getPower(onsetVectors->at(now));
    _beatTracker->findOnset(now, power);
    _backBeatTracker->findOnset(now, power);
//    printf("%s %d %f %ld\n", _parameter->getWatchFrequencyString(), _beatTracker->getTrackingBeatInterval(), _beatTracker->getReliability(), _phase);
  } else if (_reduce <= ++_walk) {
    int chosenTau = autoCorrelation();
    
    if (0 < chosenTau) {
      _intervalHistory->push_back(chosenTau);
      float mode[2];
      getModeValueWithCount(_intervalHistory, mode);
      
      //_phase = findPhase(mode[0]);
      //printf("%s %f %f %ld\n", _parameter->getWatchFrequencyString(), mode[0], mode[1], _phase);
      _phase = -1;
      
      if (START_TRACKING_THRESHOLD < mode[1]) {
        _phase = findPhase(mode[0]);
        
        if (0 <= _phase) {
          int interval = mode[0];
          _beatTracker = new Tracker(_phase, interval, _parameter);
          _excludedIntervals->push_back(interval);
          _backBeatTracker = new Tracker(_phase, interval, _parameter);
          
          _doTrack = true;
          
          int phase = std::max(_phase - errorTolerance, (long)0);
          for (; phase <= now; phase++) {
            power = getPower(onsetVectors->at(phase));
            _beatTracker->findOnset(phase, power);
            _backBeatTracker->findOnset(phase, power);
          }
        }
      }
    }
    _walk = 0;
  }
}

const float TrackingAgent::getPower(const float *vec) const {
  int i = _watchAt;
  int l = _watchLength;
  float p = 0;
  for (; i < l; i++) {
    p += vec[i];
  }
  return p;
}

const int TrackingAgent::autoCorrelation() {
  int W = _parameter->autoCorrelationSize;
  float numerator = 0, denominator = 0;
  int validMin = floor(60.0f / _parameter->ceilBPM * 44100 / (float)_timeIntervalFrames);
  int validMax = ceil(60.0f / (_parameter->floorBPM * _acRangeExpander) * 44100 / (float)_timeIntervalFrames);
  const std::vector<float*>* onsetVectors = _vectorizer->getOnsetVectors();
  const std::vector<bool>* isZeroFlags = _vectorizer->getIsZeroFlags();
  
  int c = onsetVectors->size();
  if (c < W + validMax) return 0;
  
  std::vector<float> denominators;
  std::vector<float> winResults;
  
  bool isFirst = true;
  int firstT = c - W;
  
  float maxR = 0;
  int chosenTau = 0;
  
  for (int tau = validMin; tau < validMax; tau++) {
    numerator = 0;
    for (int t = firstT; t < c; t++) {
      float* vec = onsetVectors->at(t);
      float* oldVec = onsetVectors->at(t - tau);
      bool vecIsZero = isZeroFlags->at(t);
      bool oldVecIsZero = isZeroFlags->at(t - tau);
      float win = 0;
      
      if (isFirst) {
        win = window(c - t, W);
        winResults.push_back(win);
        if (!vecIsZero) denominator += dot(vec, vec) * win;
      } else {
        int idx = t - firstT;
        win = winResults.at(idx);
      }
      if (!oldVecIsZero) numerator += dot(vec, oldVec) * win;
    }
    
    isFirst = false;
    float r = numerator / denominator;
    if (maxR < r) {
      maxR = r;
      chosenTau = tau;
    }
  }
  
  if (maxR < 0.1) {
    _acNoCorrelationCount++;
    if (_acNoCorrelationCount == 3) {
      _acRangeExpander = 1.0f / 4.0f;
    }
  }
  return chosenTau;
  
  return 0;
}

const float TrackingAgent::window(float t, int W) const {
  return 1.0f - 0.5f * t / W;
}

const float TrackingAgent::dot(const float *vec1, const float *vec2) const {
  float d = 0;
  int i = _watchAt;
  int l = _watchLength;
  for (; i < l; i++) {
    d += vec1[i] * vec2[i];
  }
  return d;
}

void TrackingAgent::getModeValueWithCount(const std::vector<float> *vec, float *res) const {
  __gnu_cxx::hash_map<int, float> counter;
  
  int m = 0;
  int mode = 0;
  
  for (int i = 0, l = vec->size(); i < l; i++) {
    float v = vec->at(i);
    bool excluded = false;
    for (int j = 0, jl = _excludedIntervals->size(); j < jl; j++) {
      if (v == excluded) {
        excluded = true;
        break;
      }
    }
    
    if (!excluded) {
      if (isnan(counter[v])) counter[v] = 0;
      counter[v]++;
      
      if (m < counter[v]) {
        m = counter[v];
        mode = v;
      }
    }
    
  }
  res[0] = mode;
  res[1] = counter[mode];
}

const long TrackingAgent::findPhase(int interval) const {
  const std::vector<float*>* onsetVectors = _vectorizer->getOnsetVectors();
  float maxPeak = 0;
  int stride = interval;
  int w = ceil(5.0 / ((float)_timeIntervalFrames / 44100.0));
  int i = 0, l = onsetVectors->size();
  float allPeaks[l];
  int size = 50;
  if (w * 2 + size < l) {
    for (i = 0; i < l; i++) {
      float v = 0;
      float* vec = onsetVectors->at(i);
      for (int j = _watchAt, jl = _watchLength; j < jl; j++) {
        v += vec[j];
      }
      allPeaks[i] = v;
      if (maxPeak < v) maxPeak = v;
    }
    
    float beats[w];
    bzero(beats, sizeof(float) * w);
    for (i = 0; i < w; i++) {
      if (i % stride == 0) {
        beats[i] = maxPeak;
      }
    }
    
    float maxR = 0;
    int maxRIdx = 0;
    for (int tau = 0; tau < w; tau++) {
      float r = 0;
      for (i = 0; i < w; i++) {
        r += allPeaks[i + tau] * beats[i];
      }
      if (maxR < r) {
        maxR = r;
        maxRIdx = tau;
      }
    }
    
    if (maxR == 0) return -1;
    float m = 0;
    int mi = 0;
    for (i = std::max(maxRIdx, 0), l = maxRIdx + size; i < l; i++) {
      if (m < allPeaks[i]) {
        m = allPeaks[i];
        mi = i;
      }
    }
    return mi;
  }
  return -1;
}

const float TrackingAgent::calcBPM(float interval) const {
  if (interval == 0) return 0;
  float bpm = 60.0f / (interval * (float)_timeIntervalFrames / 44100.0f);
  while (bpm < _parameter->floorBPM) bpm *= 2.0f;
  while (_parameter->ceilBPM < bpm) bpm *= 0.5f;
  return bpm;
}

const float TrackingAgent::getReliability() const {
  return (_beatTracker != 0 ? _beatTracker->getReliability() + _backBeatTracker->getReliability() : 0);
}

const long TrackingAgent::getPhase() const {
  return _phase;
}

const float TrackingAgent::getBPM() const {
  return _beatTracker != 0 ? calcBPM((float)_beatTracker->getTrackingBeatInterval()) : 0;
}

const OnsetVectorizer* TrackingAgent::getVectorizer() const {
  return _vectorizer;
}

void TrackingAgent::setVectorizer(OnsetVectorizer* vectorizer) {
  _vectorizer = vectorizer;
  int wf = _parameter->watchFrequency;
  if (wf == StrategyParameterLowFrequency) {
    _watchAt = 0;
    _watchLength = 2;
  } else if (wf == StrategyParameterMidFrequency) {
    _watchAt = 2;
    _watchLength = 5;
  } else if (wf == StrategyParameterHighFrequency) {
    _watchAt = 5;
    _watchLength = 7;
  } else {
    _watchAt = 0;
    _watchLength = 7;
  }
}
