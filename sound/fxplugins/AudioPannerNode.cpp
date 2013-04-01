//
//  AudioPannerNode.cpp
//  CloudDJ
//
//  Created by Satoshi Takano on 1/23/13.
//
//

#include "AudioPannerNode.hpp"
#include <Accelerate/Accelerate.h>

const AURenderCallback AudioPannerNode::getRenderer() const
{
    return render;
}

OSStatus AudioPannerNode::render(void* inRefCon,
                                     AudioUnitRenderActionFlags* ioActionFlags,
                                     const  AudioTimeStamp* inTimeStamp,
                                     UInt32 inBusNumber,
                                     UInt32 inNumberFrames,
                                     AudioBufferList* ioData)
{
    if (*ioActionFlags == kAudioUnitRenderAction_PostRender) {
        AudioPannerNode* node = (AudioPannerNode*)inRefCon;
        AudioUnitParameterValue pan = node->mPan;
        if (node->doBypass() || pan == 0) return noErr;
        
        Float32* L = (Float32*)ioData->mBuffers[0].mData;
        Float32* R = (Float32*)ioData->mBuffers[1].mData;
        
        if (pan < 0) {
            float rF = -pan * 0.5f;
            vDSP_vsmul(R, 1, &rF, R, 1, inNumberFrames);
            float lF = 1.0f - rF;
            vDSP_vsmul(L, 1, &lF, L, 1, inNumberFrames);
            vDSP_vadd(L, 1, R, 1, L, 1, inNumberFrames);
            
            rF = (1.0f + pan) / rF;
            vDSP_vsmul(R, 1, &rF, R, 1, inNumberFrames);
        } else {
            float lF = pan * 0.5f;
            vDSP_vsmul(L, 1, &lF, L, 1, inNumberFrames);
            float rF = 1.0f - lF;
            vDSP_vsmul(R, 1, &rF, R, 1, inNumberFrames);
            vDSP_vadd(L, 1, R, 1, R, 1, inNumberFrames);

            lF = (1.0f - pan) / rF;
            vDSP_vsmul(L, 1, &lF, L, 1, inNumberFrames);
        }
    }
    
    return noErr;
}