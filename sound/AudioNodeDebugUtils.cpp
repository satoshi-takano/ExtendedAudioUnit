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

#include "AudioNodeDebugUtils.hpp"

void printBusses(AudioNode* node) {
  std::cout << "Input  bus count : " << node->numberOfInputs() << std::endl;
  std::cout << "Output bus count : " << node->numberOfOutputs() << std::endl;
}

const char* uidToChar(AudioUnitParameterUnit uid) {
  switch (uid) {
    case kAudioUnitParameterUnit_Indexed:
        return "indexed";
      break;
    case kAudioUnitParameterUnit_Boolean:
        return "boolean";
      break;
    case kAudioUnitParameterUnit_Percent:
        return "percent";
      break;
    case kAudioUnitParameterUnit_Seconds:
        return "seconds";
      break;
    case kAudioUnitParameterUnit_SampleFrames:
        return "sample frames";
      break;
    case kAudioUnitParameterUnit_Rate:
        return "rate";
      break;
    case kAudioUnitParameterUnit_Hertz:
        return "hertz";
      break;
    case kAudioUnitParameterUnit_Cents:
        return "cent";
      break;
    case kAudioUnitParameterUnit_RelativeSemiTones:
      return "relative semitones";
      break;
    case kAudioUnitParameterUnit_MIDINoteNumber:
      return "midi notenumber";
      break;
    case kAudioUnitParameterUnit_MIDIController:
      return "midi controller";
      break;
    case kAudioUnitParameterUnit_Decibels:
      return "decibels";
      break;
    case kAudioUnitParameterUnit_LinearGain:
      return "linear gain";
      break;
    case kAudioUnitParameterUnit_Degrees:
      return "degrees";
      break;
    case kAudioUnitParameterUnit_EqualPowerCrossfade:
      return "equal power crossfade";
      break;
    case kAudioUnitParameterUnit_MixerFaderCurve1:
      return "mixier fader curve1";
      break;
    case kAudioUnitParameterUnit_Pan:
      return "pan";
      break;
    case kAudioUnitParameterUnit_Meters:
      return "meters";
      break;
    case kAudioUnitParameterUnit_AbsoluteCents:
      return "absolute cents";
      break;
    case kAudioUnitParameterUnit_Octaves:
      return "octaves";
      break;
    case kAudioUnitParameterUnit_BPM:
      return "bpm";
      break;
    case kAudioUnitParameterUnit_Beats:
      return "beats";
      break;
    case kAudioUnitParameterUnit_Milliseconds:
      return "milliseconds";
      break;
    case kAudioUnitParameterUnit_Ratio:
      return "ratio";
      break;
    default:
      return "";
      break;
  } 
}

void printParameterInfo(AudioUnitParameterInfo info) {
    CFShow(info.cfNameString);
    std::cout
    //      << "   clumpID : " << info.clumpID << std::endl
    << "   Value range : " << info.minValue << " ~ " << info.maxValue << " (" << uidToChar(info.unit) << ")" << std::endl;
    std::cout << "   Default val : " << info.defaultValue << std::endl;
}

void printParameters(AudioNode* node) {
  AudioUnitScope scope = kAudioUnitScope_Global;
  UInt32 numParams = node->numberOfParameters(scope);
  for (int i = 0; i < numParams; i++) {
    AudioUnitParameterInfo info = node->getParameterInfo(i, scope, 0);
    std::cout << i << ". ";
    CFShow(info.cfNameString);
    std::cout 
    //      << "   clumpID : " << info.clumpID << std::endl
    << "   Value range : " << info.minValue << " ~ " << info.maxValue << " (" << uidToChar(info.unit) << ")" << std::endl; 
    std::cout << "   Default val : " << info.defaultValue << std::endl;
  }
  std::cout << std::endl;
}