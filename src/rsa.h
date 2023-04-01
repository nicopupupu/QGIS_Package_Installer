//  Simple RSA Implementation
//  * Do not use this in production! *
//  Created by Arve Nygård on 05/05/15.
//  Copyright (c) 2015 Arve Nygård. All rights reserved.
//

#ifndef __rsa__rsa__
#define __rsa__rsa__

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <thread>

#include "lib/ttmath.h"
class Rsa;
typedef ttmath::Int<16> num; // 16 words. 16*64 = 1024 bit
typedef num (*expFunc)(const num&, const num&, const num&);
enum ExpType {
    POWERLADDER,
    MODEXP,
    MODEXP_SLEEP
};

class Rsa {

private:
    num p, q, theta;
    expFunc ef;
public:
    /* These could probably be 