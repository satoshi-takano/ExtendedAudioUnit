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