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
#include "AudioParallelNode.hpp"

const size_t BUFFER_SIZE = sizeof(Float32) * 2 * 4096; // 4096frame分とっとけば十分かと

AudioParallelNode::AudioParallelNode(AUGraph graph) : AudioSourceNode(graph), mSourceNode(0)
{
  configuration(44100, 2);
  mBuffer = (Float32*)malloc(BUFFER_SIZE);
  memset(mBuffer, 1, BUFFER_SIZE);
}

AudioParallelNode::AudioParallelNode(AUGraph graph, AudioNode* sourceNode) : AudioSourceNode(graph), mSourceNode(sourceNode) {
  AudioUnit au;
  checkError(AUGraphNodeInfo(graph, sourceNode->getAUNode(), NULL, &au), 
             "AUGraphNodeInfo");
  checkError(AudioUnitAddRenderNotify(au, render, this), 
             "AudioUnitAddRenderNotify");
  
  configuration(44100, 2);
  mBuffer = (Float32*)malloc(BUFFER_SIZE);
  memset(mBuffer, 1, BUFFER_SIZE);
}

AudioParallelNode::~AudioParallelNode()
{
  free(mBuffer);
  mBuffer = 0;
  if (mSourceNode) {
    AudioUnit au;
    checkError(AUGraphNodeInfo(getAUGraph(), mSourceNode->getAUNode(), NULL, &au), "AUGraphNodeInfo");
    checkError(AudioUnitRemoveRenderNotify(au, render, this), "AudioUnitRemoveRenderNotify");
  }
}

void AudioParallelNode::setSourceNode(AudioNode* sourceNode)
{
  AudioUnit au;
  if (mSourceNode) {
    checkError(AUGraphNodeInfo(getAUGraph(), mSourceNode->getAUNode(), NULL, &au), "AUGraphNodeInfo");
    checkError(AudioUnitRemoveRenderNotify(au, render, this), "AudioUnitRemoveRenderNotify");
  }
  checkError(AUGraphNodeInfo(getAUGraph(), sourceNode->getAUNode(), NULL, &au), 
             "AUGraphNodeInfo");
  checkError(AudioUnitAddRenderNotify(au, render, this), 
             "AudioUnitAddRenderNotify");
  mSourceNode = sourceNode;
}

OSStatus AudioParallelNode::render(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData)
{
  if (*ioActionFlags == kAudioUnitRenderAction_PostRender) {
    AudioParallelNode* para = static_cast<AudioParallelNode*>(inRefCon);
    if (!para->isRunning()) return noErr;
    
    Float32* buf = para->mBuffer;
    if (buf) {
      bzero(para->mBuffer, BUFFER_SIZE);
      vDSP_vadd(buf, 2, (Float32*)ioData->mBuffers[0].mData, 1, buf, 2, inNumberFrames);
      vDSP_vadd(&buf[1], 2, (Float32*)ioData->mBuffers[1].mData, 1, &buf[1], 2, inNumberFrames);
    }
  }
  return noErr;
};

void AudioParallelNode::readBuffer(const AudioTimeStamp *timeStamp, UInt32 busNumber, UInt64 position, UInt32& numberFrames, Float32 *data)
{
  setFrameOffset(0);
  memcpy(data, mBuffer, numberFrames * sizeof(Float32) * 2);
};