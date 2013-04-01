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

#include "AudioNode.hpp"

class AudioSampleProcessingNode : public AudioNode {
public:
  AudioSampleProcessingNode(AUGraph graph) : mGraph(graph), mBypass(false), mSource(0) {};
  
  virtual ~AudioSampleProcessingNode() {
    AUGraphRemoveNode(getAUGraph(), getAUNode());
  };
  
  void input(AudioNode& node, UInt32 srcOutputCh, UInt32 inputCh);
  void disconnectInput(UInt32 inputCh);
  
  // Does nothing. If you want something to do, you redefine the function on derived classes.
  void setFormat(const AudioStreamBasicDescription& asbd, AudioUnitScope scope, UInt32 bus) {}
  void setProperty(AudioUnitPropertyID pid, AudioUnitScope scope, UInt32 bus, const void* value, UInt32 size){}
  
  virtual void bypass(Boolean bypass);
  const Boolean doBypass() const;
  
  AUGraph getAUGraph() const;
  AUNode getAUNode() const;
  
  virtual const AURenderCallback getRenderer() const = 0;
  
private:
  AUGraph mGraph;
  Boolean mBypass;
  AudioNode* mSource;
};