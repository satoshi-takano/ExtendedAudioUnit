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

#include <AudioToolbox/AudioToolbox.h>
#include <AudioUnit/AudioUnit.h>
#include "AudioException.hpp"
#include <string.h>

class AudioNode {
public:
  virtual ~AudioNode(){};
  
  virtual const AudioStreamBasicDescription getFormat(AudioUnitScope scope, AudioUnitElement bus) const
  {
    AudioStreamBasicDescription asbd;
    AudioUnit unit;
    checkError(AUGraphNodeInfo(getAUGraph(), getAUNode(), NULL, &unit), "AUGraphNodeInfo AudioFileMemoryBufferNode");
    UInt32 size = sizeof(AudioStreamBasicDescription);
    checkError(AudioUnitGetProperty(unit, kAudioUnitProperty_StreamFormat, scope, bus, &asbd, &size), "AudioUnitGetProperty");
    return asbd;
  }
  
  virtual void setFormat(const AudioStreamBasicDescription& asbd, AudioUnitScope scope, UInt32 bus) = 0;
  virtual void input(AudioNode& node, UInt32 srcOutputCh, UInt32 inputCh) = 0;
  virtual void disconnectInput(UInt32 inputCh) = 0;
  virtual void bypass(Boolean bypass) = 0;
  
  virtual AUNode getAUNode() const = 0;
  virtual AUGraph getAUGraph() const = 0;
  
  virtual const UInt32 numberOfInputs() const = 0;
  virtual const UInt32 numberOfOutputs() const = 0;
  virtual const UInt32 numberOfParameters(UInt32 scope) const = 0;
  virtual const Boolean doBypass() const = 0;
  
  virtual const AudioUnitParameterInfo 
  getParameterInfo(AudioUnitParameterID pid, AudioUnitScope scope, AudioUnitElement bus) const = 0;
  virtual const AudioUnitParameterValue getParameter(AudioUnitParameterID pid, AudioUnitScope scope, AudioUnitElement bus) const = 0;
  const AudioUnitParameterValue getParameterValueRatio(AudioUnitParameterID pid, AudioUnitScope scope, AudioUnitElement bus) const 
  {
    AudioUnitParameterInfo info = getParameterInfo(pid, scope, 0);
    AudioUnitParameterValue value = (getParameter(pid, scope, 0) - info.minValue) / (info.maxValue - info.minValue);
    return value;
  }
  void setParameterValueRatio(AudioUnitParameterID pid, float ratio, AudioUnitScope scope, AudioUnitElement bus)
  {
    AudioUnitParameterInfo info = getParameterInfo(pid, scope, bus);
    AudioUnitParameterValue value = info.minValue + (info.maxValue - info.minValue) * ratio;
    setParameter(pid, value, scope, bus);
  }
  
  virtual void 
  setParameter(AudioUnitParameterID pid, AudioUnitParameterValue value, AudioUnitScope scope, AudioUnitElement bus) = 0;
  
  virtual void
  setProperty(AudioUnitPropertyID pid, AudioUnitScope scope, UInt32 bus, const void* value, UInt32 size) = 0;
  
  static void zeroBuffer(AudioBufferList *ioData,
                         UInt32 inNumberFrames,
                         UInt32 numberOfChannels,
                         size_t sampleTypeByteSize)
  {
    size_t size = sampleTypeByteSize * inNumberFrames;
    
    if (ioData->mNumberBuffers == 2) {
      memset(ioData->mBuffers[0].mData, 0, size);
      memset(ioData->mBuffers[1].mData, 0, size);
    } else {
      if (numberOfChannels == 2) {
        memset(ioData->mBuffers[0].mData, 0, size * 2);
      } else {
        memset(ioData->mBuffers[0].mData, 0, size);
      }
    }
  }
  
protected:
  void ioValidate(AudioNode& node, UInt32 srcOutputCh, UInt32 inputCh)
  {
    if ((node.numberOfOutputs()-1) < srcOutputCh) {
      AudioNodeException exception("The output channel you specified is larger than number of output ports on this node.");
      throw exception;
    }
    if ((numberOfInputs() - 1) < inputCh) {
      AudioNodeException exception("The inputs channel you specified is larger than number of input ports on this node.");
      throw exception;
    }
  }
};