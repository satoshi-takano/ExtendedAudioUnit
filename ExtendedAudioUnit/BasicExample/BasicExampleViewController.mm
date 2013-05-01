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

@interface BasicExampleViewController ()

@end

@implementation BasicExampleViewController

static AudioContext* context;
static AudioNode* destination;
static AudioNode* mixer;

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

- (void)connect:(AudioSourceNode*)source {
    mixer->input(*source, 0, 0);
    destination->input(*mixer, 0, 0);
    
    context->start();
    source->startRendering();
}

- (void)playLocalMP3 {
    [self createAudioNodes];
    
    NSURL* url = [NSURL fileURLWithPath:[[NSBundle mainBundle] pathForResource:@"local" ofType:@"mp3"]];
    AudioSourceNode* source = new AudioFileMemoryBufferNode(context->getGraph(), (__bridge CFURLRef)url);
    
    [self connect:source];
}

- (void)playRemoteMP3 {
    [self createAudioNodes];    
    
    NSURL* url = [NSURL URLWithString:@"http://octoberlab.com/projects/resources/remote.mp3"];
    NSURL* tmpURL = [NSURL fileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:@"tmp.caf"]];
    AudioSourceNode* source = new AudioFileStreamingNode(context->getGraph(), (__bridge CFURLRef)url, (__bridge CFURLRef)tmpURL);
    
    [self connect:source];
}

- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
}

@end
