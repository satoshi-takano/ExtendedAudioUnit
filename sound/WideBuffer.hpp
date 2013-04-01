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

template <class _T>
class WideBuffer {
public:
  WideBuffer(long numSamples) : mTotalSamples(numSamples), mCenterPosition(0), mCenter(0), mLeftWingSize(0), mRightWingSize(0) {
    mSampleSize = sizeof(_T);
    mMem = (_T*)calloc(numSamples, mSampleSize);
    int radius = (int)floorf(numSamples * 0.5f);
    mCenter = radius + 1;
  };
  
  ~WideBuffer() {
    free(mMem);
  };
  
  void set(long position, _T* data, long numSamples) {
    long left;
    bool isOdd;
    if (numSamples % 2) {
      left = (int)floorf(numSamples * 0.5f);
      isOdd = true;
    } else {
      left = numSamples * 0.5f;
      isOdd = false;
    }
    mCenterPosition = position + left;
    mLeftWingSize = left;
    mRightWingSize = isOdd ? left + 1 : left;
    memcpy(&mMem[mCenter - left], data, numSamples * mSampleSize);
  }
  
  int read(long position, long numSamples, _T* data) {
    if (needBufferForward(position, numSamples)) return 0;
    int distance = mCenterPosition - position;
    long long idx = mCenter - distance;
    if (idx < 0) {
      //printf("crash\n");
      idx = 0;
    }
    memcpy(data, &mMem[idx], numSamples * mSampleSize);
    return 1;
  }
  
  bool needBufferForward(long position, long size) {
    long distance = position - mCenterPosition;
    return (mRightWingSize < (distance + size));
  }
  
  bool needBufferBackward(long position, long size) {
    long distance = mCenterPosition - position;
    return (-distance - size < -mLeftWingSize);
  }
  
  long getCenterPosition() const {
    return mCenterPosition;
  }
  
private:
  _T* mMem;
  long mLeftWingSize;
  long mRightWingSize;
  unsigned long mTotalSamples;
  size_t mSampleSize;
  
  long mCenter;
  long mCenterPosition;
};
