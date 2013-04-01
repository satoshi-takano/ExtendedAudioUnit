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
#include "AudioTremoroNode.hpp"

AudioTremoroNode::AudioTremoroNode(AUGraph graph) : AudioSampleProcessingNode(graph), mDepth(0.5), mRate(5.0), mCurrentFrame(0)
{
  mDepthInfo.cfNameString = CFSTR("depth");
  mDepthInfo.minValue = 0.0;
  mDepthInfo.maxValue = 1.0;
  mDepthInfo.defaultValue = mDepth;
  mDepthInfo.unit = kAudioUnitParameterUnit_Ratio;
  
  mRateInfo.cfNameString = CFSTR("rate");
  mRateInfo.minValue = 0.0;
  mRateInfo.maxValue = 30.0;
  mRateInfo.defaultValue = mRate;
  mRateInfo.unit = kAudioUnitParameterUnit_Rate;
}

const AudioUnitParameterInfo AudioTremoroNode::getParameterInfo(AudioUnitParameterID pid, AudioUnitScope scope, AudioUnitElement bus) const
{
  if (pid == 0) return mDepthInfo;
  else return mRateInfo;
}

const AudioUnitParameterValue AudioTremoroNode::getParameter(AudioUnitParameterID pid, AudioUnitScope scope, AudioUnitElement bus) const
{
  if (pid == 0) return mDepth;
  else return mRate;
}

void AudioTremoroNode::setParameter(AudioUnitParameterID pid, AudioUnitParameterValue value, AudioUnitScope scope, AudioUnitElement bus)
{
  if (pid == 0) {
    mDepth = value;
  } else {
    mRate = value;
  }
}

const AURenderCallback AudioTremoroNode::getRenderer() const
{
  return render;
}

OSStatus AudioTremoroNode::render(void* inRefCon,
                       AudioUnitRenderActionFlags* ioActionFlags,
                       const  AudioTimeStamp* inTimeStamp,
                       UInt32 inBusNumber,
                       UInt32 inNumberFrames,
                       AudioBufferList* ioData)
{
  if (*ioActionFlags == kAudioUnitRenderAction_PostRender) {
    AudioTremoroNode* tremoro = (AudioTremoroNode*)inRefCon;
    if (tremoro->doBypass()) return noErr;
    
    Float32 *sampleL = (Float32*)ioData->mBuffers[0].mData;
    Float32 *sampleR = (Float32*)ioData->mBuffers[1].mData;
    Float32 a;
    Float32 d = tremoro->mDepth;
    Float32 r = tremoro->mRate;
    UInt32 c = tremoro->mCurrentFrame;
    
    for (int i = 0; i < inNumberFrames; i++) {
      a = (1 + d * sin(2.0 * M_PI * r * ((Float32)c + (Float32)i) / 44100.0)) / (1.0+d);
      *sampleL++ = *sampleL * a;
      *sampleR++ = *sampleR * a;
    }
    tremoro->mCurrentFrame += inNumberFrames;
  }
  return noErr;
}