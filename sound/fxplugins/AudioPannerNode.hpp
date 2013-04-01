//
//  AudioPannerNode.h
//  CloudDJ
//
//  Created by Satoshi Takano on 1/23/13.
//
//

#pragma once

#include "AudioSampleProcessingNode.hpp"

enum {
    kPannerNodeParam_Pan = 0, // -1 ~ 1
};

class AudioPannerNode : public AudioSampleProcessingNode {
    
public:
    AudioPannerNode(AUGraph graph) : AudioSampleProcessingNode (graph), mPan(0) {
        mPanInfo.cfNameString = CFSTR("pan");
        mPanInfo.minValue = -1.0;
        mPanInfo.maxValue = 1.0;
        mPanInfo.defaultValue = mPan;
        mPanInfo.unit = kAudioUnitParameterUnit_Pan;
    };
    ~AudioPannerNode () {};
    
    const AudioUnitParameterInfo getParameterInfo(AudioUnitParameterID pid, AudioUnitScope scope, AudioUnitElement bus) const {
        return mPanInfo;
    };
    const AudioUnitParameterValue getParameter(AudioUnitParameterID pid, AudioUnitScope scope, AudioUnitElement bus) const {return 0;};
    void setParameter(AudioUnitParameterID pid, AudioUnitParameterValue value, AudioUnitScope scope, AudioUnitElement bus) {
        mPan = value;
    };
    
    const UInt32 numberOfInputs() const {return 1;}
    const UInt32 numberOfOutputs() const {return 1;}
    const UInt32 numberOfParameters(UInt32 scope) const {return 0;}
    
    const AURenderCallback getRenderer() const;
    
private:
    float mPan;
    AudioUnitParameterInfo mPanInfo;
    
    static OSStatus render(void* inRefCon,
                           AudioUnitRenderActionFlags* ioActionFlags,
                           const  AudioTimeStamp* inTimeStamp,
                           UInt32 inBusNumber,
                           UInt32 inNumberFrames,
                           AudioBufferList* ioData);
};