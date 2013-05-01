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

void FFT::getPowerSpectrum(float *spectrum)
{
    vDSP_vdist(mSplitComplex.realp, 1, mSplitComplex.imagp, 1, spectrum, 1, mFFTSize);
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