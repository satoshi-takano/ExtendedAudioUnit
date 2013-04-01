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
#include <Accelerate/Accelerate.h>
#include "AudioDistortionNode.hpp"

AudioDistortionNode::AudioDistortionNode(AUGraph graph) : AudioSampleProcessingNode(graph), mGain(20.0), mLevel(0.3)
{
  mGainInfo.cfNameString = CFSTR("gain");
  mGainInfo.minValue = 0.0;
  mGainInfo.maxValue = 200.0;
  mGainInfo.defaultValue = mGain;
  mGainInfo.unit = kAudioUnitParameterUnit_LinearGain;
  
  mLevelInfo.cfNameString = CFSTR("level");
  mLevelInfo.minValue = 0.0;
  mLevelInfo.maxValue = 100.0;
  mLevelInfo.defaultValue = mLevel * 100.0;
  mLevelInfo.unit = kAudioUnitParameterUnit_Percent;
}

const AudioUnitParameterInfo AudioDistortionNode::getParameterInfo(AudioUnitParameterID pid, AudioUnitScope scope, AudioUnitElement bus) const
{
  if (pid == 0) return mGainInfo;
  else  return mLevelInfo;
};

const AudioUnitParameterValue AudioDistortionNode::getParameter(AudioUnitParameterID pid, AudioUnitScope scope, AudioUnitElement bus) const
{
  if (pid == 0) return mGain;
  else return mLevel * 100.0;
}

void AudioDistortionNode::setParameter(AudioUnitParameterID pid, AudioUnitParameterValue value, AudioUnitScope scope, AudioUnitElement bus)
{
  if (pid == 0) mGain = value;
  else mLevel = value / 100.0;
}

const AURenderCallback AudioDistortionNode::getRenderer() const
{
  return render;
}

OSStatus AudioDistortionNode::render(void* inRefCon,
                                     AudioUnitRenderActionFlags* ioActionFlags,
                                     const  AudioTimeStamp* inTimeStamp,
                                     UInt32 inBusNumber,
                                     UInt32 inNumberFrames,
                                     AudioBufferList* ioData)
{
  if (*ioActionFlags == kAudioUnitRenderAction_PostRender) {
    AudioDistortionNode* distortion = (AudioDistortionNode*)inRefCon;
    if (distortion->doBypass()) return noErr;
    
    for (int i = 0; i < ioData->mNumberBuffers; i++) {
      Float32* samples = (Float32*)ioData->mBuffers[i].mData;
      Float32 gain = distortion->mGain;
      Float32 level = distortion->mLevel;
      
      vDSP_vsmul(samples, 1, &gain, samples, 1, inNumberFrames);
      float low = -1.0;
      float high = 1.0;
      vDSP_vclip(samples, 1, &low, &high, samples, 1, inNumberFrames);
      vDSP_vsmul(samples, 1, &level, samples, 1, inNumberFrames);
    }
  }
  return noErr;
}