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

#include <AudioToolbox/AudioToolbox.h>
#include "AudioSourceNode.hpp"
#include "Thread.hpp"
#include "EventSender.hpp"

class OfflineConverter : public Thread, public events::EventSender {
public:
    OfflineConverter(AudioStreamBasicDescription inFmt, AudioStreamBasicDescription clientFmt, AudioFileID destinationFile);
    ~OfflineConverter();
    
    void run();
    
    static void timerUpdate(CFRunLoopTimerRef timer, void* info);
    static OSStatus encoderProc(AudioConverterRef aconv,
                                UInt32* ioNumberDataPackets,
                                AudioBufferList* ioData,
                                AudioStreamPacketDescription** outDataPacketDescs,
                                void* userData);
    
    void convert(AudioFileStreamID afID, void* inputData, UInt32 inputDataByteSize, UInt32 numberPackets, AudioStreamPacketDescription* packetDescriptions);
    UInt64 getPreparedFrames() const;
    
    virtual void deleteAfterCanceling();
    
private:
    struct ConvertDataPool {
        AudioFileStreamID fileID;
        void* inputData;
        UInt32 numberPackets;
        UInt32 numberBytes;
        AudioStreamPacketDescription* ASPDs;
    };
    
    struct ConverterData {
        void* inputData;
        UInt32 numberBytes;
        UInt32 numberInPackets;
        UInt32 numberOutPackets;
        unsigned channels;
        AudioStreamPacketDescription* ASPDs;
        AudioStreamPacketDescription ASPD;
        int doneCount;
    };
    
    std::vector<ConvertDataPool*> mConverterPoolQ;
    std::vector<ConverterData*> mConverterQ;
    CFRunLoopRef mRunLoop;
    
    CFRunLoopTimerRef mTimer;
    pthread_mutex_t mMutex;
    
    AudioFileID mDestinationFile;
    AudioConverterRef mConverter;
    UInt64 mWriteStartingPacket;
    
    AudioStreamBasicDescription mClientASBD;
    bool mNeedCancel;
};