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

@interface BasicExampleViewController ()

@end

@implementation BasicExampleViewController

- (id)init {
    if (self = [super init]) {
        
    }
    return self;
}

- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    
    AudioContext* context = new AudioContext();
    
    AUComponentDescription desc;
    AudioNode* destination = new AudioUnitNode(context->getGraph(), desc.setOutputType(kAudioUnitSubType_RemoteIO));
    AudioUnitNode* mixer = new AudioUnitNode(context->getGraph(), desc.setMixerType(kAudioUnitSubType_MultiChannelMixer));
    
    NSURL* url = [NSURL fileURLWithPath:[[NSBundle mainBundle] pathForResource:@"local" ofType:@"mp3"]];
    AudioSourceNode* source = new AudioFileMemoryBufferNode(context->getGraph(), (__bridge CFURLRef)url);
    
    mixer->input(*source, 0, 0);
    destination->input(*mixer, 0, 0);

    context->start();
    source->startRendering();
}

@end
