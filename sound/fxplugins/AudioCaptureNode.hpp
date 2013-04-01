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

#pragma once
#include "AudioSampleProcessingNode.hpp"

enum {
  kAudioCaptureNodeParam_CaptureFrameSize,
};

class AudioCaptureNode : public AudioSampleProcessingNode {
public:
  AudioCaptureNode(AUGraph graph);
  ~AudioCaptureNode();
  
  const UInt32 numberOfInputs() const {return 1;}
  const UInt32 numberOfOutputs() const {return 1;}
  
  const UInt32 numberOfParameters(UInt32 scope) const { return 1;}
  const AudioUnitParameterInfo getParameterInfo(AudioUnitParameterID pid, AudioUnitScope scope, AudioUnitElement bus) const{
    return mCaptureSampleLengthInfo;
  }
  const AudioUnitParameterValue getParameter(AudioUnitParameterID pid, AudioUnitScope scope, AudioUnitElement bus) const{
    return mCaptureSampleLength;}
  void setParameter(AudioUnitParameterID pid, AudioUnitParameterValue value, AudioUnitScope scope, AudioUnitElement bus){};
  
  const Float32* getCapturedSamples() const {
    return mCaptureBuffer;
  }
  
  const AURenderCallback getRenderer() const {return render;}
  
private:
  static OSStatus render(void* inRefCon,
                         AudioUnitRenderActionFlags* ioActionFlags,
                         const  AudioTimeStamp* inTimeStamp,
                         UInt32 inBusNumber,
                         UInt32 inNumberFrames,
                         AudioBufferList* ioData);
  
  UInt32 mCaptureSampleLength;
  AudioUnitParameterInfo mCaptureSampleLengthInfo;
  
  Float32* mCaptureBuffer;
  UInt32 mWritePosition;
};