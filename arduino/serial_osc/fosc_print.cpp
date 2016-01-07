/*
 *
 This file is part of Fou.
 
 The MIT License (MIT)
 
 Copyright (C) 2010  Daniel Saakes
 
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
 *
 */

#include "fosc_print.h"
#include "fosc.h"

#include "Arduino.h"


void printMessage(char *buf, int capacity) {
  fou::osc::MessageIterator mi;
  
  int32_t i32;
  float f;
  uint8_t* blob; blob = 0;
  char* str; str = 0;
  int data_size;
  mi.decode(buf,capacity);
  
  Serial.print("osc message:\n");
  Serial.print("\taddress: "); Serial.println(mi.address());
  Serial.print("\n\ttype tag string: "); Serial.print(mi.types()); Serial.print(" length("); Serial.print(mi.args_size()); Serial.println(")");
  
  for(int i=0;i<mi.args_size();i++) {
    switch(mi.arg_type()) {
      case fou::osc::kFOSC_INT32:
        mi.i(i32);
        Serial.print("\t\tinteger: "); Serial.println((int)i32);
        break;
      case fou::osc::kFOSC_FLOAT:
        mi.f(f);
        Serial.print("\t\tfloat:"); Serial.println(f); 
        break;
      case fou::osc::kFOSC_STRING:
        data_size = mi.s(&str);
        Serial.print("\t\tstring: length("); Serial.print(data_size); Serial.print(") "); Serial.println(str);
        break;
      case fou::osc::kFOSC_BLOB:
        data_size = mi.b(blob);
        
        Serial.print("\t\tblob: size("); Serial.print(data_size); Serial.println(")");
        break;
      default:
        Serial.print("\t\tunknown argument.. bail out\n\n");
        return;
        break;
    }
  }
}

void printBundle(char *buffer, int size) {
  fou::osc::BundleIterator bi;
  // TimeTag_t tt;
  int element_size;
  char *element_buffer;
  
  bi.decode(buffer, size);
  //printf("osc bundle:\n");
  //printf("\ttimetag (sec,frac):%d,%d\n", tt.sec, tt.frac);
  
  // TODO:: make something in bundle, that makes it easy to see when done.
  while( bi.element(&element_buffer, element_size)) {
    if (bi.element_is_bundle()) {
//      printBundle(element_buffer, element_size);
      continue;
    }
    if (bi.element_is_message()) {
//      printMessage(element_buffer, element_size);
      continue;
    }
    Serial.print("unknown element in bundle\n");
    break;
  }
  Serial.print("osc bundle done\n");
}






