//
//  FFT.h
//  IKKitDevelop
//
//  Created by  on 4/8/12.
//  Copyright (c) 2012 アップルジャパン株式会社. All rights reserved.
//

#pragma once

#include <Accelerate/Accelerate.h>

class FFT {
public:
  FFT(int fftSize);
  ~FFT();
  
  void transform(float* normSignals, float* powerSpectra);
  void transform(float* normSignals, float* real, float* imag);
  void inverseTransform(float* real, float* imag, float* samples);
  
private:
  void prepareFFT(int fftSize);
  
  DSPSplitComplex mSplitComplex;
  FFTSetup mFFTSetup;
  vDSP_Length mFFTSizeLog;
  float* mWindow;
  vDSP_Length mFFTSize;
};