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
#include <pthread.h>
#include "WideBuffer.hpp"

class AudioFileDoubleBuffer {
    
public:
    AudioFileDoubleBuffer(AudioFileID audioFileID, UInt32 numFrames, UInt64& totalFrames) : mPreviousPosition(0), mPreImmediacy(false), mTotalFrames(totalFrames)
    {
        mNumFrontBufferSamples = numFrames;
        mAudioFileID = audioFileID;
        
        mFrontBuffer = new WideBuffer<Float32>(mNumFrontBufferSamples);
        mBackBuffer = new WideBuffer<Float32>(mNumFrontBufferSamples);
        
        mFileReadJob.done = true;
        
        CFRunLoopTimerContext timerCtx;
        bzero(&timerCtx, sizeof(timerCtx));
        timerCtx.info = this;
        //mTimer = CFRunLoopTimerCreate(NULL, CFAbsoluteTimeGetCurrent(), 0.1, 0, 0, timerUpdate, &timerCtx);
    }
    
    ~AudioFileDoubleBuffer() {
        delete mFrontBuffer;
        delete mBackBuffer;
    }
    
    
    static void* _run(void *pthis)
    {
        AudioFileDoubleBuffer* _this = static_cast<AudioFileDoubleBuffer*>(pthis);
        FileReadJob& job = _this->mFileReadJob;
        
        if (!job.done) {
            UInt32 numFrames = job.numFrames;
            // 前後に余分にサンプルを読み込んでおく (保留)
            unsigned long leftFrames = (unsigned int)floorf((float)numFrames * 0.5);
            UInt32 position = job.position;
            if (position < leftFrames) leftFrames = position;
            position -= leftFrames;
            
            UInt32 outNumBytes = sizeof(Float32) * 2 * numFrames;
            Float32* tmp = (Float32*)malloc(outNumBytes);
            checkError(AudioFileReadPacketData(_this->mAudioFileID, true, &outNumBytes, NULL, position, &numFrames, tmp),
                       "AudioFileReadPackets");
            
            job.buffer->set(position * 2, tmp, numFrames * 2);
            // バッファを入れ替える
            WideBuffer<Float32>* tmpbuf = _this->mFrontBuffer;
            _this->mFrontBuffer = job.buffer;
            _this->mBackBuffer = tmpbuf;
            
            free(tmp);
            job.done = true;
        }
        return 0;
    };
    
    void read(UInt64 position, UInt32& numberFrames, Float32* data)
    {
        bool checkBackward = true;
        UInt64 samplePosition = position * 2;
        
        UInt64 numFrontBufferFrames = mNumFrontBufferSamples * 0.5;
        if (position < numFrontBufferFrames) {
            checkBackward = false;
        }
        
        bool immediacy = false;
        if (position == 0 && (mPreImmediacy && !mFileReadJob.done)) {
            // position == 0 なら強制的にファイルから読み込む
            immediacy = true;
        } else {
            // cue で遠いフレームへジャンプされた場合強制的にファイルから読み込み
            UInt32 checkLength = numberFrames * 2;
            if (mPreviousPosition < position && mFrontBuffer->needBufferForward(samplePosition, checkLength)) {
                immediacy = true;
            } else if (position < mPreviousPosition){
                if (position < numberFrames) checkLength = position;
                if (mFrontBuffer->needBufferBackward(samplePosition, checkLength))
                    immediacy = true;
            }
        }
        // position の前後に3秒分のバッファが溜まっていなければバッファを取得する
        unsigned checkSampleSize = 44100 * 2;
        if (immediacy || mFrontBuffer->needBufferForward(samplePosition, checkSampleSize) ||
            (checkBackward && mFrontBuffer->needBufferBackward(samplePosition, checkSampleSize))) {
            
            bool needBuffering = true;
            if (mTotalFrames < position + checkSampleSize) {
                checkSampleSize = mTotalFrames - position;
                needBuffering = mFrontBuffer->needBufferForward(samplePosition, checkSampleSize);
            }
            if (mFileReadJob.done && needBuffering) {
                mFileReadJob.done = false;
                mFileReadJob.position = position;
                mFileReadJob.numFrames = numFrontBufferFrames;
                mFileReadJob.buffer = mBackBuffer;
                
                pthread_create(&mThread, NULL, _run, this);
                pthread_detach(mThread);
//                struct sched_param   param;
//                param.sched_priority = sched_get_priority_min(SCHED_FIFO);
//                pthread_setschedparam(mThread, SCHED_FIFO, &param);
            }
        }
        
        if (mPreImmediacy) {
            // 前回ファイルから直接読み込んでいて、かつまだバッファリング完了していなければ、
            // またファイルから直接読み込む.
            if (!mFileReadJob.done) immediacy = true;
            // バッファリング完了していたらフラグを戻す
            else immediacy = mPreImmediacy = false;
        }
        // ファイルから直接読み込んだかどうか記録しておく
        mPreImmediacy = immediacy;
        
        if (!immediacy) {
            mFrontBuffer->read(samplePosition, numberFrames * 2, data);
        } else {
            //printf("immediacy pos : %llu frm : %lu\n", position, numberFrames);
            UInt32 outNumBytes = sizeof(Float32) * numberFrames * 2;
            AudioFileReadPackets(mAudioFileID, true, &outNumBytes, NULL, position, &numberFrames, data);
            //outNumBytes = sizeof(Float32) * 2 * numberFrames;
            //bzero(data, sizeof(Float32) * numberFrames * 2);
            //numberFrames = 0;
        }
        mPreviousPosition = position;
    }
    
private:
    struct FileReadJob {
        bool done;
        UInt64 position;
        UInt32 numFrames;
        WideBuffer<Float32>* buffer;
    };
    
    unsigned long mNumFrontBufferSamples;
    UInt64& mTotalFrames;
    
    AudioFileID mAudioFileID;
    WideBuffer<Float32>* mFrontBuffer;
    WideBuffer<Float32>* mBackBuffer;
    FileReadJob mFileReadJob;
    
    UInt64 mPreviousPosition;
    bool mPreImmediacy;
    
    pthread_t mThread;
};