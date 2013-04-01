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
#include <CoreFoundation/CoreFoundation.h>

class AudioFile {
public:
  AudioFile(CFURLRef fileURL);
  ~AudioFile();
  const AudioStreamBasicDescription& getInputFormat() const;
  const OSStatus setClientFormat(const AudioStreamBasicDescription clientASBD);
  const SInt64 getTotalFrames() const;
  const Boolean isVBR() const;
  void read(AudioBufferList* data, UInt64* cursor, UInt32* numFrames);
  void read(Float32* data, UInt64* cursor, UInt32* numFrames);
  
private:
  static OSStatus encoderProc(AudioConverterRef aconv,
                              UInt32* ioNumberDataPackets,
                              AudioBufferList* ioData,
                              AudioStreamPacketDescription** outDataPacketDescs,
                              void* userData);
  
  AudioFileID mAudioFileID;
  AudioConverterRef mAudioConverterRef;
  AudioStreamBasicDescription mInputFormat;
  AudioStreamBasicDescription mClientFormat;
  SInt64 mTotalFrames;
  UInt64 mFileSize;
  Boolean mIsVBR;
  
  UInt32 mNumPacketsToRead;
  UInt32 mConvertByteSize;

  AudioStreamPacketDescription* mPacketDescs;
  char* mConverterBuffer;
  UInt64* mCursor;
};