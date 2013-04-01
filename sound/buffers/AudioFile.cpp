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
#include "AudioFile.hpp"
#include "AudioException.hpp"
#include "AudioSourceNode.hpp"

AudioFile::AudioFile(CFURLRef fileURL) : mAudioFileID(0), mAudioConverterRef(0), mTotalFrames(0), mFileSize(0), mIsVBR(false), mNumPacketsToRead(0), mPacketDescs(0), mConverterBuffer(0), mCursor(0)
{
  checkError(AudioFileOpenURL(fileURL, kAudioFileReadPermission, NULL, &mAudioFileID), "AudioFileOpenURL");
  UInt32 size = sizeof(AudioStreamBasicDescription);
  checkError(AudioFileGetProperty(mAudioFileID, kAudioFilePropertyDataFormat, &size, &mInputFormat), "AudioFileGetProperty");
  
  size = sizeof(UInt64);
  checkError(AudioFileGetProperty(mAudioFileID, kAudioFilePropertyAudioDataByteCount, &size, &mFileSize), "AudioFileGetProperty");
  if (mInputFormat.mBytesPerFrame == 0) {
    mIsVBR = true;
  }
  
  UInt64 totalPackets;
  size = sizeof(UInt64);
  checkError(AudioFileGetProperty(mAudioFileID, kAudioFilePropertyAudioDataPacketCount, &size, &totalPackets), "AudioFileGetProperty");
  
  if (!mIsVBR) {
    mTotalFrames = totalPackets;
  } else {
    AudioFramePacketTranslation translation;
    translation.mPacket = totalPackets;
    size = sizeof(AudioFramePacketTranslation);
    checkError(AudioFileGetProperty(mAudioFileID, kAudioFilePropertyPacketToFrame, &size, &translation), "AudioFileGetProperty");
    mTotalFrames = translation.mFrame;
  }
  
  mCursor = (UInt64*)calloc(1, sizeof(UInt64));
  *mCursor = 0;
  std::cout << "Total Packets : " << mTotalFrames << std::endl;
}

AudioFile::~AudioFile()
{
  free(mCursor);
  free(mPacketDescs);
  free(mConverterBuffer);
}

const AudioStreamBasicDescription& AudioFile::getInputFormat() const
{
  return mInputFormat;
}

const OSStatus AudioFile::setClientFormat(const AudioStreamBasicDescription clientASBD)
{
  checkError(AudioConverterNew(&mInputFormat, &clientASBD, &mAudioConverterRef), "AudioConverterNew");
  mClientFormat = clientASBD;
  
  UInt32 size = sizeof(UInt32);
  UInt32 maxPacketSize;
  checkError(AudioFileGetProperty(mAudioFileID, kAudioFilePropertyPacketSizeUpperBound, &size, &maxPacketSize), "AudioFileGetProperty");
  
  if (mIsVBR) {
    mPacketDescs = (AudioStreamPacketDescription*)calloc(mNumPacketsToRead, sizeof(AudioStreamPacketDescription));
  }
  
  // set magic cookie to the AudioConverter
  {
    UInt32 cookieSize;
    OSStatus err = AudioFileGetPropertyInfo(mAudioFileID, 
                                            kAudioFilePropertyMagicCookieData, 
                                            &cookieSize, 
                                            NULL);
    
    if (err == noErr && cookieSize > 0){
      char *magicCookie = (char*)malloc(sizeof(UInt8) * cookieSize);
      UInt32	magicCookieSize = cookieSize;
      AudioFileGetProperty(mAudioFileID,
                           kAudioFilePropertyMagicCookieData,
                           &magicCookieSize,
                           magicCookie);
      
      AudioConverterSetProperty(mAudioConverterRef,
                                kAudioConverterDecompressionMagicCookie,
                                magicCookieSize,
                                magicCookie);
      free(magicCookie);
    }
  }
  
  return noErr;
}

OSStatus AudioFile::encoderProc(AudioConverterRef aconv, UInt32 *ioNumberDataPackets, AudioBufferList *ioData, AudioStreamPacketDescription **outDataPacketDescs, void *userData)
{
  AudioFile* ud = (AudioFile*)userData;
  
//  UInt32 outNumBytes;
  *ioNumberDataPackets = ud->mNumPacketsToRead;
  //checkError(AudioFileReadPackets(ud->mAudioFileID, false, &outNumBytes, ud->mPacketDescs, *ud->mCursor, ioNumberDataPackets, ud->mConverterBuffer), "AudioFileReadPackets");
  
  ioData->mBuffers[0].mData = ud->mConverterBuffer;
  ioData->mBuffers[0].mDataByteSize = ud->mConvertByteSize;
  ioData->mBuffers[0].mNumberChannels = ud->mInputFormat.mChannelsPerFrame;
  
  *ud->mCursor += *ioNumberDataPackets;
  
  if (outDataPacketDescs) {
    if (ud->mPacketDescs) *outDataPacketDescs = ud->mPacketDescs;
    else *outDataPacketDescs = NULL;
  }
  return noErr;
}

void AudioFile::read(AudioBufferList* buf, UInt64* cursor, UInt32* numFrames)
{
  AudioFramePacketTranslation t;
  UInt32 size = sizeof(AudioFramePacketTranslation);
  t.mFrame = *cursor;
  AudioFileGetProperty(mAudioFileID, kAudioFilePropertyFrameToPacket, &size, &t);
  *mCursor = t.mPacket;
  
  UInt32 numFramesToRead = *numFrames;//t.mFrameOffsetInPacket + *numFrames;
  AudioBufferList* tmpbuf = AudioSourceNode::createAudioBufferList(2, false, numFramesToRead, sizeof(Float32));
  checkError(AudioConverterFillComplexBuffer(mAudioConverterRef, encoderProc, this, &numFramesToRead, tmpbuf, NULL),
             "AudioConverterFillComplexBuffer");

  memcpy(buf->mBuffers[0].mData, tmpbuf->mBuffers[0].mData, *numFrames * sizeof(Float32));
  memcpy(buf->mBuffers[1].mData, tmpbuf->mBuffers[1].mData, *numFrames * sizeof(Float32));

  AudioSourceNode::deleteAudioBufferList(tmpbuf);
  if (numFramesToRead == 0) {
    AudioConverterReset(mAudioConverterRef);
  }
  *numFrames = numFramesToRead;
}

void AudioFile::read(Float32 *data, UInt64 *cursor, UInt32 *numFrames)
{
  AudioFramePacketTranslation t;
  UInt32 size = sizeof(AudioFramePacketTranslation);
  t.mFrame = *cursor;
  AudioFileGetProperty(mAudioFileID, kAudioFilePropertyFrameToPacket, &size, &t);
  *mCursor = t.mPacket;
  
  AudioFramePacketTranslation t2;
  t2.mFrame = *numFrames;
  AudioFileGetProperty(mAudioFileID, kAudioFilePropertyFrameToPacket, &size, &t2);
  UInt32 numPacketsToRead = t2.mPacket ? t2.mPacket : 1;
  
  AudioBytePacketTranslation t3;
  t3.mPacket = numPacketsToRead;
  size = sizeof(AudioBytePacketTranslation);
  AudioFileGetProperty(mAudioFileID, kAudioFilePropertyPacketToByte, &size, &t3);
  
  if (mConverterBuffer) free(mConverterBuffer);
  mConverterBuffer = (char*)malloc(t3.mByte);
  mNumPacketsToRead = numPacketsToRead;
  
  UInt32 outNumBytes;
  checkError(AudioFileReadPackets(mAudioFileID, false, &outNumBytes, mPacketDescs, *mCursor, &numPacketsToRead, mConverterBuffer), "AudioFileReadPackets");
  mConvertByteSize = outNumBytes;
  
  UInt32 numFramesToConvert = t.mFrameOffsetInPacket + *numFrames;
  bool interleaved = true;
  interleaved = !(mClientFormat.mFormatFlags & kAudioFormatFlagIsNonInterleaved);
  AudioBufferList* tmpbuf = AudioSourceNode::createAudioBufferList(2, interleaved, numFramesToConvert, sizeof(Float32));
  checkError(AudioConverterFillComplexBuffer(mAudioConverterRef, encoderProc, this, &numFramesToConvert, tmpbuf, NULL),
             "AudioConverterFillComplexBuffer");
  
  if (interleaved) {
    Float32* sample = (Float32*)tmpbuf->mBuffers[0].mData;
    memcpy(data, &sample[t.mFrameOffsetInPacket], numFramesToConvert * sizeof(Float32) * mClientFormat.mChannelsPerFrame);
  }
  
  AudioSourceNode::deleteAudioBufferList(tmpbuf);
  
  if (numFramesToConvert == 0) {
    AudioConverterReset(mAudioConverterRef);
  }
  
  *numFrames = numFramesToConvert;
}

const SInt64 AudioFile::getTotalFrames() const
{
  return mTotalFrames;
}

const Boolean AudioFile::isVBR() const
{
  return mIsVBR;
}