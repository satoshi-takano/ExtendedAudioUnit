//
//  PluginExampleViewController.m
//  PluginExample
//
//  Created by Satoshi Takano on 5/2/13.
//  Copyright (c) 2013 Satoshi Takano. All rights reserved.
//

#import "PluginExampleViewController.h"
#import "EffectViewController.h"

#include <vector>

#include "AudioTremoroNode.hpp"
#include "AudioDistortionNode.hpp"
#include "AudioDelayNode.hpp"
#include "AudioBitCrusherNode.hpp"
#include "AudioLevelNode.hpp"

#import "AudioContext.hpp"
#import "AudioUnitNode.hpp"
#import "AudioFileMemoryBufferNode.hpp"
#import "AudioFileStreamingNode.hpp"

#import "AUStreamBasicDescription.hpp"

@interface PluginExampleViewController() {
    NSMutableArray* effectNames;
    std::vector<AudioNode*> effects;
}

@end

@implementation PluginExampleViewController

static AudioContext* context;
static AudioNode* destination;
static AudioNode* mixer;
static AudioSourceNode* source;

- (id)init {
    if (self = [super init]) {
        effectNames = [@[] mutableCopy];
    }
    return self;
}

- (void)loadView {
    [super loadView];
    
    [self createAudioNodes];
    
    NSURL* url = [NSURL fileURLWithPath:[[NSBundle mainBundle] pathForResource:@"local" ofType:@"mp3"]];
    source = new AudioFileMemoryBufferNode(context->getGraph(), (__bridge CFURLRef)url);
    
    // set audio format
    AUStreamBasicDescription asbd;
    asbd.SetFloat(2);
    asbd.mSampleRate = 44100;
    source->setFormat(asbd, kAudioUnitScope_Output, 0);

    // set loop
    source->setLoopInFrame(0);
    source->setLoopOutFrame(source->getTotalFrames());
    source->enableLoop(true);
    
    [self connect:source];
}

- (void)createAudioNodes {
    AUComponentDescription desc;
    context = new AudioContext();
    destination = new AudioUnitNode(context->getGraph(), desc.setOutputType(kAudioUnitSubType_RemoteIO));
    mixer = new AudioUnitNode(context->getGraph(), desc.setMixerType(kAudioUnitSubType_MultiChannelMixer));
    
    effects.push_back(new AudioLevelNode(context->getGraph()));
    [effectNames addObject:@"Level (Custom)"];
    
    effects.push_back(new AudioTremoroNode(context->getGraph()));
    [effectNames addObject:@"Tremoro (Custom)"];
    
    effects.push_back(new AudioDistortionNode(context->getGraph()));
    [effectNames addObject:@"Distortion (Custom)"];
    
    effects.push_back(new AudioDelayNode(context->getGraph()));
    [effectNames addObject:@"Delay (Custom)"];
    
    effects.push_back(new AudioUnitNode(context->getGraph(), desc.setEffectType(kAudioUnitSubType_HighPassFilter)));
    [effectNames addObject:@"High pass filter (Native)"];
    
    effects.push_back(new AudioUnitNode(context->getGraph(), desc.setEffectType(kAudioUnitSubType_LowPassFilter)));
    [effectNames addObject:@"Low pass filter (Native)"];
}

- (void)connect:(AudioSourceNode*)source {
    std::vector<AudioNode*>::iterator it = effects.begin();
    AudioNode* effect = *it++;
    effect->bypass(true);
    effect->input(*source, 0, 0);
    
    while (it != effects.end()) {
        AudioNode* nextEffect = *it++;
        nextEffect->input(*effect, 0, 0);
        effect = nextEffect;
        effect->bypass(true);
        effect->setFormat(source->getFormat(kAudioUnitScope_Output, 0), kAudioUnitScope_Input, 0);
        effect->setFormat(source->getFormat(kAudioUnitScope_Output, 0), kAudioUnitScope_Output, 0);
    }
    
    mixer->input(*effect, 0, 0);
    destination->input(*mixer, 0, 0);
    
    context->start();
    source->startRendering();
}


#pragma mark -- table view delegate --
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return effectNames.count;
}

- (UITableViewCell*)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:@"effectsList"];
    if (cell == nil) {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:@"effectsList"];
    }
    NSString* name = [effectNames objectAtIndex:indexPath.row];
    cell.textLabel.text = name;
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    AudioNode* effect = (AudioNode*)effects.at(indexPath.row);
    EffectViewController* viewController = [[EffectViewController alloc] initWithEffect:effect];
    
    [self.navigationController pushViewController:viewController animated:YES];
}

- (void)dealloc {
    delete context;
    delete destination;
    delete mixer;
    delete source;
}

@end
