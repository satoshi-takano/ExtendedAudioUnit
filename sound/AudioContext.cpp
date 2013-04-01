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

#include <iostream>
#include "AudioContext.hpp"
#include "AudioException.hpp"

AudioContext::AudioContext()
{
  NewAUGraph(&mAUGraph);
  AUGraphOpen(mAUGraph);
  AUGraphInitialize(mAUGraph);
}

AudioContext::~AudioContext() 
{
  AUGraphStop(mAUGraph);
  AUGraphClose(mAUGraph);
  AUGraphUninitialize(mAUGraph);
  DisposeAUGraph(mAUGraph);
}

void AudioContext::start() 
{
  checkError(AUGraphStart(mAUGraph), "AUGraphStart");
}

void AudioContext::stop()
{
  checkError(AUGraphStop(mAUGraph), "AUGraphStop");
}

void AudioContext::clearAllConnections()
{
  checkError(AUGraphClearConnections(mAUGraph), "AUGraphClearConnections");
}

void AudioContext::update(const Boolean synchronous)
{
  if (synchronous) {
    checkError(AUGraphUpdate(mAUGraph, NULL), "AUGraphUpdate");
  } else {
    Boolean flag;
    checkError(AUGraphUpdate(mAUGraph, &flag), "AUGraphUpdate with flag");
  }
}

AUGraph AudioContext::getGraph() 
{
  return mAUGraph;
}

const Float32 AudioContext::getCPULoad() const
{
  Float32 cpuLoad;
  AUGraphGetCPULoad(mAUGraph, &cpuLoad);
  return cpuLoad;
}