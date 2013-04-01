//
//  HTTPLoader.h
//  IKKitDevelop
//
//  Created by  on 4/11/12.
//  Copyright (c) 2012 アップルジャパン株式会社. All rights reserved.
//

#pragma once

#include <CFNetwork/CFNetwork.h>
#include "EventSender.hpp"

class HTTPLoader : public events::EventSender {
public:
  HTTPLoader() : mNumberOfReceivedByteSize(0), mFileBytes(0) {
  };
  ~HTTPLoader();
  
  void load(CFURLRef url);
  
  const CFIndex getNumberOfReceivedByteSize() const;
  const CFIndex getFileBytes() const;
  
private:
  static void readStreamCallback(CFReadStreamRef stream,
                                 CFStreamEventType type,
                                 void *userData);
  
  CFReadStreamRef mStream;
  CFIndex mNumberOfReceivedByteSize;
  CFIndex mFileBytes;
};