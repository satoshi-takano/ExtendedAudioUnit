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
#include "AudioVariSpeedNode.hpp"
#include "AUStreamBasicDescription.hpp"

AudioVariSpeedNode::AudioVariSpeedNode(AUGraph graph) : AudioSampleProcessingNode(graph), mSpeedRatio(1.0), mNeedWindow(false)
{
  mSpeedRatioInfo.cfNameString = CFSTR("speed ratio");
  mSpeedRatioInfo.minValue = 0.015;
  mSpeedRatioInfo.maxValue = 4.0;
  mSpeedRatioInfo.defaultValue = mSpeedRatio;
  mSpeedRatioInfo.unit = kAudioUnitParameterUnit_Rate;
  
  AUComponentDescription desc;
  mConverter = new AudioUnitNode(graph, desc.setFormatConverterType(kAudioUnitSubType_AUConverter));
  AUStreamBasicDescription asbd;
  asbd.SetFloat(2);
  asbd.mSampleRate = 44100.0 * mSpeedRatio;
  mConverter->setFormat(asbd, 0, 0);
  AudioSampleProcessingNode::input(*mConverter, 0, 0);
  asbd.mSampleRate = 44100.0;
  mConverter->setFormat(asbd, 0, 0);
}

AudioVariSpeedNode::~AudioVariSpeedNode()
{
  delete mConverter;
}

void AudioVariSpeedNode::input(AudioNode &node, UInt32 srcOutputCh, UInt32 inputCh)
{
  mConverter->input(node, srcOutputCh, inputCh);
}

void AudioVariSpeedNode::bypass(Boolean bypass)
{
  AudioSampleProcessingNode::bypass(bypass);
  mConverter->bypass(bypass);
}

const AudioUnitParameterInfo AudioVariSpeedNode::getParameterInfo(AudioUnitParameterID pid, AudioUnitScope scope, AudioUnitElement bus) const
{
  return mSpeedRatioInfo;
}

const AudioUnitParameterValue AudioVariSpeedNode::getParameter(AudioUnitParameterID pid, AudioUnitScope scope, AudioUnitElement bus) const
{
  return mSpeedRatio;
}

void AudioVariSpeedNode::setParameter(AudioUnitParameterID pid, AudioUnitParameterValue value, AudioUnitScope scope, AudioUnitElement bus)
{
  mSpeedRatio = value;
  AUStreamBasicDescription asbd;
  asbd.SetFloat(2);
  asbd.mSampleRate = 44100.0 / mSpeedRatio;
  mConverter->setFormat(asbd, 0, 0);
  mNeedWindow = true;
}

const AURenderCallback AudioVariSpeedNode::getRenderer() const
{
  return render;
}

OSStatus AudioVariSpeedNode::render(void* inRefCon,
                                  AudioUnitRenderActionFlags* ioActionFlags,
                                  const  AudioTimeStamp* inTimeStamp,
                                  UInt32 inBusNumber,
                                  UInt32 inNumberFrames,
                                  AudioBufferList* ioData)
{
  if (*ioActionFlags == kAudioUnitRenderAction_PostRender) {
    AudioVariSpeedNode* node = (AudioVariSpeedNode*)inRefCon;
    if (node->doBypass()) return noErr;
    
    if (node->mSpeedRatio == node->mSpeedRatioInfo.minValue) {
      AudioNode::zeroBuffer(ioData, inNumberFrames, node->numberOfInputs(), sizeof(Float32));
    } else {
      if (node->mNeedWindow) {
        float* mWindow = (float*)calloc(inNumberFrames, sizeof(float));
        vDSP_hann_window(mWindow, inNumberFrames, 0);
        //float vol = 1.0;
        for (int i = 0; i < ioData->mNumberBuffers; i++) {
          float* sample = (float*)ioData->mBuffers[i].mData;
          vDSP_vmul(sample, 1, mWindow, 1, sample, 1, inNumberFrames);
          //vDSP_vsmul(sample, 1, &vol, 1, sample, 1, inNumberFrames);
        }
        free(mWindow);
      }
    }
    
    node->mNeedWindow = false;
  }
  return noErr;
}