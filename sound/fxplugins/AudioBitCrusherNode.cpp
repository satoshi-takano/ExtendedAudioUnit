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
#include "AudioBitCrusherNode.hpp"

AudioBitCrusherNode::AudioBitCrusherNode(AUGraph graph) : AudioSampleProcessingNode(graph), mQuality(0.05)
{
  mQualityInfo.cfNameString = CFSTR("quality");
  mQualityInfo.minValue = 0.01;
  mQualityInfo.maxValue = 0.1;
  mQualityInfo.defaultValue = mQuality;
  mQualityInfo.unit = kAudioUnitParameterUnit_Ratio;
}

const AudioUnitParameterInfo AudioBitCrusherNode::getParameterInfo(AudioUnitParameterID pid, AudioUnitScope scope, AudioUnitElement bus) const
{
  if (pid == 0) return mQualityInfo;
  else return mQualityInfo;
};

const AudioUnitParameterValue AudioBitCrusherNode::getParameter(AudioUnitParameterID pid, AudioUnitScope scope, AudioUnitElement bus) const
{
  return mQuality;
}

void AudioBitCrusherNode::setParameter(AudioUnitParameterID pid, AudioUnitParameterValue value, AudioUnitScope scope, AudioUnitElement bus)
{
  if (pid == 0) mQuality = value;
}

const AURenderCallback AudioBitCrusherNode::getRenderer() const
{
  return render;
}

OSStatus AudioBitCrusherNode::render(void* inRefCon,
                                     AudioUnitRenderActionFlags* ioActionFlags,
                                     const  AudioTimeStamp* inTimeStamp,
                                     UInt32 inBusNumber,
                                     UInt32 inNumberFrames,
                                     AudioBufferList* ioData)
{
  if (*ioActionFlags == kAudioUnitRenderAction_PostRender) {
    AudioBitCrusherNode* node = (AudioBitCrusherNode*)inRefCon;
    if (node->doBypass()) return noErr;
    
    Float32* L = (Float32*)ioData->mBuffers[0].mData;
    Float32* R = (Float32*)ioData->mBuffers[1].mData;
    
    Float32 step = 1/pow(2, 8);
    Float32 phasor = 0;
    Float32 lastL = 0;
    Float32 lastR = 0;
    Float32 nfreq = node->mQuality;
    
    for (int i = 0; i < inNumberFrames; i++) {
      phasor = phasor + nfreq;
      if (phasor >= 1.0)  {
        phasor = phasor - 1.0;
        lastL = step * floor(L[i] / step);
        lastR = step * floor(R[i] / step);
      }
      L[i] = lastL;
      R[i] = lastR;
    }
  }
  return noErr;
}