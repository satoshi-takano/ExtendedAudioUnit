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
#include "AudioAnalyzeNode.hpp"

AudioAnalyzeNode::AudioAnalyzeNode(AUGraph graph, UInt32 fftSize) : AudioSampleProcessingNode(graph)
{
  prepareFFT(fftSize);
}

AudioAnalyzeNode::~AudioAnalyzeNode()
{
  dispose();
}

void AudioAnalyzeNode::prepareFFT(UInt32 fftSize)
{
  mFFTSizeLog = (vDSP_Length)log2f(fftSize);
  mFFTSetup = vDSP_create_fftsetup(mFFTSizeLog, FFT_RADIX2);
  mFFTSize = 1 << mFFTSizeLog;
  mSplitComplex.realp = (float*)calloc(mFFTSize, sizeof(float));
  mSplitComplex.imagp = (float*)calloc(mFFTSize, sizeof(float));
  
  mWindow = (float*)calloc(mFFTSize, sizeof(float));
  mWindowedInput = (float*)calloc(mFFTSize, sizeof(float));
  
  vDSP_hann_window(mWindow, mFFTSize, 0);
}

void AudioAnalyzeNode::dispose()
{
  vDSP_destroy_fftsetup(mFFTSetup);
  free(mSplitComplex.realp);
  free(mSplitComplex.imagp);
  free(mWindow);
  free(mWindowedInput);
}

const AudioUnitParameterInfo AudioAnalyzeNode::getParameterInfo(AudioUnitParameterID pid, AudioUnitScope scope, AudioUnitElement bus) const {
  AudioUnitParameterInfo info;
  return info;
};

const AudioUnitParameterValue AudioAnalyzeNode::getParameter(AudioUnitParameterID pid, AudioUnitScope scope, AudioUnitElement bus) const
{
  return 0;
}

void AudioAnalyzeNode::setParameter(AudioUnitParameterID pid, AudioUnitParameterValue value, AudioUnitScope scope, AudioUnitElement bus)
{
}

const AURenderCallback AudioAnalyzeNode::getRenderer() const
{
  return render;
}

const vDSP_Length AudioAnalyzeNode::getFFTSize() const
{
  return mFFTSize;
}

const float* const AudioAnalyzeNode::getReal() const
{
  return mSplitComplex.realp;
}

const float* const AudioAnalyzeNode::getImag() const
{
  return mSplitComplex.imagp;
}

OSStatus AudioAnalyzeNode::render(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData)
{
  if (*ioActionFlags == kAudioUnitRenderAction_PostRender) {
    AudioAnalyzeNode* node = (AudioAnalyzeNode*)inRefCon;
    if (node->doBypass()) return noErr;
    
    vDSP_Length fftSize = node->mFFTSize;
    
    if (fftSize != inNumberFrames) {
      node->dispose();
      node->prepareFFT(inNumberFrames);
    }
    
    FFTSetup setUp = node->mFFTSetup;
    float* window = node->mWindow;
    float* windowInput = node->mWindowedInput;
    float* realp = node->mSplitComplex.realp;
    float* imagp = node->mSplitComplex.imagp;
    
    //int numBuffers = ioData->mNumberBuffers;
    UInt32 size = inNumberFrames * sizeof(Float32);
    
    memset(imagp, 0, size);
    
    for (int i = 0; i < 1; i++) {
      Float32* samples = (Float32*)ioData->mBuffers[i].mData;
      memcpy(realp, samples, size);
      vDSP_vmul(realp, 1, window, 1, windowInput, 1, inNumberFrames);
      vDSP_fft_zip(setUp, &node->mSplitComplex, 1, node->mFFTSizeLog, FFT_FORWARD);
      
      /*
       // inverse
       fft_zip(setUp, &node->mSplitComplex, 1, node->mFFTSizeLog, FFT_INVERSE);
       float scale = 1.0 / inNumberFrames;
       vsmul(realp, 1, &scale, realp, 1, inNumberFrames);
       vsmul(imagp, 1, &scale, imagp, 1, inNumberFrames);
       memcpy(samples, realp, size);
       */
    }
  }
  return noErr;
}