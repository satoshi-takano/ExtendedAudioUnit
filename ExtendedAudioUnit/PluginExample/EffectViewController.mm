//
//  EffectViewController.m
//  ExtendedAudioUnit
//
//  Created by Satoshi Takano on 5/2/13.
//  Copyright (c) 2013 Satoshi Takano. All rights reserved.
//

#import "EffectViewController.h"

#include "AudioNode.hpp"
#include "AudioNodeDebugUtils.hpp"

@interface EffectViewController () {
    NSMutableArray* mControllers;
    void* mNode;
    NSMutableArray* mCurrentValueLabels;
}

@end

@implementation EffectViewController

- (id)initWithEffect:(void *)effect {
    if (self = [super init]) {
        UIScrollView* view = (UIScrollView*)self.view;
        
        AudioNode* audioNode;
        audioNode = (AudioNode*)effect;
        
        printParameters(audioNode);
        
        const UInt32 nParams = audioNode->numberOfParameters(kAudioUnitScope_Global);
        mControllers = [[NSMutableArray alloc] initWithCapacity:nParams];
        
        //UISwitch* bypass = [[UISwitch alloc] initWithFrame:CGRectMake(10, 20, 50, 30)];
        UIButton* bypass = [UIButton buttonWithType:UIButtonTypeRoundedRect];
        bypass.frame = CGRectMake(10, 20, 100, 40);
        [self.view addSubview:bypass];
        [bypass setTitle:@"Off" forState:UIControlStateNormal];
        [bypass setTitle:@"On" forState:UIControlStateSelected];
        if (audioNode->doBypass()) bypass.selected = YES;
        [bypass addTarget:self action:@selector(doBypass:) forControlEvents:UIControlEventTouchUpInside];
        //[bypass release];
        
        mCurrentValueLabels = [[NSMutableArray alloc] initWithCapacity:nParams];
        
        int i;
        for (i = 0; i < nParams; i++) {
            AudioUnitParameterInfo info = audioNode->getParameterInfo(i, kAudioUnitScope_Global, 0);
            UILabel* nameLabel = [[UILabel alloc] initWithFrame:CGRectMake(10, 70 + 90 * i, 300, 40)];
            nameLabel.text = [(__bridge NSString*)info.cfNameString capitalizedString];
            [self.view addSubview:nameLabel];
            
            UISlider* slider = [[UISlider alloc] initWithFrame:CGRectMake(15, nameLabel.frame.origin.y + 40, 320 - 30, 20)];
            [self.view addSubview:slider];
            [mControllers addObject:slider];
            Float32 ratio = (audioNode->getParameter(i, kAudioUnitScope_Global, 0) - info.minValue) / (info.maxValue - info.minValue);
            slider.value = ratio;
            slider.tag = i;
            [slider addTarget:self action:@selector(updateValue:) forControlEvents:UIControlEventValueChanged];
            
            UILabel* minLabel = [[UILabel alloc] initWithFrame:CGRectMake(10, slider.frame.origin.y + 35, 100, 20)];
            minLabel.font = [UIFont systemFontOfSize:11];
            minLabel.text = [NSString stringWithFormat:@"%.1f", info.minValue];
            [self.view addSubview:minLabel];
            minLabel.backgroundColor = [UIColor clearColor];
            
            UILabel* maxLabel = [[UILabel alloc] initWithFrame:CGRectMake(300, minLabel.frame.origin.y, 100, 20)];
            maxLabel.font = [UIFont systemFontOfSize:11];
            maxLabel.text = [NSString stringWithFormat:@"%.1f", info.maxValue];
            [self.view addSubview:maxLabel];
            maxLabel.backgroundColor = [UIColor clearColor];
            [maxLabel sizeToFit];
            maxLabel.frame = CGRectMake(310 - maxLabel.frame.size.width, maxLabel.frame.origin.y, maxLabel.frame.size.width, maxLabel.frame.size.height);
            
            UILabel* unitLabel = [[UILabel alloc] initWithFrame:CGRectMake(0, maxLabel.frame.origin.y + 15, 100, 20)];
            unitLabel.font = [UIFont systemFontOfSize:11];
            unitLabel.text = [NSString stringWithFormat:@"(%s)", uidToChar(info.unit)];
            [self.view addSubview:unitLabel];
            [unitLabel sizeToFit];
            unitLabel.frame = CGRectMake(310 - unitLabel.frame.size.width, unitLabel.frame.origin.y, unitLabel.frame.size.width, unitLabel.frame.size.height);
            
            AudioUnitParameterValue currentValue = audioNode->getParameter(i, kAudioUnitScope_Global, 0);
            Float32 r = (currentValue - info.minValue) / (info.maxValue - info.minValue);
            UILabel* currentValueLabel = [[UILabel alloc] initWithFrame:CGRectMake(10 + 300 * r, minLabel.frame.origin.y - 10 , 100, 20)];
            currentValueLabel.font = [UIFont systemFontOfSize:11];
            [self.view addSubview:currentValueLabel];
            currentValueLabel.text = [NSString stringWithFormat:@"%.1f", currentValue];
            [mCurrentValueLabels addObject:currentValueLabel];
            [currentValueLabel sizeToFit];
            currentValueLabel.frame = CGRectMake(10 + 300 * r - (currentValueLabel.frame.size.width / 2), currentValueLabel.frame.origin.y, currentValueLabel.frame.size.width, currentValueLabel.frame.size.height);
        }
        view.contentSize = CGSizeMake(self.view.frame.size.width, 70 + 90 * i + 40);
        
        mNode = (void*)audioNode;
    }
    return self;
}

- (void)loadView {
    CGRect bounds = [[UIScreen mainScreen] bounds];
    UIScrollView* view = [[UIScrollView alloc] initWithFrame:CGRectMake(0, 0, bounds.size.width, bounds.size.height)];
    self.view = view;
    view.backgroundColor = [UIColor whiteColor];
}

- (void)updateValue:(UISlider*)slider {
    AudioNode* audioNode = (AudioNode*)mNode;
    AudioUnitParameterInfo info = audioNode->getParameterInfo(slider.tag, kAudioUnitScope_Global, 0);
    audioNode->setParameter(slider.tag, info.minValue + (info.maxValue - info.minValue) * slider.value, kAudioUnitScope_Global, 0);
    
    
    AudioUnitParameterValue currentValue = audioNode->getParameter(slider.tag, kAudioUnitScope_Global, 0);
    UILabel* currentValueLabel = [mCurrentValueLabels objectAtIndex:slider.tag];
    Float32 r = (currentValue - info.minValue) / (info.maxValue - info.minValue);
    currentValueLabel.text = [NSString stringWithFormat:@"%.1f", currentValue];
    [currentValueLabel sizeToFit];
    currentValueLabel.frame = CGRectMake(10 + 300 * r - (currentValueLabel.frame.size.width / 2), currentValueLabel.frame.origin.y, currentValueLabel.frame.size.width, currentValueLabel.frame.size.height);
}

- (void)doBypass:(UIButton*)swt {
    [swt setSelected:!swt.selected];
    
    AudioNode* effect = (AudioNode*)mNode;
    effect->bypass(swt.selected);
    effect->bypass(swt.selected);
}
@end
