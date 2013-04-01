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
#include "AudioMetronomeNode.hpp"
#include "AUStreamBasicDescription.hpp"


AudioMetronomeNode::AudioMetronomeNode(AUGraph graph) : AudioSourceNode(graph), mNormalFrequency(440.0), mAccentFrequency(880.0), mLatestTime(0), mBeatIntervalMsec(0), mBPM(120), mBeatIndex(0)
{
  AUStreamBasicDescription clientFormat;
  clientFormat.mSampleRate = 44100.0;
  clientFormat.SetFloat(2);
  configuration(44100, 2);
  
  mFrequency = mNormalFrequency;
  mBeatDuration = 5.0 / mBPM;
  mRestBeatDuration = mBeatDuration;
  
  setFormat(clientFormat, kAudioUnitScope_Output, 0);
}

const unsigned int AudioMetronomeNode::getBPM() const
{
  return mBPM;
}

void AudioMetronomeNode::setBPM(unsigned int BPM)
{
  mBPM = BPM;
  mBeatDuration = 5.0 / mBPM;
  mRestBeatDuration = mBeatDuration;
  mBeatIndex = 3;
  mBeatIntervalMsec = 0;
}

void AudioMetronomeNode::readBuffer(const AudioTimeStamp *timeStamp, UInt32 busNumber, UInt64 position, UInt32 numberFrames, Float32 *data)
{
  float f = mNormalFrequency;
  if (mBeatIndex == 3) {
    f = mAccentFrequency;
  }
  
  if (0 < mRestBeatDuration) {
    float freq = f * 2.0 * M_PI / 44100.0;
    static int p = 0;
    for (int i = 0, len = numberFrames; i < len; i++) {
      Float32 sample = sinf(freq * p++) * 0.5;
      *data++ = sample;
      *data++ = sample;
    }
  } else {
    //memset(data, 0, numberFrames * sizeof(Float32) * getNumberOfSourceChannels());
  }
  
  double elapsed = (CFAbsoluteTimeGetCurrent() - mLatestTime);
  if (mLatestTime)
    mBeatIntervalMsec += elapsed;
  
  Float32 threshold = 60.0 / mBPM;
  if (threshold <= mBeatIntervalMsec) {
    mBeatIndex++;
    if (3 < mBeatIndex) mBeatIndex = 0;
    mBeatIntervalMsec = mBeatIntervalMsec - threshold;
    mRestBeatDuration = mBeatDuration;
  }
  
  mRestBeatDuration -= elapsed;
  mLatestTime = CFAbsoluteTimeGetCurrent();
}