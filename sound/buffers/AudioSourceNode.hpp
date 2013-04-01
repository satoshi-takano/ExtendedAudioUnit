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

#include "AudioUnitNode.hpp"
#include "EventSender.hpp"

enum AudioSourceNodeEvent {
    kAudioSourceNodeEventArrivalLastFrame = 'salf',
};

class AudioSourceNode : public AudioUnitNode, public events::EventSender {
public:
    static AudioBufferList* createAudioBufferList(unsigned ch, bool interleaved, UInt32 frames, UInt32 sampleByteSize);
    static void deleteAudioBufferList(AudioBufferList* bufList);
    
    AudioSourceNode(const AUGraph graph);
    virtual ~AudioSourceNode();
    
    virtual void setFormat(const AudioStreamBasicDescription& asbd, AudioUnitScope scope, UInt32 bus);
    
    void input(AudioNode& node, UInt32 srcOutputCh, UInt32 inputCh);
    // Does nothing.
    void disconnectInput(UInt32 inputCh) {};
    
    void startRendering();
    void stopRendering();
    const bool isRunning() const;
    
    void setCuePoint(float frame, UInt32 index);
    const float* getCuePoints() const
    {
        return mCuePoints;
    }
    void cue(UInt32 cuePointIndex);
    
    void setLoopInFrame(UInt64 frame);
    void setLoopOutFrame(UInt64 frame);
    void enableLoop(bool enable);
    
    void startScratching();
    void stopScratching();
    
    virtual void readBuffer(const AudioTimeStamp* timeStamp, UInt32 busNumber, UInt64 position, UInt32& numberFrames, Float32* data) = 0;
    
    const UInt32 numberOfInputs() const
    {
        return 0;
    };
    
    const SInt64 getTotalFrames() const {return mTotalFrames;}
    const float getFrameOffset() const {return mPosition;}// 誤差調整
    float* getFrameOffsetPtr() {return &mPosition;};
    void setFrameOffset(float offset);
    
    const UInt32 getNumberOfQuePoints() const {return mNumberOfCuePoints;}
    
    virtual const AURenderCallback getRenderer() const;
    
    UInt64 loopInFrame;
    UInt64 loopOutFrame;
    
protected:
    void configuration(UInt64 totalFrames, UInt32 numChannels);
    const UInt32 getNumberOfSourceChannels() const {return mNumberOfSourceChannels;}
    
private:
    static OSStatus render(void* inRefCon,
                           AudioUnitRenderActionFlags* ioActionFlags,
                           const  AudioTimeStamp* inTimeStamp,
                           UInt32 inBusNumber,
                           UInt32 inNumberFrames,
                           AudioBufferList* ioData);
    
    UInt32 mNumberOfSourceChannels;
    UInt32 mNumberOfCuePoints;
    
    float mCuePoints[3];
    bool mNeedLoop;
    
    bool mRunning;
    
    // scratching
    double mLatestTime;
    bool mScratchMode;
    float mPosition;
    float mPreviousPosition;
    float mScratchingPosition;
    float mScratchingPositionVelocity;
    float mScratchingPositionSmoothVelocity;
    int mSmoothBufferPosition;
    float mSmoothBuffer[3000];
    
    Float32* mInputData;
    UInt32 mInputDataSize;
    Float32* mLInterpData;
    Float32* mRInterpData;
protected:
    UInt64 mTotalFrames;
};
