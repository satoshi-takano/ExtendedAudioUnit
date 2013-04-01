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

#include "Tracker.hpp"

Tracker::Tracker(int phase, int beatInterval, StrategyParameter* param) : _phase(phase), _trackingBeatInterval(beatInterval), _parameter(param), _reliability(0), _predictionFrame(phase + beatInterval) {
  _predictionField = new float[param->errorTolerance * 2 + 1]();
}

Tracker::~Tracker() {
  delete[] _predictionField;
}

void Tracker::findOnset(int now, float power) {
  int errorTlerance = _parameter->errorTolerance;
  _maxPower = std::max(_maxPower, power);
  int rest = _predictionFrame + errorTlerance - now;
  
  if (rest <= errorTlerance * 2) {
    _predictionField[errorTlerance * 2 - rest] = power;
  }
  
  if (rest == 0) {
    float maxInPredictField = 0;
    int maxIndexInPredictField = 0;
    for (int i = 0, l = errorTlerance * 2 + 1; i < l; i++) {
      if (maxInPredictField < _predictionField[i]) {
        maxInPredictField = _predictionField[i];
        maxIndexInPredictField = i;
      }
    }
    
    int peakFrame = maxIndexInPredictField == 0 ? _predictionFrame : _predictionFrame - (errorTlerance - maxIndexInPredictField);
    
    if (maxInPredictField < 0.1) {
      _preBeatTime = peakFrame;
      _predictionFrame = peakFrame + _trackingBeatInterval;
      _reliability *= 0.8;
    } else {
      _preBeatTime = peakFrame;
      _predictionFrame = peakFrame + _trackingBeatInterval;
      _reliability += maxInPredictField / _maxPower;
    }
  }
}

const float Tracker::getReliability() const {
  return _reliability;
}

const int Tracker::getTrackingBeatInterval() const {
  return _trackingBeatInterval;
}