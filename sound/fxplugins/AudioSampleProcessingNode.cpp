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
#include "AudioSampleProcessingNode.hpp"

void AudioSampleProcessingNode::input(AudioNode& node, UInt32 srcOutputCh, UInt32 inputCh)
{
  ioValidate(node, srcOutputCh, inputCh);
  AudioUnit au = 0;
  checkError(AUGraphNodeInfo(node.getAUGraph(), node.getAUNode(), NULL, &au), 
             "AUGraphNodeInfo");
  checkError(AudioUnitAddRenderNotify(au, getRenderer(), this), 
             "AudioUnitAddRenderNotify");
  mSource = &node;
};

void AudioSampleProcessingNode::disconnectInput(UInt32 inputCh)
{
  AudioUnit au;
  checkError(AUGraphNodeInfo(mSource->getAUGraph(), mSource->getAUNode(), NULL, &au), 
             "AUGraphNodeInfo");
  checkError(AudioUnitRemoveRenderNotify(au, getRenderer(), this), "AudioUnitRemoveRenderNotify");
}

void AudioSampleProcessingNode::bypass(Boolean bypass)
{
  mBypass = bypass;
}

const Boolean AudioSampleProcessingNode::doBypass() const
{
   return mBypass;
}

AUGraph AudioSampleProcessingNode::getAUGraph() const
{
  return mGraph;
}

AUNode AudioSampleProcessingNode::getAUNode() const
{
  if (mSource)
    return mSource->getAUNode();
  else return 0;
}