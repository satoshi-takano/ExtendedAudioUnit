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
#include "AudioFileStreamingNode.hpp"
#include "AUStreamBasicDescription.hpp"

// ここもあまり大きくし過ぎると、cue などで大きくジャンプした際 AudioFile 直読みの回数が増えてノイズになる
static const int EXT_BUF_SECONDS = 6;
static const unsigned long BUFFER_SAMPLES = 44100 * EXT_BUF_SECONDS * 2;

#pragma mark ---- constructor, destructor ----
AudioFileStreamingNode::AudioFileStreamingNode(const AUGraph graph, const CFURLRef urlRef, const CFURLRef tmpLocalFileURL) : AudioSourceNode(graph), mLocalAudioFile(0), mNeedFreeObject(false), mDoubleBuffer(0)
{
    AudioFileStreamOpen(this, propertyListenerProc, packetsProc, 0, &mAudioFileStream);
    
    mHTTPLoader = new HTTPLoader();
    mHTTPLoader->load(urlRef);
    mHTTPLoader->addReceiver(kNetworkEventDataReceived, this, &AudioFileStreamingNode::dataReceived);
    mHTTPLoader->addReceiver(kNetworkEventComplete, this, &AudioFileStreamingNode::dataLoaded);
    mHTTPLoader->addReceiver(kNetworkEventErrorOccured, this, &AudioFileStreamingNode::networkErrorOccured);
    mLocalFileURL = tmpLocalFileURL;
    CFRetain(mLocalFileURL);
}

AudioFileStreamingNode::~AudioFileStreamingNode()
{
    if (mNeedFreeObject) {
        mOfflineConverter->removeReceivers(OfflineConverterEventWriteToFile, this, &AudioFileStreamingNode::didConvert);
        mOfflineConverter->deleteAfterCanceling();
        delete mDoubleBuffer; mDoubleBuffer = 0;
        AudioFileClose(mLocalAudioFile);
    }
    AudioFileStreamClose(mAudioFileStream);
    CFRelease(mLocalFileURL);
    mHTTPLoader->removeReceivers(kNetworkEventDataReceived, this, &AudioFileStreamingNode::dataReceived);
    mHTTPLoader->removeReceivers(kNetworkEventComplete, this, &AudioFileStreamingNode::dataLoaded);
    mHTTPLoader->removeReceivers(kNetworkEventErrorOccured, this, &AudioFileStreamingNode::networkErrorOccured);
    delete mHTTPLoader;
}

void AudioFileStreamingNode::deleteAfterCanceling() {
    delete this;
}

#pragma mark ---- define callbacks ----
void AudioFileStreamingNode::dataReceived(NetworkEvent* e)
{
    checkError(AudioFileStreamParseBytes(mAudioFileStream, e->getReceivedByteSize(), e->getReceivedData(), 0), "AudioFileStreamParseBytes");
    //  invoke(*e);
}

void AudioFileStreamingNode::dataLoaded(NetworkEvent* e)
{
    invoke(*e);
}

void AudioFileStreamingNode::networkErrorOccured(NetworkEvent *e)
{
    invoke(*e);
}

void AudioFileStreamingNode::didConvert(OfflineConverterEvent *e)
{
    mTotalFrames += e->currentFrames;
    invoke(*e);
}

void AudioFileStreamingNode::propertyListenerProc(void* userData,
                                                  AudioFileStreamID audioFileStream,
                                                  AudioFileStreamPropertyID propertyID,
                                                  UInt32* ioFlags)
{
    if (propertyID == kAudioFileStreamProperty_ReadyToProducePackets) {
        AudioFileStreamingNode* node = (AudioFileStreamingNode*)userData;
        
        // get ASBD in the audio file.
        UInt32 size = sizeof(AudioStreamBasicDescription);
        checkError(AudioFileStreamGetProperty(audioFileStream, kAudioFileStreamProperty_DataFormat, &size, &node->mSrcFileFormat), "AudioFileStreamGetProperty");
        
        // get file type
        AudioFileTypeID type;
        size = sizeof(AudioFileTypeID);
        checkError(AudioFileStreamGetProperty(node->mAudioFileStream, kAudioFileStreamProperty_FileFormat, &size, &type),"AudioFileStreamGetProperty FileTypeID");
        
        // print filetype
        /*
        char property[5];
        *(UInt32 *)property = CFSwapInt32HostToBig(type);
        property[4] = '\0';
        printf("\n........ AudioFileType ........\n%-4.4s\n", property);
         */
        
        // create AudioConverter
        AUStreamBasicDescription caffmt;
        caffmt.mSampleRate = node->mSrcFileFormat.mSampleRate;
        caffmt.SetFloat(node->mSrcFileFormat.mChannelsPerFrame, true);
        
        // create temporaly audio file in local storage
        checkError(AudioFileCreateWithURL(node->mLocalFileURL, kAudioFileCAFType, &caffmt, kAudioFileFlags_EraseFile, &node->mLocalAudioFile), "Fail AudioFileCreateWithURL");
        
        AUStreamBasicDescription outFormat;
        outFormat.mSampleRate = node->mSrcFileFormat.mSampleRate;
        outFormat.SetFloat(2, false);
        node->configuration(0, 2);
        
        node->mOfflineConverter = new OfflineConverter(node->mSrcFileFormat, caffmt, node->mLocalAudioFile);
        node->mOfflineConverter->addReceiver(OfflineConverterEventWriteToFile, node, &AudioFileStreamingNode::didConvert);
        node->mOfflineConverter->start();
        
        node->mDoubleBuffer = new AudioFileDoubleBuffer(node->mLocalAudioFile, BUFFER_SAMPLES, node->mTotalFrames);
        
        node->mNeedFreeObject = true;
    }
}

void AudioFileStreamingNode::packetsProc(void* userData,
                                         UInt32 numberBytes,
                                         UInt32 numberPackets,
                                         const void* inputData,
                                         AudioStreamPacketDescription* packetDescriptions)
{
    AudioFileStreamingNode* node = (AudioFileStreamingNode*)userData;
    node->mOfflineConverter->convert(node->mAudioFileStream, const_cast<void*>(inputData), numberBytes, numberPackets, packetDescriptions);
}

#pragma mark ---- render audio samples ----
void AudioFileStreamingNode::readBuffer(const AudioTimeStamp *timeStamp, UInt32 busNumber, UInt64 position, UInt32& numberFrames, Float32 *data)
{
    if (mTotalFrames <= position + numberFrames) numberFrames = mTotalFrames - position;
    if (mDoubleBuffer) {
        mDoubleBuffer->read(position, numberFrames, data);
//        if (44100 < position + mOfflineConverter->getPreparedFrames())
//            mDoubleBuffer->read(position, numberFrames, data);
    }
}