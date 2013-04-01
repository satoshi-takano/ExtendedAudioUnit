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
#include "AudioFileNode.hpp"
#include "AUStreamBasicDescription.hpp"



AudioFileNode::AudioFileNode(AUGraph graph, CFURLRef urlRef) : AudioSourceNode(graph)
{
  checkError(ExtAudioFileOpenURL(urlRef, &mExtAudioFileRef), "ExtAudioFileOpenURL");
  
  AudioStreamBasicDescription inputFormat;
  UInt32 size = sizeof(AudioStreamBasicDescription);
  checkError(ExtAudioFileGetProperty(mExtAudioFileRef, kExtAudioFileProperty_FileDataFormat, &size, &inputFormat), "ExtAudioFileGetProperty FileDataFormat");
  
  UInt32 numChannels = inputFormat.mChannelsPerFrame;
  AUStreamBasicDescription clientFormat;
  clientFormat.mSampleRate = inputFormat.mSampleRate;
  clientFormat.SetFloat(numChannels, true);
  
  checkError(ExtAudioFileSetProperty(mExtAudioFileRef, kExtAudioFileProperty_ClientDataFormat, size, &clientFormat), "ExtAudioFileSetProperty ClientDataFormat");
  size = sizeof(SInt64);
  SInt64 totalFrames;
  checkError(ExtAudioFileGetProperty(mExtAudioFileRef, kExtAudioFileProperty_FileLengthFrames, &size, &totalFrames), "ExtAudioFileGetProperty FileLengthFrames");
  
  configuration(totalFrames, numChannels);
  ExtAudioFileSeek(mExtAudioFileRef, 0);
  
  mAudioFile = new AudioFile(urlRef);
  mAudioFile->setClientFormat(clientFormat);
}

AudioFileNode::~AudioFileNode()
{
  ExtAudioFileDispose(mExtAudioFileRef);
  delete mAudioFile;
}

/*
void AudioFileNode::readBuffer(const AudioTimeStamp *timeStamp, UInt32 busNumber, UInt64 position, UInt32 numberFrames, AudioBufferList *data)
{
  //checkError(ExtAudioFileSeek(mExtAudioFileRef, position), "ExtAudioFileSeek");
  //checkError(ExtAudioFileRead(mExtAudioFileRef, &numberFrames, data), "ExtAudioFileRead");
  mAudioFile->read(data, &position, &numberFrames);
}
*/

void AudioFileNode::readBuffer(const AudioTimeStamp *timeStamp, UInt32 busNumber, UInt64 position, UInt32& numberFrames, Float32 *data)
{
  mAudioFile->read(data, &position, &numberFrames);
}