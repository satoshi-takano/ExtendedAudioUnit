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

#include "PeakDetector.hpp"
#include <math.h>

PeakDetector::PeakDetector(int index, OnsetVectorizer* vectorizer, int watchFrequencyStartBandIndex, int watchFrequencyEndBandIndex) : _index(index), _vectorizer(vectorizer), _hadOnset(false), _preonsetPower(0), _watchFrequencyStartBandIndex(watchFrequencyStartBandIndex), _watchFrequencyEndBandIndex(watchFrequencyEndBandIndex), _watchFrequencyBandLength(watchFrequencyEndBandIndex - watchFrequencyStartBandIndex), _time(0) {
  _magnitudes = new std::deque<float*>();
  for (int i = 0; i < 4; i++) {
    _magnitudes->push_back((float*)malloc(sizeof(float) * _watchFrequencyBandLength));
  }
  
  _movingAverageWindowSize = 1;
  _movingAverageWindow = new std::deque<float>();
  for (int i = 0; i < _movingAverageWindowSize; i++) {
    _movingAverageWindow->push_back(0);
  }
  
#ifdef WAVE
  _numVertices = ceil(44100.0f * 30.0f / 256.0f);
  _vertices = new Vertex[_numVertices]();
  for (int i = 0; i < _numVertices; i++) {
    _vertices[i].color[0] = 1.0;
    _vertices[i].color[1] = 1.0;
    _vertices[i].color[2] = 1.0;
    _vertices[i].color[3] = 1.0;
    _vertices[i].position[0] = 0;
    _vertices[i].position[1] = -3.0 + 6.0 * (float)i / (float)(_numVertices - 1);
  }
  
  _render = new RenderingEngine(320, 480, _vertices, _numVertices);
#endif
}

PeakDetector::~PeakDetector() {
  while (_magnitudes->size()) {
    free(_magnitudes->front());
    _magnitudes->erase(_magnitudes->begin());
  }
  delete _movingAverageWindow;
  delete _magnitudes;
}

void PeakDetector::pushMagnitudes(float* mags) {
  float* front = _magnitudes->front();
  _magnitudes->pop_front();
  memcpy(front, &mags[_watchFrequencyStartBandIndex], sizeof(float) * _watchFrequencyBandLength);
  _magnitudes->push_back(front);
  
  
  if (3 <= _time) {
    int nowT = 2;
    float d = 0, D = 0,
    p, pp, np,
    pm, pl, pr, ppm, nm, nl, nr;
    
    for (int i = 1, l = _watchFrequencyBandLength - 1; i < l; i++) {
      p = _magnitudes->at(nowT)[i];
      ppm = _magnitudes->at(nowT - 2)[i];
      float* preMags = _magnitudes->at(nowT - 1);
      pm = preMags[i];
      pl = preMags[i - 1];
      pr = preMags[i + 1];
      float* nextMags = _magnitudes->at(nowT + 1);
      nm = nextMags[i];
      nl = nextMags[i - 1];
      nr = nextMags[i + 1];
      
      pp = max(max(pm, max(pr, pl)), ppm);
      np = min(nm, min(nl, nr));
      
      if (pp < p && pp < np) {
        d = p - pp + min(0.0f, nm - p);
        D += d;
      }
    }
    _movingAverageWindow->pop_front();
    _movingAverageWindow->push_back(D);
    
    float avg = 0;
    for (int i = 0; i < _movingAverageWindowSize; i++) {
      avg += _movingAverageWindow->at(i);
    }
    avg /= _movingAverageWindowSize;
    
    if (_hadOnset && avg < _preonsetPower)
    {
      _vectorizer->addOnset(_index, _time - 1, _preonsetPower);
#ifdef WAVE
      //_vertices[_time].position[0] = avg/100.0;
#endif
    }
    else
      _vectorizer->addOnset(_index, _time - 1, 0);
    
    _hadOnset = _preonsetPower < avg;
    _preonsetPower = avg;
  } else if (0 < _time) {
    _vectorizer->addOnset(_index, _time - 1, 0);
  }
  
  _time++;
}

#ifdef WAVE
void PeakDetector::render() {
  _render->render(GL_LINE_STRIP);
}
#endif