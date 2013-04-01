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

#include <iostream>
#include <string>
#include <CoreAudio/CoreAudioTypes.h>

class AUStreamBasicDescription : public AudioStreamBasicDescription {
public:
  AUStreamBasicDescription(){};
  
  void SetCanonical(UInt32 nChannels, bool interleaved)
  {
    mFormatID = kAudioFormatLinearPCM;
    int sampleSize = sizeof(AudioSampleType);
    mFormatFlags = kAudioFormatFlagsCanonical;
		mBitsPerChannel = 8 * sampleSize;
		mChannelsPerFrame = nChannels;
		mFramesPerPacket = 1;
		if (interleaved)
			mBytesPerPacket = mBytesPerFrame = nChannels * sampleSize;
		else {
			mBytesPerPacket = mBytesPerFrame = sampleSize;
			mFormatFlags |= kAudioFormatFlagIsNonInterleaved;
		}
  }
  
  void	SetAUCanonical(UInt32 nChannels, bool interleaved)
	{
		mFormatID = kAudioFormatLinearPCM;
#if CA_PREFER_FIXED_POINT
		mFormatFlags = kAudioFormatFlagsCanonical | (kAudioUnitSampleFractionBits << kLinearPCMFormatFlagsSampleFractionShift);
#else
		mFormatFlags = kAudioFormatFlagsCanonical;
#endif
		mChannelsPerFrame = nChannels;
		mFramesPerPacket = 1;
		mBitsPerChannel = 8 * sizeof(AudioUnitSampleType);
		if (interleaved)
			mBytesPerPacket = mBytesPerFrame = nChannels * sizeof(AudioUnitSampleType);
		else {
			mBytesPerPacket = mBytesPerFrame = sizeof(AudioUnitSampleType);
			mFormatFlags |= kAudioFormatFlagIsNonInterleaved;
		}
	}
  
  void SetAIFF(UInt32 nChannels)
  {
    mSampleRate         = 44100.0;
    mFormatID			= kAudioFormatLinearPCM;
    mFormatFlags		= kAudioFormatFlagIsBigEndian 
    | kLinearPCMFormatFlagIsSignedInteger 
    | kLinearPCMFormatFlagIsPacked;
    mFramesPerPacket	= 1;
    mChannelsPerFrame	= nChannels;
    mBitsPerChannel    = 16;
    mBytesPerPacket    = 2 * nChannels;
    mBytesPerFrame		= 2 * nChannels;
    mReserved			= 0;
  }
  
  void SetFloat(UInt32 nChannels, Boolean interleaved = false)
  {
    UInt32 sampleSize = sizeof(AudioUnitSampleType);
    mFormatID         = kAudioFormatLinearPCM;
    mFormatFlags      = kAudioFormatFlagIsFloat|kAudioFormatFlagIsPacked;
    mChannelsPerFrame = nChannels;
    mFramesPerPacket  = 1;
    mBitsPerChannel   = 8 * sampleSize;
    if (interleaved) {
      mBytesPerPacket = mBytesPerFrame = nChannels * sampleSize;
    } else {
      mBytesPerPacket = mBytesPerFrame = sampleSize;
      mFormatFlags |= kAudioFormatFlagIsNonInterleaved;
    }
  }
  
  static void printASBD(AudioStreamBasicDescription& audioFormat){
    UInt32 mFormatFlags = audioFormat.mFormatFlags;
    std::string fmtflg;
    if(mFormatFlags & kAudioFormatFlagIsFloat) fmtflg += "kAudioFormatFlagIsFloat";
    if(mFormatFlags & kAudioFormatFlagIsBigEndian) fmtflg += "\nkAudioFormatFlagIsBigEndian";
    if(mFormatFlags & kAudioFormatFlagIsSignedInteger) fmtflg += "\nkAudioFormatFlagIsSignedInteger";
    if(mFormatFlags & kAudioFormatFlagIsPacked) fmtflg += "\nkAudioFormatFlagIsPacked";
    if(mFormatFlags & kAudioFormatFlagIsNonInterleaved) fmtflg += "\nkAudioFormatFlagIsNonInterleaved";
    
    if(mFormatFlags & kAudioFormatFlagIsAlignedHigh) fmtflg += "\nkAudioFormatFlagIsAlignedHigh";
    if(mFormatFlags & kAudioFormatFlagIsNonMixable) fmtflg += "\nkAudioFormatFlagIsNonMixable";
    if(mFormatFlags & (kAudioUnitSampleFractionBits << kLinearPCMFormatFlagsSampleFractionShift)) fmtflg += "\n(kAudioUnitSampleFractionBits << kLinearPCMFormatFlagsSampleFractionShift)";
    
    char formatID[5];
    *(UInt32 *)formatID = CFSwapInt32HostToBig(audioFormat.mFormatID);
    formatID[4] = '\0';
    
    printf("\n");
    printf("audioFormat.mSampleRate       = %.2f;\n",audioFormat.mSampleRate);
    printf("audioFormat.mFormatID         = '%-4.4s;'\n",formatID);
    std::cout << fmtflg << std::endl;
    printf("audioFormat.mBytesPerPacket   = %lu;\n",audioFormat.mBytesPerPacket);
    printf("audioFormat.mFramesPerPacket  = %lu;\n",audioFormat.mFramesPerPacket);
    printf("audioFormat.mBytesPerFrame    = %lu;\n",audioFormat.mBytesPerFrame);
    printf("audioFormat.mChannelsPerFrame = %lu;\n",audioFormat.mChannelsPerFrame);
    printf("audioFormat.mBitsPerChannel   = %lu;\n",audioFormat.mBitsPerChannel);
    printf("\n");
  }
};