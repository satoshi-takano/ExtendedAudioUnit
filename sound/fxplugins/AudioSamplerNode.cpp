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
#include "AudioSamplerNode.hpp"
#include <Accelerate/Accelerate.h>

AudioSamplerNode::AudioSamplerNode(AUGraph graph, UInt32 bufFrameLength) : AudioSampleProcessingNode(graph), mLoopFrames(44100.0 * 0.5), mCurrentReadPosition(0), mCurrentWritePosition(0), mIsOut(true), mBufferFrameLength(bufFrameLength)
{
  mLoopFramesInfo.cfNameString = CFSTR("loop frames");
  mLoopFramesInfo.minValue = 0;
  mLoopFramesInfo.maxValue = 44100.0;
  mLoopFramesInfo.defaultValue = mLoopFrames;
  mLoopFramesInfo.unit = kAudioUnitParameterUnit_SampleFrames;
  
  mSampleBufferL = (Float32*)calloc(bufFrameLength, sizeof(Float32));
  mSampleBufferR = (Float32*)calloc(bufFrameLength, sizeof(Float32));
}

AudioSamplerNode::~AudioSamplerNode()
{
  free(mSampleBufferL);
  free(mSampleBufferR);
}

const AudioUnitParameterInfo AudioSamplerNode::getParameterInfo(AudioUnitParameterID pid, AudioUnitScope scope, AudioUnitElement bus) const
{
  return mLoopFramesInfo;
}

const AudioUnitParameterValue AudioSamplerNode::getParameter(AudioUnitParameterID pid, AudioUnitScope scope, AudioUnitElement bus) const
{
  return mLoopFrames;
}

void AudioSamplerNode::setParameter(AudioUnitParameterID pid, AudioUnitParameterValue value, AudioUnitScope scope, AudioUnitElement bus)
{
  mLoopFrames = value;
}

const AURenderCallback AudioSamplerNode::getRenderer() const 
{
  return render;
}

void AudioSamplerNode::startSampling()
{
  mIsOut = false;
  mCurrentReadPosition = 0;
  mCurrentWritePosition = 0;
  bypass(false);
}

void AudioSamplerNode::stopSampling()
{
  mIsOut = true;
  bypass(true); 
  if (mCurrentWritePosition == 0) {
    mCurrentWritePosition = 1024;
  }
}

void AudioSamplerNode::bypass(Boolean bypass)
{
  AudioSampleProcessingNode::bypass(bypass);
  mCurrentWritePosition = 0;
}

void AudioSamplerNode::setBuffer(Float32 *interleavedBuffer, UInt32 numberOfFrames)
{
  UInt32 size = numberOfFrames * sizeof(Float32);
  bzero(mSampleBufferL, size);
  bzero(mSampleBufferR, size);
  vDSP_vadd(mSampleBufferL, 1, interleavedBuffer, 2, mSampleBufferL, 1, numberOfFrames);
  vDSP_vadd(mSampleBufferR, 1, &interleavedBuffer[1], 2, mSampleBufferR, 1, numberOfFrames);
  mCurrentReadPosition = numberOfFrames;
  mCurrentWritePosition = 0;
}

void AudioSamplerNode::writeToFile(AudioFileID& file) const
{
  if (mCurrentReadPosition == 0) return;
  
  Float32* data = new Float32[mCurrentReadPosition * 2]();
  vDSP_vadd(data, 2, mSampleBufferL, 1, data, 2, mCurrentReadPosition);
  vDSP_vadd(&data[1], 2, mSampleBufferR, 1, &data[1], 2, mCurrentReadPosition);

  UInt32 size = mCurrentReadPosition * sizeof(Float32) * 2;
  UInt32 frames = mCurrentReadPosition;
  checkError(AudioFileWritePackets(file, false, size, NULL, 0, &frames, data), "AudioFileWritePackets");
  delete[] data;
}

const float AudioSamplerNode::getFilled() const
{
  return (float)mCurrentReadPosition / (float)mBufferFrameLength;
}

OSStatus AudioSamplerNode::render(void *inRefCon, 
                                AudioUnitRenderActionFlags *ioActionFlags,
                                const AudioTimeStamp *inTimeStamp,
                                UInt32 inBusNumber, 
                                UInt32 inNumberFrames,
                                AudioBufferList *ioData)
{
  if (*ioActionFlags == kAudioUnitRenderAction_PostRender) {
    AudioSamplerNode* node = (AudioSamplerNode*)inRefCon;
    if (node->doBypass()) {
      return noErr;
    }
    
    Float32* sampleL = (Float32*)ioData->mBuffers[0].mData;
    Float32* sampleR = (Float32*)ioData->mBuffers[1].mData;
    Float32* bufL = node->mSampleBufferL;
    Float32* bufR = node->mSampleBufferR;
    UInt32 readPosition = node->mCurrentReadPosition;
    if (node->mIsOut) {
      UInt32 writePosition = node->mCurrentWritePosition;
      UInt32 rest = 0;
      
      if (readPosition <= writePosition + inNumberFrames) {
        rest = writePosition + inNumberFrames - readPosition;
        vDSP_vadd(sampleL, 1, &bufL[writePosition], 1, sampleL, 1, rest);
        vDSP_vadd(sampleR, 1, &bufR[writePosition], 1, sampleR, 1, rest);
        node->bypass(true);
      } else {
        vDSP_vadd(sampleL, 1, &bufL[writePosition], 1, sampleL, 1, inNumberFrames);
        vDSP_vadd(sampleR, 1, &bufR[writePosition], 1, sampleR, 1, inNumberFrames);
      }
      writePosition += inNumberFrames;
      node->mCurrentWritePosition = writePosition;
    } 
    else {
      int writeFrames = inNumberFrames;
      if (node->mBufferFrameLength < readPosition + writeFrames) 
        writeFrames = node->mBufferFrameLength - readPosition;
      if (0 < writeFrames) {
        size_t size = writeFrames * sizeof(Float32);
        memcpy(&bufL[readPosition], sampleL, size);
        memcpy(&bufR[readPosition], sampleR, size);
        node->mCurrentReadPosition += writeFrames;
      }
    }
  }
  return noErr;
}