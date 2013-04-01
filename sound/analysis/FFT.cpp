//
//  FFT.cpp
//  IKKitDevelop
//
//  Created by  on 4/8/12.
//  Copyright (c) 2012 アップルジャパン株式会社. All rights reserved.
//

#include <iostream>
#include "FFT.hpp"

FFT::FFT(int fftSize)
{
  prepareFFT(fftSize);
}

FFT::~FFT()
{
  vDSP_destroy_fftsetup(mFFTSetup);
  free(mSplitComplex.realp);
  free(mSplitComplex.imagp);
  free(mWindow);
}

void FFT::prepareFFT(int fftSize)
{
  mFFTSize = fftSize;
  mFFTSizeLog = (vDSP_Length)log2f(fftSize);
  mFFTSetup = vDSP_create_fftsetup(mFFTSizeLog, FFT_RADIX2);
  mSplitComplex.realp = (float*)calloc(mFFTSize, sizeof(float));
  mSplitComplex.imagp = (float*)calloc(mFFTSize, sizeof(float));
  mWindow = (float*)calloc(mFFTSize, sizeof(float));
  
  vDSP_hann_window(mWindow, mFFTSize, 0);
}

void FFT::transform(float *normSignals, float *powerSpectra)
{
  size_t bytesize = mFFTSize * sizeof(float);
  memset(mSplitComplex.imagp, 0, bytesize);
  memcpy(mSplitComplex.realp, normSignals, bytesize);
  vDSP_vmul(mSplitComplex.realp, 1, mWindow, 1, mSplitComplex.realp, 1, mFFTSize);
  vDSP_fft_zip(mFFTSetup, &mSplitComplex, 1, mFFTSizeLog, FFT_FORWARD);
  vDSP_vdist(mSplitComplex.realp, 1, mSplitComplex.imagp, 1, powerSpectra, 1, mFFTSize);
}

void FFT::transform(float *normSignals, float *real, float *imag)
{
  size_t bytesize = mFFTSize * sizeof(float);
  memset(mSplitComplex.imagp, 0, bytesize);
  memcpy(mSplitComplex.realp, normSignals, bytesize);
  //vDSP_vmul(mSplitComplex.realp, 1, mWindow, 1, mSplitComplex.realp, 1, mFFTSize);
  vDSP_fft_zip(mFFTSetup, &mSplitComplex, 1, mFFTSizeLog, FFT_FORWARD);
  memcpy(real, mSplitComplex.realp, bytesize);
  memcpy(imag, mSplitComplex.imagp, bytesize);
}

void FFT::inverseTransform(float *real, float *imag, float *samples)
{
  size_t bytesize = mFFTSize * sizeof(float);
  memcpy(mSplitComplex.realp, real, bytesize);
  memcpy(mSplitComplex.imagp, imag, bytesize);

  vDSP_fft_zip(mFFTSetup, &mSplitComplex, 1, mFFTSizeLog, FFT_INVERSE);
  float scale = 1.0 / mFFTSize;
  vDSP_vsmul(mSplitComplex.realp, 1, &scale, mSplitComplex.realp, 1, mFFTSize);
  vDSP_vsmul(mSplitComplex.imagp, 1, &scale, mSplitComplex.imagp, 1, mFFTSize);
  memcpy(samples, mSplitComplex.realp, bytesize);
  //vDSP_vmul(samples, 1, mWindow, 1, samples, 1, mFFTSize);
}