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

#ifndef FSLIP_H_
#define FSLIP_H_

#pragma once

#include "stdint.h"
#include "assert.h"

namespace fou {
namespace slip {
  
enum SpecialCharacters {
  kEnd    = 192,   // end of packet
  kEsc    = 219,   // byte stuffing
  kEscEnd = 220,   // esc esc_end means an END data byte
  kEscEsc = 221    // esc esc_esc means an ESC data byte
};


class Decoder {
 public:
   Decoder(uint8_t *buffer, int capacity) : mBuffer(buffer), mCapacity(capacity), mPacketLength(0), mEscMode(false), mReady(false)
    { 
    }
    
    inline void clear() { mPacketLength = 0; mReady = false; }
    
    inline int getSize() const { return mPacketLength; }
    
    inline bool hasPacket() const { return mReady; };
    
    int16_t getAsI16( int i ) {
      int16_t o;
      *(((uint8_t *)&o ) + 1) = mBuffer[i];
      *(uint8_t *)&o = mBuffer[i+1];
      return o;
    }
    
    inline uint16_t getAsU16( int i ) {
      assert( (i >= 0) and ( i+1 < mPacketLength) );
      return ((uint16_t)mBuffer[i] << 8) | (uint16_t)mBuffer[i+1];
    }

    inline uint8_t getByte( int i ) {
      assert( (i >= 0) and (i < mPacketLength) );
      return mBuffer[i];
    }

    bool pushBack( uint8_t c) 
    {
      if (mEscMode) {
        switch( c ) {
          case slip::kEscEnd:
            c = slip::kEnd;
            break;
          case slip::kEscEsc:
            c = slip::kEsc;
            break;
          default:
            // protocol violation.
            return false;
        }
        
        if( mPacketLength == mCapacity ) return false;
        // when we are ready and receive something new. 
        // discard old stuff in favour for new stuff.
        if( mReady ) clear();
        mBuffer[mPacketLength] = c;
        mPacketLength++;
        mEscMode = false;
        return true;
      }
      switch( c ) {
        case slip::kEnd:
          if (mPacketLength == 0) break; // ignore end with zero length
          // confirm packet
          mReady = true;
          break;
        case slip::kEsc:
          mEscMode = true;
          break;
        default:
          if( mPacketLength == mCapacity ) return false;
          
          // when we are ready and receive something new. 
          // discard old stuff in favour for new stuff.
          if( mReady ) clear();
          mBuffer[mPacketLength] = c;
          mPacketLength++;
      }
      return true;
    }

 protected:
   uint8_t *mBuffer;
   int mCapacity;
   int mPacketLength;
   bool mEscMode;
   bool mReady;
};

  
class Encoder {
  public:
    Encoder(uint8_t *aBuffer, int aCapacity) : mBuffer(aBuffer), mCapacity(aCapacity),  mPacketLength(0)
    {
    }
    bool endPacket()
    {
      if ( mPacketLength == mCapacity ) return false;
      mBuffer[mPacketLength] = slip::kEnd;
      mPacketLength++;
      return true;
    }

    inline int getSize() { return mPacketLength; }
    inline int capacityLeft() { return mCapacity - mPacketLength; }

    inline bool isEmpty() { return mPacketLength == 0 ?  true : false; }

    inline void clear() { mPacketLength = 0; };

    inline uint8_t getByte( int i ) {
      assert( (i >= 0) and (i < mPacketLength) );
      return mBuffer[i]; 
    }
    bool pushBackU16( uint16_t v )
    {
        bool r = pushBack( v >> 8);
        r |= pushBack( v & 0xff );
        return r;
    }

    bool pushBack( uint8_t c ) 
    {
      switch( c ) {
        case slip::kEnd:
          if ( capacityLeft() < 2 ) return false;
          mBuffer[mPacketLength++] = slip::kEsc;
          mBuffer[mPacketLength++] = slip::kEscEnd;
          break;
        case slip::kEsc:
          if ( capacityLeft() < 2 ) return false;
          mBuffer[mPacketLength++] = slip::kEsc;
          mBuffer[mPacketLength++] = slip::kEscEsc;
          break;
        default:
          if ( capacityLeft() == 0 ) return false;
          mBuffer[mPacketLength++] = c;
          break;
      }
      return true;
    }
  protected:
   uint8_t *mBuffer;
   int mCapacity;
   int mPacketLength;
  };
} // end namespace slip
} // end namespace fou

#endif
