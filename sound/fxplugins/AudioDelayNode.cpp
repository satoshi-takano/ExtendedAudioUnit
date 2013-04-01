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
#include "AudioDelayNode.hpp"

AudioDelayNode::AudioDelayNode(AUGraph graph) : AudioSampleProcessingNode(graph), mDelayTime(1.0), mDryWetMix(0.5), mRepeat(3)
{
  mDelayTimeInfo.cfNameString = CFSTR("delay time");
  mDelayTimeInfo.minValue = 0.05;
  mDelayTimeInfo.maxValue = 1.0;
  mDelayTimeInfo.defaultValue = mDelayTime;
  mDelayTimeInfo.unit = kAudioUnitParameterUnit_Seconds;
  
  mDryWetMixInfo.cfNameString = CFSTR("dry wet mix");
  mDryWetMixInfo.minValue = 0.0;
  mDryWetMixInfo.maxValue = 100.0;
  mDryWetMixInfo.defaultValue = mDryWetMix * 100.0;
  mDryWetMixInfo.unit = kAudioUnitParameterUnit_Percent;
  
  mRepeatInfo.cfNameString = CFSTR("repeats");
  mRepeatInfo.minValue = 0.0;
  mRepeatInfo.maxValue = 5.0;
  mRepeatInfo.defaultValue = mRepeat;
  mRepeatInfo.unit = kAudioUnitParameterUnit_Indexed;
  
  mSampleBufferL = new std::deque<Float32>();
  mSampleBufferR = new std::deque<Float32>();
  
  // Addition +1 is use by delaysampInd < bufL->size()
  mMaxBufferLength = mDelayTimeInfo.maxValue * 44100 * mRepeatInfo.maxValue + 1;
}

AudioDelayNode::~AudioDelayNode()
{
  delete mSampleBufferL;
  delete mSampleBufferR;
}

const AudioUnitParameterInfo AudioDelayNode::getParameterInfo(AudioUnitParameterID pid, AudioUnitScope scope, AudioUnitElement bus) const
{
  if (pid == 0) return mDelayTimeInfo;
  else if (pid == 1) return mDryWetMixInfo;
  else return mRepeatInfo;
}

const AudioUnitParameterValue AudioDelayNode::getParameter(AudioUnitParameterID pid, AudioUnitScope scope, AudioUnitElement bus) const
{
  if (pid == 0) return mDelayTime;
  else if (pid == 1) return mDryWetMix * 100.0;
  else return mRepeat;
}

void AudioDelayNode::setParameter(AudioUnitParameterID pid, AudioUnitParameterValue value, AudioUnitScope scope, AudioUnitElement bus)
{
  if (pid == 0) mDelayTime = value;
  else if (pid == 1) mDryWetMix = value / 100.0;
  else mRepeat = (UInt32)roundf(value);
}

const AURenderCallback AudioDelayNode::getRenderer() const 
{
  return render;
}

OSStatus AudioDelayNode::render(void *inRefCon, 
                                 AudioUnitRenderActionFlags *ioActionFlags,
                                 const AudioTimeStamp *inTimeStamp,
                                 UInt32 inBusNumber, 
                                 UInt32 inNumberFrames,
                                 AudioBufferList *ioData)
{
  if (*ioActionFlags == kAudioUnitRenderAction_PostRender) {
    AudioDelayNode* node = (AudioDelayNode*)inRefCon;
    if (node->doBypass()) {
      if (node->mSampleBufferR->size()) {
        node->mSampleBufferL->clear();
        node->mSampleBufferR->clear();
      }
      return noErr;
    }
    
    Float32 *sampleL = (Float32*)ioData->mBuffers[0].mData;
    Float32 *sampleR = (Float32*)ioData->mBuffers[1].mData;
    
    UInt32 delaysampInd;
    UInt32 repeat = node->mRepeat;
    Float32 mixRatio = node->mDryWetMix;
    std::deque<Float32>* bufL = node->mSampleBufferL;
    std::deque<Float32>* bufR = node->mSampleBufferR;
    
    UInt32 k = (UInt32)roundf(node->mDelayTime * 44100.0);
    
    for (int i = 0; i < inNumberFrames; i++) {
      Float32 rowL = *sampleL;
      Float32 rowR = *sampleR;

      long long size = bufL->size();
      for (int j = 0; j < repeat; j++) {
        delaysampInd = k * j;
        if (delaysampInd < (size - 1)) {
          Float32 ratio = powf(mixRatio, (float)j);
          Float32 delayL = ratio * bufL->at(delaysampInd);
          Float32 delayR = ratio * bufR->at(delaysampInd);
          *sampleL += delayL;
          *sampleR += delayR;
        }
      }
      bufL->push_front(rowL);
      bufR->push_front(rowR);
      if (node->mMaxBufferLength < size) {
        bufL->pop_back();
        bufR->pop_back();
      }

      sampleL++;
      sampleR++;
    }
  }
  return noErr;
}