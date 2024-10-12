/**
 * Copyright 2021 Guillaume AUJAY. All rights reserved.
 * Distributed under the Apache License Version 2.0
 */

#include "debug_utils.h"

// Static init
bool DbgClass::Dbg = false;
int DbgClass::idx = 0;
int DbgClass::count = 0;

int dClass::count   = 0;
int dClass::decount = 0;
uint64_t dClass::copies = 0u;
uint64_t dClass::moves  = 0u;
bool dClass::quiet  = true;
