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

#include "Event.hpp"
  
enum NetworkEventType {
  kNetworkEventDataReceived = 'nwdr',
  kNetworkEventErrorOccured = 'nweo',
  kNetworkEventComplete = 'nwcp'
};

class NetworkEvent : public events::Event {
public:
  NetworkEvent(NetworkEventType type) : Event(type), mReceivedByteSize(0), mErrorCode(0), mProgress(0) {}
  
  const size_t getReceivedByteSize() const {return mReceivedByteSize;}
  void setReceivedByteSize(size_t size) {mReceivedByteSize = size;}
  const unsigned char* getReceivedData() const {return mData;}
  void setReceivedData(unsigned char* bytes) {mData = bytes;}
  const char getErrorCode() const {return mErrorCode;}
  void setErrorCode(char code) {mErrorCode = code;}
  const float getProgress() const {return mProgress;}
  void setProgress(float progress) {mProgress = progress;}
private:
  size_t mReceivedByteSize;
  unsigned char* mData;
  char mErrorCode;
  float mProgress;
};