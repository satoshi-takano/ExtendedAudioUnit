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
#include "AudioLevelNode.hpp"
#include <Accelerate/Accelerate.h>

AudioLevelNode::AudioLevelNode(AUGraph graph) : AudioSampleProcessingNode(graph), mLevel(1.0)
{
  mLevelInfo.cfNameString = CFSTR("Level");
  mLevelInfo.minValue = 0.0;
  mLevelInfo.maxValue = 100.0;
  mLevelInfo.defaultValue = mLevel * 100.0;
  mLevelInfo.unit = kAudioUnitParameterUnit_Percent;
}

AudioLevelNode::~AudioLevelNode()
{
}

const AudioUnitParameterInfo AudioLevelNode::getParameterInfo(AudioUnitParameterID pid, AudioUnitScope scope, AudioUnitElement bus) const
{
  return mLevelInfo;
};

const AudioUnitParameterValue AudioLevelNode::getParameter(AudioUnitParameterID pid, AudioUnitScope scope, AudioUnitElement bus) const
{
  return mLevel * 100.0;
}

void AudioLevelNode::setParameter(AudioUnitParameterID pid, AudioUnitParameterValue value, AudioUnitScope scope, AudioUnitElement bus)
{
  mLevel = value / 100.0;
}

const AURenderCallback AudioLevelNode::getRenderer() const
{
  return render;
}

OSStatus AudioLevelNode::render(void* inRefCon,
                                     AudioUnitRenderActionFlags* ioActionFlags,
                                     const  AudioTimeStamp* inTimeStamp,
                                     UInt32 inBusNumber,
                                     UInt32 inNumberFrames,
                                     AudioBufferList* ioData)
{
  if (*ioActionFlags == kAudioUnitRenderAction_PostRender) {
    AudioLevelNode* node = (AudioLevelNode*)inRefCon;
    if (node->doBypass()) return noErr;

    float vol = node->mLevel;
    for (int i = 0; i < ioData->mNumberBuffers; i++) {
      vDSP_vsmul((const float*)ioData->mBuffers[i].mData, 1, &vol, (float*)ioData->mBuffers[i].mData, 1, inNumberFrames);
    }
  }
  
  return noErr;
}