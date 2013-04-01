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
#include "AudioFileMemoryBufferNode.hpp"
#include "AUStreamBasicDescription.hpp"

AudioFileMemoryBufferNode::AudioFileMemoryBufferNode(const AUGraph graph, const CFURLRef urlRef) : AudioSourceNode(graph)
{
  ExtAudioFileRef extFileRef;
  checkError(ExtAudioFileOpenURL(urlRef, 
                                 &extFileRef), "ExtAudioFileOpenURL");
  
  AudioStreamBasicDescription inputFormat;
  UInt32 size = sizeof(AudioStreamBasicDescription);
  checkError(ExtAudioFileGetProperty(extFileRef, 
                                     kExtAudioFileProperty_FileDataFormat, 
                                     &size, 
                                     &inputFormat), "ExtAudioFileGetProperty FileDataFormat");
  
  UInt32 numChannels = inputFormat.mChannelsPerFrame;
  AUStreamBasicDescription clientFormat;
  clientFormat.mSampleRate = inputFormat.mSampleRate;
  clientFormat.SetFloat(numChannels, true);

  checkError(ExtAudioFileSetProperty(extFileRef, 
                                     kExtAudioFileProperty_ClientDataFormat, 
                                     sizeof(clientFormat),
                                     &clientFormat), "ExtAudioFileSetProperty ClientDataFormat");
  
  SInt64 totalFrames;
  checkError(ExtAudioFileGetProperty(extFileRef, 
                                     kExtAudioFileProperty_FileLengthFrames, 
                                     &size, 
                                     &totalFrames), "ExtAudioFileGetProperty FileLengthFrames");
  
  mBufferList = createAudioBufferList(numChannels, true, totalFrames, sizeof(Float32));
  
  ExtAudioFileSeek(extFileRef, 0);
  
  UInt32 readFrameSize = totalFrames;
  checkError(ExtAudioFileRead(extFileRef, &readFrameSize, mBufferList),
             "ExtAudioFileRead");
  ExtAudioFileDispose(extFileRef);
  configuration(totalFrames, numChannels);
}

AudioFileMemoryBufferNode::~AudioFileMemoryBufferNode()
{
  deleteAudioBufferList(mBufferList);
}

void AudioFileMemoryBufferNode::readBuffer(const AudioTimeStamp* timeStamp, UInt32 busNumber, UInt64 position, UInt32& numberFrames, Float32* data)
{
  Float32* sample = (Float32*)mBufferList->mBuffers[0].mData;
  int ch = getNumberOfSourceChannels();
  memcpy(data, &sample[position * ch], numberFrames * sizeof(Float32) * ch);
}