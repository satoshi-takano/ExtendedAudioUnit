//
//  Runnable.h
//  IKKitDevelop
//
//  Created by  on 4/8/12.
//  Copyright (c) 2012 アップルジャパン株式会社. All rights reserved.
//

#pragma once

class Runnable {
public:
  Runnable(){}
  virtual ~Runnable(){}
  virtual void run() = 0;
};