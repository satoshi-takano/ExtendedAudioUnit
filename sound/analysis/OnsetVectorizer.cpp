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

#include "OnsetVectorizer.hpp"

OnsetVectorizer::OnsetVectorizer(int dimension, std::vector<TrackingAgent*>* agents) : _dimention(dimension), _agents(agents) {
    _onsetVectors = new std::vector<float*>();
    _isZeroFlags = new std::vector<bool>();
    
    for (int i = 0, l = _agents->size(); i < l; i++) {
        _agents->at(i)->setVectorizer(this);
    }
}

OnsetVectorizer::~OnsetVectorizer() {
    for (int i = 0, l = _onsetVectors->size(); i < l; i++) {
        free(_onsetVectors->at(i));
    }
    delete _onsetVectors;
    delete _isZeroFlags;
}

void OnsetVectorizer::addOnset(int index, long frame, float power) {
    size_t size = sizeof(float) * _dimention;

    while ((long)(_onsetVectors->size() - 1) < frame) {
        float* vec = (float*)malloc(size);
        bzero(vec, size);
        _onsetVectors->push_back(vec);
        _isZeroFlags->push_back(true);
    }
    if (power != 0) _isZeroFlags->at(frame) = false;
    _onsetVectors->at(frame)[index] = power;
    
    if (index == _dimention - 1) {
        for (int i = 0, l = _agents->size(); i < l; i++) {
            _agents->at(i)->updateOnsetVector();
        }
    }
}

const std::vector<bool>* OnsetVectorizer::getIsZeroFlags() const {
    return _isZeroFlags;
}

const std::vector<float*>* OnsetVectorizer::getOnsetVectors() const {
    return _onsetVectors;
}