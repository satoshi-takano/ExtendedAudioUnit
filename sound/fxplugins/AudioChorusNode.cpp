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
#include "AudioChorusNode.hpp"

AudioChorusNode::AudioChorusNode(AUGraph graph) : AudioSampleProcessingNode(graph), mDepth(0.5), mRate(5.0), mFrame(0)
{
  mDepthInfo.cfNameString = CFSTR("depth");
  mDepthInfo.minValue = 0.0;
  mDepthInfo.maxValue = 1.0;
  mDepthInfo.defaultValue = mDepth;
  
  mRateInfo.cfNameString = CFSTR("rate");
  mRateInfo.minValue = 0.0;
  mRateInfo.maxValue = 30.0;
  mRateInfo.defaultValue = mRate;
}

const AudioUnitParameterInfo AudioChorusNode::getParameterInfo(AudioUnitParameterID pid, AudioUnitScope scope, AudioUnitElement bus) const
{
  if (pid == 0) return mDepthInfo;
  else return mRateInfo;
}

const AudioUnitParameterValue AudioChorusNode::getParameter(AudioUnitParameterID pid, AudioUnitScope scope, AudioUnitElement bus) const
{
  if (pid == 0) return mDepth;
  else return mRate;
}

void AudioChorusNode::setParameter(AudioUnitParameterID pid, AudioUnitParameterValue value, AudioUnitScope scope, AudioUnitElement bus)
{
  if (pid == 0) mDepth = value;
  else mRate = value;
}

const AURenderCallback AudioChorusNode::getRenderer() const 
{
  return render;
}

OSStatus AudioChorusNode::render(void *inRefCon, 
                                 AudioUnitRenderActionFlags *ioActionFlags,
                                 const AudioTimeStamp *inTimeStamp,
                                 UInt32 inBusNumber, 
                                 UInt32 inNumberFrames,
                                 AudioBufferList *ioData)
{
//  AudioChorusNode* node = (AudioChorusNode*)inRefCon;
//  Float32 d, delta, radius, indexf, dpt, rate;
//  int indexi;
//  
//  rate = node->mRate;
//  
//  d = 44100.0 * 0.025;
//  dpt = 44100.0 * node->mDepth;
//  
//  for (int i = 0; i < inNumberFrames; i++) {
//
//  }
  
  return noErr;
}

/*
namespace iksound {
  class Chorus : public AudioEffect {
  private:
    float depth;
    float rate;
    
  public:
    inline void setDepth(float d) {
      if (1 < d) d = 1;
      else if (d < 0) d = 0;
      d /= 100;
      depth = d;
    }
    
    inline void setRate(float r) {
      rate = r / 10;
    }
    
    Chorus() : rate(0.1), depth(0.01) {}
    
    void inputAudioSamples(UInt32 inCurrentFrame, 
                           UInt32 inSrcTotalFrames,
                           AudioUnitSampleType* srcBufL,
                           AudioUnitSampleType* srcBufR,
                           AudioUnitSampleType* destSamplePtrL,
                           AudioUnitSampleType* destSamplePtrR) {
      float d, delta, radius, indexf, dpt;
      int indexi;
      
      d = 44100.0 * 0.025;
      dpt = 44100.0 * depth;
      
      radius = d + dpt * sin(2.0 * M_PI * rate * inCurrentFrame / 44100.0);
      indexf = (float)inCurrentFrame - radius;
      indexi = (int)indexf;
      delta = indexf - (float)indexi;
      
      if (0 <= indexi && indexi + 1 < inSrcTotalFrames) {
        *destSamplePtrL += delta * srcBufL[indexi + 1] + (1.0 - delta) * srcBufL[indexi];
        *destSamplePtrR += delta * srcBufR[indexi + 1] + (1.0 - delta) * srcBufR[indexi];
      }
      
      output(inCurrentFrame, inSrcTotalFrames, srcBufL, srcBufR, destSamplePtrL, destSamplePtrR);
    }
  };
}
*/