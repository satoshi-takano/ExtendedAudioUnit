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

#include <iostream>
#include <string.h>

static void checkErrorFunc(OSStatus err,const char *message, const char* file = "", unsigned int line = 0)
{
  if (err) {
    char property[5];
    *(UInt32 *)property = CFSwapInt32HostToBig(err);
    property[4] = '\0';
    printf("\n........ Error ........\n\
 File : %s\n\
 Line : %d\n\
 %s = %-4.4s, %d\n",file, line, message, property, (int)err);
    //exit(1);
  }
}

#define checkError(err, message) checkErrorFunc(err, message, __FILE__, __LINE__)
  


#define anodeErrorLog() printf("\
\n... AudioNodeError ........\n\
File : %s\n\
Line : %d\n", __FILE__, __LINE__)


class AudioNodeException {
public:
  AudioNodeException(const char* message) : mMessage(message){};
  const std::string& getMessage() const {
    return mMessage;
  }
private:
  std::string mMessage;
};