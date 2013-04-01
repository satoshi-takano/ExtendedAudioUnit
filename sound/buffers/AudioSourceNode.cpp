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
#include "AudioSourceNode.hpp"
#include <vector>
#include "AUStreamBasicDescription.hpp"
#include <Accelerate/Accelerate.h>

/////////////////////////////////////////////////////////////////////////////////////////////
/// @brief	"6-point, 5th-order optimal 32x z-form implementation" interpolator, see
///			Olli Niemitalo's "Elephant" paper
///			http://www.student.oulu.fi/~oniemita/dsp/deip.pdf
///
/////////////////////////////////////////////////////////////////////////////////////////////
inline static float wave_interpolator(float x, float y[6])
{
	const int offset = 2;
	
	float z = x - 1/2.0;
	float even1 = y[offset+1]+y[offset+0], odd1 = y[offset+1]-y[offset+0];
	float even2 = y[offset+2]+y[offset+-1], odd2 = y[offset+2]-y[offset+-1];
	float even3 = y[offset+3]+y[offset+-2], odd3 = y[offset+3]-y[offset+-2];
	
	float c0 = even1*0.42685983409379380 + even2*0.07238123511170030
	+ even3*0.00075893079450573;
	float c1 = odd1*0.35831772348893259 + odd2*0.20451644554758297
	+ odd3*0.00562658797241955;
	float c2 = even1*-0.217009177221292431 + even2*0.20051376594086157
	+ even3*0.01649541128040211;
	float c3 = odd1*-0.25112715343740988 + odd2*0.04223025992200458
	+ odd3*0.02488727472995134;
	float c4 = even1*0.04166946673533273 + even2*-0.06250420114356986
	+ even3*0.02083473440841799;
	float c5 = odd1*0.08349799235675044 + odd2*-0.04174912841630993
	+ odd3*0.00834987866042734;
	
	return ((((c5*z+c4)*z+c3)*z+c2)*z+c1)*z+c0;
}

const AudioComponentDescription componentDesc = {
    kAudioUnitType_FormatConverter,
    kAudioUnitSubType_AUConverter,
    kAudioUnitManufacturer_Apple,
    0, 0};
AudioSourceNode::AudioSourceNode(const AUGraph graph) : AudioUnitNode(graph, componentDesc), mTotalFrames(0), mNumberOfSourceChannels(0), mNumberOfCuePoints(3), mRunning(false),
mLatestTime(0), mScratchMode(false), mPosition(0), mPreviousPosition(0), mScratchingPosition(0.0),
mScratchingPositionVelocity(44100.0), mScratchingPositionSmoothVelocity(44100.0), mSmoothBufferPosition(0), mInputData(0),
mNeedLoop(false), loopInFrame(0), loopOutFrame(0), mInputDataSize(44100 * 3)
{
    for (int i = 0; i < mNumberOfCuePoints; i++) {
        mCuePoints[i] = 0;
    }
    
    AURenderCallbackStruct callbackStruct;
    callbackStruct.inputProc = getRenderer();
    callbackStruct.inputProcRefCon = this;
    checkError(AUGraphSetNodeInputCallback(getAUGraph(), getAUNode(), 0, &callbackStruct), "AUGraphSetNodeInputCallback");
    
    memset(mSmoothBuffer, 0, sizeof(mSmoothBuffer));
    mLInterpData = (Float32*)calloc(12, sizeof(Float32));
    mRInterpData = (Float32*)calloc(12, sizeof(Float32));
}

AudioSourceNode::~AudioSourceNode()
{
    if (mInputData) free(mInputData);
    free(mLInterpData);
    free(mRInterpData);
}

void AudioSourceNode::setFormat(const AudioStreamBasicDescription &asbd, AudioUnitScope scope, UInt32 bus)
{
    if (scope != kAudioUnitScope_Output) {
        // Float32, interleaved ã§ãªã„ã¨ã„ã‚ã„ã‚ä¸ä¾¿ãªã®ã§ä¾‹å¤–æŠ•ã’ã¡ã‚ƒã†
        anodeErrorLog();
        AudioNodeException exeption("Can't to set ASBD to AudioSourceNode except output scopes");
        throw exeption;
    } else {
        AudioUnitNode::setFormat(asbd, scope, bus);
    }
}

void AudioSourceNode::input(AudioNode &node, UInt32 srcOutputCh, UInt32 inputCh)
{
    anodeErrorLog();
    AudioNodeException exeption("AudioSourceNode has not input channel");
    throw exeption;
}

void AudioSourceNode::configuration(UInt64 totalFrames, UInt32 numChannels)
{
    mTotalFrames = totalFrames;
    mNumberOfSourceChannels = numChannels;
    
    mInputData = (Float32*)calloc(numChannels * mInputDataSize, sizeof(Float32));
    
    AUStreamBasicDescription inFmt;
    inFmt.mSampleRate = 44100.0;
    inFmt.SetFloat(numChannels, true);
    
    AudioUnit unit;
    checkError(AUGraphNodeInfo(getAUGraph(), getAUNode(), NULL, &unit), "AUGraphNodeInfo AudioFileMemoryBufferNode");
    checkError(AudioUnitSetProperty(unit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &inFmt, sizeof(inFmt)), "AudioUnitSetProperty Converter StreamFormat in");
}

void AudioSourceNode::startRendering()
{
    mRunning = true;
}

void AudioSourceNode::stopRendering()
{
    mRunning = false;
}

const bool AudioSourceNode::isRunning() const
{
    return mRunning;
}

void AudioSourceNode::setCuePoint(float frame, UInt32 index)
{
    mCuePoints[index] = frame;
}

void AudioSourceNode::cue(UInt32 cuePointIndex)
{
    mPosition = mCuePoints[cuePointIndex];
    mPreviousPosition = mPosition;
    mScratchingPosition = mPosition;
    mLatestTime = CFAbsoluteTimeGetCurrent();
    mPreviousPosition=NAN;
}

void AudioSourceNode::setLoopInFrame(UInt64 frame)
{
    loopInFrame = frame;
}

void AudioSourceNode::setLoopOutFrame(UInt64 frame)
{
    loopOutFrame = frame;
}

void AudioSourceNode::enableLoop(bool enable)
{
    mNeedLoop = enable;
}

const AURenderCallback AudioSourceNode::getRenderer() const
{
    return render;
}

void AudioSourceNode::startScratching()
{
    mScratchMode = true;
    mPreviousPosition=NAN;
    mScratchingPosition = mPosition;
}

void AudioSourceNode::stopScratching()
{
    mScratchMode = false;
    mScratchingPositionVelocity = 44100.0;
}

void AudioSourceNode::setFrameOffset(float offset)
{
    mPosition = offset;
    if (mPosition < 0.0)
        mPosition = 0.0;
    else if (mTotalFrames < mPosition)
        mPosition = mTotalFrames - 1;
}

AudioBufferList* AudioSourceNode::createAudioBufferList(unsigned int ch, bool interleaved, UInt32 frames, UInt32 sampleByteSize)
{
    AudioBufferList* bufList = (AudioBufferList*)calloc(1, sizeof(AudioBufferList) + (interleaved ? 1 : ch) * sizeof(AudioBuffer));
    bufList->mNumberBuffers = interleaved ? 1 : ch;
    for (int i = 0; i < (interleaved ? 1 : ch); i++) {
        bufList->mBuffers[i].mNumberChannels = (interleaved ? ch : 1);
        unsigned size = frames * ((ch == 2 && interleaved) ? 2 : 1);
        bufList->mBuffers[i].mDataByteSize = sampleByteSize * size;
        bufList->mBuffers[i].mData = calloc(size, sampleByteSize);
    }
    return bufList;
}

void AudioSourceNode::deleteAudioBufferList(AudioBufferList *bufList)
{
    for (int i = 0; i < bufList->mNumberBuffers; i++) {
        free(bufList->mBuffers[i].mData);
    }
    free(bufList);
}

float sinc(float x)
{
    float y;
    
    if (x == 0.0f) {
        y = 1.0f;
    }
    else {
        y = sin(x) / x;
    }
    return y;
}


OSStatus AudioSourceNode::render(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags, const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList *ioData)
{
    AudioSourceNode* node = (AudioSourceNode*)inRefCon;
    UInt64 totalFrames = node->mTotalFrames;
    
    if (!node->mRunning || (totalFrames < inNumberFrames)) {
        zeroBuffer(ioData, inNumberFrames, node->mNumberOfSourceChannels, sizeof(Float32));
        return noErr;
    };
    
    if (node->mScratchMode) {
        float position = node->mPosition;
        float& scratchingPosition = node->mScratchingPosition;
        float& scratchingPositionVelocity = node->mScratchingPositionVelocity;
        float& scratchingPositionSmoothVelocity = node->mScratchingPositionSmoothVelocity;
        float* smoothBuffer = node->mSmoothBuffer;
        int& smoothBufferPosition = node->mSmoothBufferPosition;
        
        float& previousPosition = node->mPreviousPosition;
        double& latestTime = node->mLatestTime;
        
        Float32* inputBuf = node->mInputData;
        Float32* leftInterpData = node->mLInterpData;
        Float32* rightInterpData = node->mRInterpData;
        
        double time = CFAbsoluteTimeGetCurrent();
        if (node->mScratchMode == false) {
            scratchingPositionVelocity = 44100.0;
        } else {
            if (isnan(previousPosition)) scratchingPositionVelocity = 0.0f;
            else {
                scratchingPositionVelocity = (position - previousPosition) / (time - latestTime);
            }
            previousPosition = position;
        }
        latestTime = time;
        
        Float32* dest = (Float32*)ioData->mBuffers[0].mData;
        //float leftInterpData[6], rightInterpData[6];
        
        // 細かい最適化
        float v1_3000 = 1.0 / 3000.0;
        float v44100x05 = 1.0 / 44100.0 * 0.5;
        int min = 0;
        int max = 0;
        
        // for のなかで readBuffer すると負荷高いので、
        // vector に必要なパラメータを保持しといて readBuffer を１回に抑える
        int indexes[inNumberFrames];
        Float32 reminders[inNumberFrames];
        for (int i = 0; i < inNumberFrames; i++) {
            // scratch ã®é€Ÿã•ã‚’ã€€ç§»å‹•å¹³å‡
            scratchingPositionSmoothVelocity -= smoothBuffer[smoothBufferPosition];
            smoothBuffer[smoothBufferPosition] = scratchingPositionVelocity * v1_3000;
            scratchingPositionSmoothVelocity += smoothBuffer[smoothBufferPosition];
            smoothBufferPosition = (++smoothBufferPosition) % 3000;
            
            float velocity = scratchingPositionSmoothVelocity;
            float posDiff = position - scratchingPosition;
            velocity += posDiff * 20.0;
            scratchingPosition += velocity * v44100x05;
            
            // ã“ã“ç„¡ç†ã‚„ã‚Š...
            if (totalFrames <= scratchingPosition) {
                node->stopScratching();
                zeroBuffer(ioData, inNumberFrames, node->mNumberOfSourceChannels, sizeof(Float32));
                events::Event e(kAudioSourceNodeEventArrivalLastFrame);
                node->invoke(e);
                return noErr;
            }
            
            if (scratchingPosition < 2.0) {
                scratchingPosition = 2.0;
            } else if (totalFrames - 12 < scratchingPosition) {
                scratchingPosition = totalFrames - 12;
            }
            
            float fBufPosition = scratchingPosition;
            int iBufPosition = (int)fBufPosition;
            float rem = fBufPosition - iBufPosition;
            
            int ibufminus2 = iBufPosition - 2;
            
            if (i == 0) min = ibufminus2;
            if (ibufminus2 < min) min = ibufminus2;
            if (max < ibufminus2) max = ibufminus2;
            
            indexes[i] = ibufminus2;
            reminders[i] = rem;
        }
        UInt32 numRead = max - min + 12;
//        printf("%d %d %lu\n", max, min, numRead);
        if (node->mInputDataSize <= numRead) return noErr;
        
        node->readBuffer(inTimeStamp, inBusNumber, min, numRead, inputBuf);
        
        for (int i = 0; i < inNumberFrames; i++) {
            int index = indexes[i] - min;
            Float32* buf = &inputBuf[index * 2];
            int cnt = 0;
            for (int j = 0; j < 12; j+=2) {
                leftInterpData[cnt] = *buf++;
                rightInterpData[cnt] = *buf++;
                cnt++;
            }
            float rem = reminders[i];
            *dest++ = wave_interpolator(rem, leftInterpData);
            *dest++ = wave_interpolator(rem, rightInterpData);
        }
        
        if (node->mNeedLoop) {
            float& posref = node->mPosition;
            if (position < node->loopInFrame) {
                posref = node->loopOutFrame;
                scratchingPosition = posref;
            }
            else if (node->loopOutFrame < position) {
                posref = node->loopInFrame;
                scratchingPosition = posref;
            }
        }
        
    } else {
        Float32* data = (Float32*)ioData->mBuffers[0].mData;
        float pos = node->mPosition;
        float lin = node->loopInFrame;
        float lout = node->loopOutFrame;
        if (totalFrames < pos + inNumberFrames) inNumberFrames = totalFrames - pos;
        if (node->mNeedLoop) {
            int diff = lout - lin;
            if (pos < lin) {
                int ofst = lin - pos;
                if (diff < ofst) ofst = diff;
                pos = lout - ofst;
            }
            else if (lout < pos) {
                int ofst = pos - lout;
                if (diff < ofst) ofst = diff;
                pos = lin + ofst;
            }
            if (inNumberFrames) {
                node->readBuffer(inTimeStamp, 0, (UInt64)pos, inNumberFrames, data);
                node->mPosition = pos + (float)inNumberFrames;
            } else {
                zeroBuffer(ioData, inNumberFrames, node->mNumberOfSourceChannels, sizeof(Float32));
                events::Event e(kAudioSourceNodeEventArrivalLastFrame);
                node->invoke(e);
            }
        }
        else {
            if (inNumberFrames) {
                node->readBuffer(inTimeStamp, 0, (UInt64)pos, inNumberFrames, data);
                node->mPosition += inNumberFrames;
            } else {
                zeroBuffer(ioData, inNumberFrames, node->mNumberOfSourceChannels, sizeof(Float32));
                events::Event e(kAudioSourceNodeEventArrivalLastFrame);
                node->invoke(e);
            }
        }
    }
    
    return noErr;
}