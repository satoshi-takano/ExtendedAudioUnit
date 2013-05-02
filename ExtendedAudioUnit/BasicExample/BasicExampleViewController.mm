//
//  BasicExampleViewController.m
//  ExtendedAudioUnit
//
//  Created by Satoshi Takano on 5/1/13.
//  Copyright (c) 2013 Satoshi Takano. All rights reserved.
//

#import "BasicExampleViewController.h"

#import "AudioContext.hpp"
#import "AudioUnitNode.hpp"
#import "AudioFileMemoryBufferNode.hpp"
#import "AudioFileStreamingNode.hpp"

@implementation BasicExampleViewController

static AudioContext* context;
static AudioNode* destination;
static AudioNode* mixer;
static AudioSouceNode* source;

- (id)init {
    if (self = [super init]) {
        
    }
    return self;
}

- (void)createAudioNodes {
    AUComponentDescription desc;
    context = new AudioContext();
    destination = new AudioUnitNode(context->getGraph(), desc.setOutputType(kAudioUnitSubType_RemoteIO));
    mixer = new AudioUnitNode(context->getGraph(), desc.setMixerType(kAudioUnitSubType_MultiChannelMixer));
}

- (void)deleteAudioNodes {
    delete context; context = 0;
    delete destination; destination = 0;
    delete mixer; mixer = 0;
}

- (void)connect:(AudioSourceNode*)source {
    mixer->input(*source, 0, 0);
    destination->input(*mixer, 0, 0);
    
    context->start();
    source->startRendering();
}

- (void)playLocalMP3 {
    [self createAudioNodes];
    
    NSURL* url = [NSURL fileURLWithPath:[[NSBundle mainBundle] pathForResource:@"local" ofType:@"mp3"]];
    source = new AudioFileMemoryBufferNode(context->getGraph(), (__bridge CFURLRef)url);
    
    [self connect:source];
}

- (void)playRemoteMP3 {
    [self createAudioNodes];    
    
    NSURL* url = [NSURL URLWithString:@"http://octoberlab.com/projects/resources/remote.mp3"];
    NSURL* tmpURL = [NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:@"tmp.caf"]];
    source = new AudioFileStreamingNode(context->getGraph(), (__bridge CFURLRef)url, (__bridge CFURLRef)tmpURL);
    
    [self connect:source];
}

- (void)dealloc {
    delete context;
    delete destination;
    delete mixer;
    delete source;
}

@end
