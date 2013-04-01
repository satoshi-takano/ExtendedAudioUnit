//
//  HTTPLoader.cpp
//  IKKitDevelop
//
//  Created by  on 4/12/12.
//  Copyright (c) 2012 アップルジャパン株式会社. All rights reserved.
//

#include <iostream>
#include "HTTPLoader.hpp"
#include "NetworkEvent.hpp"

HTTPLoader::~HTTPLoader()
{
  CFReadStreamUnscheduleFromRunLoop(mStream, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
  CFReadStreamClose(mStream);
  CFRelease(mStream);
}

void HTTPLoader::load(CFURLRef url)
{
  CFHTTPMessageRef request = CFHTTPMessageCreateRequest(NULL, CFSTR("GET"), url, kCFHTTPVersion1_1);
  mStream = CFReadStreamCreateForHTTPRequest(NULL, request);
  CFRelease(request);
  
  if (!CFReadStreamOpen(mStream)) {
    CFRelease(mStream);
    mStream = 0;
  } else {
    CFStreamClientContext context = {0, this, NULL, NULL, NULL};
    CFReadStreamSetClient(mStream, 
                          kCFStreamEventHasBytesAvailable |
                          kCFStreamEventErrorOccurred |
                          kCFStreamEventEndEncountered, 
                          readStreamCallback, 
                          &context);
    CFReadStreamScheduleWithRunLoop(mStream, 
                                    CFRunLoopGetCurrent(), 
                                    kCFRunLoopCommonModes);
  }
}

void HTTPLoader::readStreamCallback(CFReadStreamRef stream, CFStreamEventType type, void *userData)
{
  HTTPLoader* loader = static_cast<HTTPLoader*>(userData);
  
  CFTypeRef response = CFReadStreamCopyProperty(stream, 
                                                kCFStreamPropertyHTTPResponseHeader);
  if (loader->mFileBytes == 0) {
    CFHTTPMessageRef message = (CFHTTPMessageRef)CFReadStreamCopyProperty(stream, kCFStreamPropertyHTTPResponseHeader);
    CFDictionaryRef headerFields = CFHTTPMessageCopyAllHeaderFields(message);
    CFTypeRef totalByteSize = CFDictionaryGetValue(headerFields, CFSTR("Content-Length"));
    loader->mFileBytes = CFStringGetIntValue((CFStringRef)totalByteSize);
    CFRelease(message);
    CFRelease(headerFields);
  }
  
  CFIndex code = CFHTTPMessageGetResponseStatusCode((CFHTTPMessageRef)response);
  CFRelease(response);
  
  if (code != 200) {
    std::cout << "read stream error : code = " << code << std::endl;
    return;
  }
  
  switch (type) {
    case kCFStreamEventHasBytesAvailable: {
      UInt8 bytes[32768];
      CFIndex length = CFReadStreamRead(stream, bytes, 32768);
      if (length == -1) return;
      loader->mNumberOfReceivedByteSize += length;
      NetworkEvent e(kNetworkEventDataReceived);
      e.setReceivedByteSize(length);
      e.setReceivedData(bytes);
      e.setProgress((float)loader->mNumberOfReceivedByteSize / (float)loader->mFileBytes);
      loader->invoke(e);
      break;
    }
    case kCFStreamEventErrorOccurred: {
      std::cout << "kCFStreamEventErrorOccurred" << std::endl;
      NetworkEvent e(kNetworkEventErrorOccured);
      e.setErrorCode(code);
      loader->invoke(e);
      break;
    }
    case kCFStreamEventEndEncountered: {
      NetworkEvent e(kNetworkEventComplete);
      e.setProgress(1.0);
      loader->invoke(e);
    }
    default:
      break;
  }
}

const CFIndex HTTPLoader::getNumberOfReceivedByteSize() const
{
  return mNumberOfReceivedByteSize;
}

const CFIndex HTTPLoader::getFileBytes() const
{
  return mFileBytes;
}