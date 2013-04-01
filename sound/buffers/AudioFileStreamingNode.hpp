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

#include "AudioSourceNode.hpp"
#include "HTTPLoader.hpp"
#include "NetworkEvent.hpp"
#include "OfflineConverter.hpp"
#include "OfflineConverterEvent.hpp"
#include "AudioFileDoubleBuffer.hpp"
#include "Asynchronous.hpp"

class AudioFileStreamingNode : public AudioSourceNode, public Asynchronous {
public:
    
    AudioFileStreamingNode(const AUGraph graph, const CFURLRef srcURLRef, const CFURLRef tmpLocalFileURL);
    ~AudioFileStreamingNode();
    
    void readBuffer(const AudioTimeStamp* timeStamp, UInt32 busNumber, UInt64 position, UInt32& numberFrames, Float32* data);
    
    void deleteAfterCanceling();
    
    static OSStatus render(void* inRefCon,
                           AudioUnitRenderActionFlags* ioActionFlags,
                           const  AudioTimeStamp* inTimeStamp,
                           UInt32 inBusNumber,
                           UInt32 inNumberFrames,
                           AudioBufferList* ioData);
    
private:
    
    static void propertyListenerProc(void* userData,
                                     AudioFileStreamID audioFileStream,
                                     AudioFileStreamPropertyID propertyID,
                                     UInt32* ioFlags);
    static void packetsProc(void* userData,
                            UInt32 numberBytes,
                            UInt32 numberPackets,
                            const void* inputData,
                            AudioStreamPacketDescription* packetDescriptions);
    
    void dataReceived(NetworkEvent* e);
    void dataLoaded(NetworkEvent* e);
    void networkErrorOccured(NetworkEvent* e);
    void didConvert(OfflineConverterEvent* e);
    
    HTTPLoader* mHTTPLoader;
    AudioFileStreamID mAudioFileStream;
    CFURLRef mLocalFileURL;
    AudioFileID mLocalAudioFile;
    AudioStreamBasicDescription mSrcFileFormat;
    bool mNeedFreeObject;
    
    OfflineConverter* mOfflineConverter;
    AudioFileDoubleBuffer* mDoubleBuffer;
};