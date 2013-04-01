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
#include "AudioRecordingNode.hpp"

AudioRecordingNode::AudioRecordingNode(AUGraph graph, CFURLRef outfileURL, AudioFileTypeID type, AudioStreamBasicDescription& srcFormat, AudioStreamBasicDescription& fileFormat) : AudioSampleProcessingNode(graph)
{
  UInt32 size = sizeof(AudioStreamBasicDescription);
  
  // create temporaly audio file in local storage
  checkError(ExtAudioFileCreateWithURL(outfileURL, type, &fileFormat, NULL, kAudioFileFlags_EraseFile, &mLocalAudioFile),
             "Fail ExtAudioFileCreateWithURL");
  checkError(ExtAudioFileSetProperty(mLocalAudioFile, kExtAudioFileProperty_ClientDataFormat, size, &srcFormat),
             "ExtAudioFileSetProperty");
}

AudioRecordingNode::~AudioRecordingNode()
{
  ExtAudioFileDispose(mLocalAudioFile);
}

void AudioRecordingNode::startRecording()
{
  mRecording = true;
}

void AudioRecordingNode::stopRecording()
{
  mRecording = false;
  ExtAudioFileDispose(mLocalAudioFile);
}

const AURenderCallback AudioRecordingNode::getRenderer() const
{
  return render;
}

OSStatus AudioRecordingNode::render(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData)
{
  if (*ioActionFlags == kAudioUnitRenderAction_PostRender) {
    AudioRecordingNode* node = (AudioRecordingNode*)inRefCon;
    if (node->mRecording) {
      checkError(ExtAudioFileWriteAsync(node->mLocalAudioFile, inNumberFrames, ioData), "ExtAudioFileWriteAsync");
    }
  }
  return noErr;
}