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

#include "AudioGateNode.hpp"
#include <Accelerate/Accelerate.h>

AudioGateNode::AudioGateNode(AUGraph graph) : AudioSampleProcessingNode(graph), mDepth(0.5), mRate(5.0), mNeedProcessFrames(0), mProcessedFrames(0), mNeedGate(false)
{
  mDepthInfo.cfNameString = CFSTR("depth");
  mDepthInfo.minValue = 0.0;
  mDepthInfo.maxValue = 1.0;
  mDepthInfo.defaultValue = mDepth;
  mDepthInfo.unit = kAudioUnitParameterUnit_Ratio;
  
  mRateInfo.cfNameString = CFSTR("rate");
  mRateInfo.minValue = 0.0;
  mRateInfo.maxValue = 50.0;
  mRateInfo.defaultValue = mRate;
  mRateInfo.unit = kAudioUnitParameterUnit_Rate;
}


const AudioUnitParameterInfo AudioGateNode::getParameterInfo(AudioUnitParameterID pid, AudioUnitScope scope, AudioUnitElement bus) const
{
  if (pid == 0) return mDepthInfo;
  else return mRateInfo;
}

const AudioUnitParameterValue AudioGateNode::getParameter(AudioUnitParameterID pid, AudioUnitScope scope, AudioUnitElement bus) const
{
  if (pid == 0) return mDepth;
  else return mRate;
}

void AudioGateNode::setParameter(AudioUnitParameterID pid, AudioUnitParameterValue value, AudioUnitScope scope, AudioUnitElement bus)
{
  if (pid == 0) {
    mDepth = value;
  } else {
    mRate = value;
    mNeedProcessFrames = (UInt32)roundf(44100.0 / mRate);
  }
}

const AURenderCallback AudioGateNode::getRenderer() const
{
  return render;
}

OSStatus AudioGateNode::render(void* inRefCon,
                                  AudioUnitRenderActionFlags* ioActionFlags,
                                  const  AudioTimeStamp* inTimeStamp,
                                  UInt32 inBusNumber,
                                  UInt32 inNumberFrames,
                                  AudioBufferList* ioData)
{
  if (*ioActionFlags == kAudioUnitRenderAction_PostRender) {
    AudioGateNode* node = static_cast<AudioGateNode*>(inRefCon);
    if (node->doBypass()) {
      node->mProcessedFrames = 0;
      return noErr;
    }
    
    Float32 *sampleL = (Float32*)ioData->mBuffers[0].mData;
    Float32 *sampleR = (Float32*)ioData->mBuffers[1].mData;
    Float32 d = 1.0 - node->mDepth;
    
    int need = node->mNeedProcessFrames;
    int processed = node->mProcessedFrames;
    
    int gateFrames = need - processed;
    bool needGate = node->mNeedGate;
    
    gateFrames = inNumberFrames < gateFrames ? inNumberFrames : gateFrames;
    if (needGate) {
      vDSP_vsmul(sampleL, 1, &d, sampleL, 1, gateFrames);
      vDSP_vsmul(sampleR, 1, &d, sampleR, 1, gateFrames);
    }
    node->mProcessedFrames += gateFrames;
    if (need <= node->mProcessedFrames) {
        node->mProcessedFrames = node->mProcessedFrames - need + (inNumberFrames - gateFrames);
      node->mNeedGate = !needGate;
    }
  }
  return noErr;
}