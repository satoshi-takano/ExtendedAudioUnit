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
#include "AudioCaptureNode.hpp"

AudioCaptureNode::AudioCaptureNode(AUGraph graph) : AudioSampleProcessingNode(graph), mCaptureSampleLength(44100 * 20), mWritePosition(0)
{
  mCaptureSampleLengthInfo.cfNameString = CFSTR("capture length");
  mCaptureSampleLengthInfo.minValue = 44100.0;
  mCaptureSampleLengthInfo.maxValue = 2.0;
  mCaptureSampleLengthInfo.defaultValue = mCaptureSampleLength;
  mCaptureSampleLengthInfo.unit = kAudioUnitParameterUnit_SampleFrames;
  
  mCaptureBuffer = new Float32[mCaptureSampleLength]();
}

AudioCaptureNode::~AudioCaptureNode()
{
  delete[] mCaptureBuffer;
}

OSStatus AudioCaptureNode::render(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData)
{
  if (*ioActionFlags == kAudioUnitRenderAction_PostRender) {
    AudioCaptureNode* node = (AudioCaptureNode*)inRefCon;
    if (node->doBypass()) return noErr;
    
    UInt32 captureLength = node->mCaptureSampleLength;
    UInt32 writePosition = node->mWritePosition;
    UInt32 writeEnd = writePosition + inNumberFrames;
    Float32* capture = node->mCaptureBuffer;
    if (captureLength < writeEnd) {
      UInt32 slideOffset = writeEnd - captureLength;
      Float32* slide = &capture[slideOffset];
      memmove(capture, slide, sizeof(Float32) * (captureLength - slideOffset));
      writePosition -= slideOffset;
    }
    
    bool isInterleaved = ioData->mNumberBuffers == 1 && ioData->mBuffers[0].mNumberChannels == 2;
    if (isInterleaved) {
      Float32* destination = &capture[writePosition];
      memcpy(destination, ioData->mBuffers[0].mData, inNumberFrames);
    } 
    else {
      Float32* bufL =(Float32*)ioData->mBuffers[0].mData;
      Float32* bufR =(Float32*)ioData->mBuffers[1].mData;
      Float32* destination = &capture[writePosition];
      for (int i = 0; i < inNumberFrames; i++) {
        *destination++ = bufL[i];
        *destination++ = bufR[i];
      }
    }
    
    node->mWritePosition = writePosition + inNumberFrames;
  }
  return noErr;
}