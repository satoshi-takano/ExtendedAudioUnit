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
#include "StrategyParameter.hpp"
#include "Tracker.hpp"

class OnsetVectorizer;

class TrackingAgent {
public:
  TrackingAgent(int timeIntervalFrames, StrategyParameter* param);
  ~TrackingAgent();
  
  void updateOnsetVector();
  const float getReliability() const;
  const long getPhase() const;
  const float getBPM() const;
  
  const OnsetVectorizer* getVectorizer() const;
  void setVectorizer(OnsetVectorizer* vectorizer);
  
  
private:
  const float getPower(const float* vec) const;
  const int autoCorrelation();
  const float window(float t, int W) const;
  const float dot(const float* vec1, const float* vec2) const;
  void getModeValueWithCount(const std::vector<float>* vec, float* res) const;
  const long findPhase(int interval) const;
  const float calcBPM(float interval) const;
  
  OnsetVectorizer* _vectorizer;
  int _timeIntervalFrames;
  StrategyParameter* _parameter;
  int _watchAt;
  int _watchLength;
  
  std::vector<float>* _intervalHistory;
  long _phase;
  bool _doTrack;
  std::vector<float>* _excludedIntervals;
  
  int _reduce;
  int _walk;
  
  float _acRangeExpander;
  int _acNoCorrelationCount;
  
  Tracker* _beatTracker;
  Tracker* _backBeatTracker;
};