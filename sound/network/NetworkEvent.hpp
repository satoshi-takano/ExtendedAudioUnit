//
//  NetworkEvent.h
//  IKKitDevelop
//
//  Created by  on 4/12/12.
//  Copyright (c) 2012 アップルジャパン株式会社. All rights reserved.
//

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