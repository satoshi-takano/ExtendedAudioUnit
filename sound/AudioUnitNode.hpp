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

#ifndef IKKitDevelop_AUNode_h
#define IKKitDevelop_AUNode_h

#include "AudioNode.hpp"
#include <AudioToolbox/AudioToolbox.h>
#include "AUComponentDescription.hpp"

class AudioUnitNode : public AudioNode {
public:
  AudioUnitNode(AUGraph graph, const AudioComponentDescription& cd);
  virtual ~AudioUnitNode();
  
  virtual void setFormat(const AudioStreamBasicDescription& asbd, AudioUnitScope scope, UInt32 bus);
  
  virtual void input(AudioNode& node, UInt32 srcOutputCh, UInt32 inputCh);
  virtual void disconnectInput(UInt32 inputCh);
  
  void bypass(Boolean bypass);
  const Boolean doBypass() const;
  
  const UInt32 numberOfInputs() const;
  const UInt32 numberOfOutputs() const;
  
  const UInt32 numberOfParameters(UInt32 scope) const;
  
  AUGraph getAUGraph() const { return mGraph;};
  AUNode getAUNode() const { return mNode;};
  
  const AudioUnitParameterInfo getParameterInfo(AudioUnitParameterID pid, AudioUnitScope scope, AudioUnitElement bus) const;
  void setParameter(AudioUnitParameterID pid, AudioUnitParameterValue value, AudioUnitScope scope, UInt32 bus);
  const AudioUnitParameterValue getParameter(AudioUnitParameterID pid, AudioUnitScope scope, AudioUnitElement bus) const;

  void getProperty(AudioUnitPropertyID pid, AudioUnitScope scope, UInt32 bus, void* outValue, UInt32 size);
  void setProperty(AudioUnitPropertyID pid, AudioUnitScope scope, UInt32 bus, const void* value, UInt32 size);

private:
  AUGraph mGraph;
  AUNode mNode;
  AudioUnit mUnit;
};

#endif
