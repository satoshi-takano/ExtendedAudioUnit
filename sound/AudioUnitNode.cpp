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
#include "AudioUnitNode.hpp"

AudioUnitNode::AudioUnitNode(AUGraph graph, const AudioComponentDescription& cd) : mGraph(graph)
{
    OSStatus err;
    err = AUGraphAddNode(graph, &cd, &mNode);
    if ((int)err == -2005) {
        AudioNodeException exception("badComponentType.");
        throw exception;
    }
    err = AUGraphNodeInfo(graph, mNode, NULL, &mUnit);
}

AudioUnitNode::~AudioUnitNode()
{
    AUGraphRemoveNode(getAUGraph(), getAUNode());
}

void AudioUnitNode::setFormat(const AudioStreamBasicDescription &asbd, AudioUnitScope scope, UInt32 bus)
{
    checkError(AudioUnitSetProperty(mUnit,
                                    kAudioUnitProperty_StreamFormat,
                                    scope,
                                    bus,
                                    &asbd,
                                    sizeof(asbd)),
               "Fail AudioUnitSetProperty");
}

void AudioUnitNode::input(AudioNode &node, UInt32 srcOutputCh, UInt32 inputCh)
{
    ioValidate(node, srcOutputCh, inputCh);
    checkError(AUGraphConnectNodeInput(node.getAUGraph(), node.getAUNode(), srcOutputCh, getAUNode(), inputCh), "Fail AUGraphConnectNodeInput");
}

void AudioUnitNode::disconnectInput(UInt32 inputCh)
{
    checkError(AUGraphDisconnectNodeInput(mGraph, mNode, inputCh), "Fail AUGraphDisconnectNodeInput");
}

void AudioUnitNode::bypass(Boolean bypass)
{
    UInt32 b = bypass ? 1 : 0;
    checkError(AudioUnitSetProperty(mUnit, kAudioUnitProperty_BypassEffect, kAudioUnitScope_Global, 0, &b, sizeof(UInt32)), "AudioUnitSetProperty Bypass");
}

const Boolean AudioUnitNode::doBypass() const
{
    UInt32 doB;
    UInt32 size = sizeof(UInt32);
    AudioUnitGetProperty(mUnit, kAudioUnitProperty_BypassEffect, kAudioUnitScope_Global, 0, &doB, &size);
    return doB == 1 ? true : false;
}

const UInt32 AudioUnitNode::numberOfInputs() const
{
    UInt32 numElements;
    UInt32 size = sizeof(UInt32);
    checkError(AudioUnitGetProperty(mUnit, kAudioUnitProperty_ElementCount, kAudioUnitScope_Input, 0, &numElements, &size), "AudioUnitGetProperty");
    return numElements;
}

const UInt32 AudioUnitNode::numberOfOutputs() const
{
    UInt32 numElements;
    UInt32 size = sizeof(UInt32);
    checkError(AudioUnitGetProperty(mUnit, kAudioUnitProperty_ElementCount, kAudioUnitScope_Output, 0, &numElements, &size), "AudioUnitGetProperty");
    return numElements;
}

const UInt32 AudioUnitNode::numberOfParameters(UInt32 scope) const
{
    UInt32 size;
    checkError(AudioUnitGetPropertyInfo(mUnit, kAudioUnitProperty_ParameterList, scope, 0, &size, NULL), "AudioUnitGetPropertyInfo");
    UInt32 numOfParams = size / sizeof(AudioUnitParameterID);
    return numOfParams;
}

const AudioUnitParameterInfo AudioUnitNode::getParameterInfo(AudioUnitParameterID pid, AudioUnitScope scope, AudioUnitElement bus) const
{
    AudioUnitParameterInfo param;
    UInt32 size = sizeof(param);
    AudioUnitGetProperty(mUnit, kAudioUnitProperty_ParameterInfo, scope, pid, &param, &size);
    return param;
};

const AudioUnitParameterValue AudioUnitNode::getParameter(AudioUnitParameterID pid, AudioUnitScope scope, AudioUnitElement bus) const
{
    AudioUnitParameterValue value;
    checkError(AudioUnitGetParameter(mUnit, pid, scope, bus, &value), "AudioUnitGetParameter");
    return value;
}

void AudioUnitNode::setParameter(AudioUnitParameterID pid, AudioUnitParameterValue value, AudioUnitScope scope, AudioUnitElement bus)
{
    checkError(AudioUnitSetParameter(mUnit, pid, scope, bus, value, 0), "AudioUnitSetParameter");
}

void AudioUnitNode::getProperty(AudioUnitPropertyID pid, AudioUnitScope scope, UInt32 bus, void* outValue, UInt32 size)
{
    checkError(AudioUnitGetProperty(mUnit, pid, scope, bus, outValue, &size), "AudioUnitGetProperty");
}

void AudioUnitNode::setProperty(AudioUnitPropertyID pid, AudioUnitScope scope, UInt32 bus, const void *value, UInt32 size)
{
    checkError(AudioUnitSetProperty(mUnit, pid, scope, bus, &value, size), "AudioUnitSetProperty");
}