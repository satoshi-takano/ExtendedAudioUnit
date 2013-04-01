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

#include "AudioSourceNode.hpp"
#include <mach/mach_time.h>

class AudioMetronomeNode : public AudioSourceNode {
public:
  AudioMetronomeNode(AUGraph graph);
  
  const unsigned int getBPM() const;
  void setBPM(unsigned int BPM);
  
  void readBuffer(const AudioTimeStamp* timeStamp, UInt32 busNumber, UInt64 position, UInt32 numberFrames, Float32* data);
  
private:
  Float32 mFrequency;
  Float32 mNormalFrequency;
  Float32 mAccentFrequency;
  double mLatestTime;
  Float32 mBeatIntervalMsec;
  unsigned int mBPM;
  
  unsigned int mBeatIndex;
  Float32 mBeatDuration;
  Float32 mRestBeatDuration;
};