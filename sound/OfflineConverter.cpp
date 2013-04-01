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

#include "OfflineConverter.hpp"
#include "OfflineConverterEvent.hpp"

OfflineConverter::OfflineConverter(AudioStreamBasicDescription inFmt, AudioStreamBasicDescription clientFmt, AudioFileID destinationFile) : mWriteStartingPacket(0), mNeedCancel(false)
{
    checkError(AudioConverterNew(&inFmt, &clientFmt, &mConverter), "Fail AudioConverterNew");
    mDestinationFile = destinationFile;
    mClientASBD = clientFmt;
    
    CFRunLoopTimerContext timerCtx;
    bzero(&timerCtx, sizeof(timerCtx));
    timerCtx.info = this;
    mTimer = CFRunLoopTimerCreate(NULL, CFAbsoluteTimeGetCurrent(), 0.005, 0, 0, timerUpdate, &timerCtx);
    
    pthread_mutex_init(&mMutex, NULL);
}

OfflineConverter::~OfflineConverter()
{
    printf("delete OfflineConverter\n");
    pthread_mutex_destroy(&mMutex);
    AudioConverterDispose(mConverter);
    while (mConverterPoolQ.size()) {
        ConvertDataPool* cdp = mConverterPoolQ.front();
        free(cdp->inputData);
        delete cdp;
        mConverterPoolQ.erase(mConverterPoolQ.begin());
    }
    while (mConverterQ.size()) {
        ConverterData* cd = mConverterQ.front();
        free(cd->inputData);
        delete cd;
        mConverterQ.erase(mConverterQ.begin());
    }
}

void OfflineConverter::deleteAfterCanceling() {
    mNeedCancel = true;
}

void OfflineConverter::run()
{
    //  struct sched_param   param;
    //  param.sched_priority = sched_get_priority_min(SCHED_RR);
    //  pthread_setschedparam(getThreadRef(), SCHED_RR, &param);
    
    mRunLoop = CFRunLoopGetCurrent();
    
    CFRunLoopAddTimer(mRunLoop, mTimer, kCFRunLoopDefaultMode);
    
    CFRunLoopRun();
}

void OfflineConverter::timerUpdate(CFRunLoopTimerRef timer, void *info)
{
    OfflineConverter* _this = static_cast<OfflineConverter*>(info);
    if (_this->mNeedCancel) {
        CFRunLoopRemoveTimer(_this->mRunLoop, _this->mTimer, kCFRunLoopDefaultMode);
        CFRelease(_this->mTimer);
        CFRunLoopStop(_this->mRunLoop);
        delete _this;
        return;
    }
    if (_this->mConverterPoolQ.size() < 1) return;
    
    ConverterData* cd = new ConverterData();
    ConvertDataPool* pool = _this->mConverterPoolQ.front();
    AudioFramePacketTranslation translation;
    UInt32 size = sizeof(AudioFramePacketTranslation);
    translation.mPacket = 1;
    checkError(AudioFileStreamGetProperty(pool->fileID, kAudioFileStreamProperty_PacketToFrame, &size, &translation), "AudioFileGetProperty");
    UInt32 numberOutPackets = translation.mFrame;
    AudioBufferList* buflist = AudioSourceNode::createAudioBufferList(_this->mClientASBD.mChannelsPerFrame, true, numberOutPackets, sizeof(Float32));
    
    for (int i = 0, l = _this->mConverterPoolQ.size(); i < l; i++) {
        ConvertDataPool* pool = _this->mConverterPoolQ.front();
        
        if (_this->mNeedCancel) break;
        
        char* data = static_cast<char*>(pool->inputData);
        
        UInt32 numberPackets = pool->numberPackets;
        AudioStreamPacketDescription* packetDescriptions = pool->ASPDs;
        for (int i = 0; i < numberPackets; i++) {
            
            if (_this->mNeedCancel) break;
            
            UInt32 bytesPerPacket = packetDescriptions[i].mDataByteSize;
            UInt32 startOffset = packetDescriptions[i].mStartOffset;
            
            
            cd->inputData = &data[startOffset];
            cd->numberBytes = bytesPerPacket;
            cd->numberInPackets = 1;
            cd->numberOutPackets = numberOutPackets;
            cd->channels = _this->mClientASBD.mChannelsPerFrame;
            cd->ASPDs = &packetDescriptions[i];
            cd->ASPD.mDataByteSize = bytesPerPacket;
            cd->ASPD.mStartOffset = 0;
            cd->ASPD.mVariableFramesInPacket = packetDescriptions[i].mVariableFramesInPacket;
            
            OSStatus err;
            err = AudioConverterFillComplexBuffer(_this->mConverter, encoderProc, cd, &cd->numberOutPackets, buflist, NULL);
            
            if (err) {
                checkError(err, "AudioConverterFillComplexBuffer");
            }
            
            UInt32 numberBytes = buflist->mBuffers[0].mDataByteSize;
            
            // 複数回同じ箇所をreadする場合 chache は true にすべきらしい (write も ?)
            if (!_this->mNeedCancel)
                checkError(AudioFileWritePackets(_this->mDestinationFile, true, numberBytes, NULL, _this->mWriteStartingPacket, &cd->numberOutPackets, buflist->mBuffers[0].mData), "AudioFileWritePackets");
            
            _this->mWriteStartingPacket += cd->numberOutPackets;
            
            OfflineConverterEvent e(OfflineConverterEventWriteToFile);
            e.currentData = buflist->mBuffers[0].mData;
            e.currentFrames = cd->numberOutPackets;
            _this->invoke(e);
        }
        
        pthread_mutex_lock(&_this->mMutex);
        _this->mConverterPoolQ.erase(_this->mConverterPoolQ.begin());
        pthread_mutex_unlock(&_this->mMutex);
        
        free(pool->inputData);
        free(pool->ASPDs);
        delete pool;
    }
    AudioSourceNode::deleteAudioBufferList(buflist);
    delete cd;
}

OSStatus OfflineConverter::encoderProc(AudioConverterRef aconv,
                                       UInt32* ioNumberDataPackets,
                                       AudioBufferList* ioData,
                                       AudioStreamPacketDescription** outDataPacketDescs,
                                       void* userData)
{
    ConverterData* cd = static_cast<ConverterData*>(userData);
    
    ioData->mBuffers[0].mNumberChannels = cd->channels;
    ioData->mBuffers[0].mDataByteSize = cd->numberBytes;
    ioData->mBuffers[0].mData = cd->inputData;
    
    *ioNumberDataPackets = cd->numberInPackets;
    if (outDataPacketDescs) {
        if (&cd->ASPD) {
            *outDataPacketDescs = &cd->ASPD;
        }
        else *outDataPacketDescs = NULL;
    }
    
    return noErr;
}

UInt64 OfflineConverter::getPreparedFrames() const {
    return mWriteStartingPacket;
}

void OfflineConverter::convert(AudioFileStreamID afID, void* inputData, UInt32 inputDataByteSize, UInt32 numberPackets, AudioStreamPacketDescription* packetDescriptions)
{
    // パケットを一度に多数渡された場合、
    // Loop が多くなり Main Thread が長時間ブロックされる可能性があるので、
    // ここでは渡されたデータをそのままスタックにつっこんでおいて、
    // 別 Thread で分割、変換処理する.
    ConvertDataPool* pool = new ConvertDataPool();
    pool->fileID = afID;
    pool->inputData = malloc(inputDataByteSize);
    memcpy(pool->inputData, inputData, inputDataByteSize);
    pool->numberPackets = numberPackets;
    pool->ASPDs = (AudioStreamPacketDescription*)calloc(numberPackets, sizeof(AudioStreamPacketDescription));
    memcpy(pool->ASPDs, packetDescriptions, sizeof(AudioStreamPacketDescription) * numberPackets);
    pthread_mutex_lock(&mMutex);
    mConverterPoolQ.push_back(pool);
    pthread_mutex_unlock(&mMutex);
}