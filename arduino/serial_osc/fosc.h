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

#ifndef FOSC_H_
#define FOSC_H_

#include "stdint.h"
#include "stddef.h"

namespace fou {
namespace osc {

typedef enum {
	kFOSC_BUNDLE,
	kFOSC_MESSAGE,
	kFOSC_UNKOWN_ELEMENT
} ElementType_t;

typedef struct {
	uint32_t size;
	char *data;
} Blob_t;

typedef struct {
	uint32_t sec;
	uint32_t frac;
} TimeTag_t;

typedef enum {
	/* TODO: internal OSC types */	
	kFOSC_DONE =		0, 	/** iterator : no more tags in the string */
	kFOSC_UNKNOWN =	1, 	/** unknown tag */
	
	// basic OSC types 
	kFOSC_INT32 =     'i',   /** 32 bit signed integer. */
	kFOSC_FLOAT =     'f',  /** 32 bit IEEE-754 float. */
	kFOSC_STRING =    's',  /** Standard C, NULL terminated string. */
	kFOSC_BLOB =      'b', /** OSC binary blob type. */
	
	// extended OSC types 	
	kFOSC_INT64 =     'h', /** 64 bit signed integer. */
	kFOSC_TIMETAG =   't', 	/** OSC TimeTag type. */
	kFOSC_DOUBLE =    'd', 	/** 64 bit IEEE-754 double. */
  kFOSC_SYMBOL =    'S', /** Standard C, NULL terminated, string. */
	kFOSC_CHAR =      'c', /** Standard C, 8 bit, char variable. */
	kFOSC_MIDI =      'm', 	/** A 4 byte MIDI packet. */
	kFOSC_TRUE =      'T', 	/** Sybol representing the value True. */
	kFOSC_FALSE =     'F', /** Sybol representing the value False. */
	kFOSC_NIL =       'N', 	/** Sybol representing the value Nil. */
	kFOSC_INFINITUM = 'I' 	/** Symbol representing the value Infinitum. */
} TypeTag_t;


/**
 *  A Open Sound Control Message (OSC) Iterator. The Message Iterator encodes 
 *  and decodes OSC messages to and from a buffer. The Message Iterator makes
 *  use of external buffers (such as lwIP pbuf. It simply provides a friendly
 *  interface to read and write from those buffers. 
 */
class MessageIterator {
  
public:
  /**
   *  Constructor.
   */
  MessageIterator() : 
  buffer_(NULL), arg_types_(NULL), args_(NULL), arg_types_size_(0), mesg_size_(0) {};
  

// #ifdef FOU_USE_STD_ARG  
  /**
   *  Encode an OSC message using a variable argument list. Requires stdarg.h.
   *  @param buffer the output buffer.
   *  @param capacity the output buffer capacity.
   *  @param addr is the OSC address.
   *  @param typetags is the string with arguments.
   *  @param ... arguments
   *  @return true on success. 
   *  @see encode()
   */
//  bool encode(char *buf, uint16_t capacity, char *addr, char *typetags, ...);
//#endif  


  bool encode(char* out_buffer, int capacity, const char *addr, const char *typetags);
  bool append_i(int32_t i);
  bool append_f(float f);
  bool append_s(const char *s);
  bool append_b(uint8_t *data, int32_t size);
  
  
  bool decode(char* buf, int size);
  bool i(int32_t &i);
  bool f(float &f);
  int s(char** s);
  int32_t b(uint8_t* data);
  
  
  /**
   *  Get the address string.
   *  @return the string.
   */    
  inline char *address() const { return buffer_; };
  /**
   *  Get the arg type string.
   *  @return the string.
   */      
  inline char *types() const { return arg_types_; };
  /**
   *  Get the number of arguments. This might not be equal to the length of the 
   *  type string with future array support [].
   *  @return the number of arguments.
   */      
  uint16_t args_size() const { return arg_types_size_; };
  /**
   *  Return the current type of the current argument.
   *  @return the argument's type.
   */
  TypeTag_t arg_type() const;
  /**
   *  Return the current size of the message.
   *  @return the size of the message.
   */  
  inline int size() const { return mesg_size_;};
  
private:
  inline void pad() {
    while (0 != (mesg_size_ & 3)) {
      buffer_[mesg_size_] = '\0';
      mesg_size_++;
    }
  };
  
  bool append_data_and_pad(uint8_t *src, uint32_t size);
  bool append_string_and_pad(const char *src);
  char *buffer_; // pointer to the start of the message
  char *arg_types_; // pointer to the start of the argument types 
  char *args_;	// a convenience pointer to iterate through the arguments 	
  int arg_types_size_;
  int capacity_;            
  int mesg_size_;	// length of the total message 
  int args_index_;
};

class BundleIterator {
  
public:
  BundleIterator() : buffer_(0), capacity_(0), size_(0) {};
  
  inline ElementType_t element_type() {
    if (buffer_[size_] == '/') return kFOSC_MESSAGE;
    if (buffer_[size_] == '#') return kFOSC_BUNDLE;
    return kFOSC_UNKOWN_ELEMENT;
  };
  inline bool element_is_bundle() {
    if (buffer_[size_] == '#') return true;
    return false;
  }
  inline bool element_is_message() {
    if (buffer_[size_] == '/') return true;
    return false;
  }
  
  void timetag(int32_t &sec, int32_t &frac);
  void set_timetag(int32_t sec, int32_t frac);
  inline int size() { return size_; };
  
  bool encode(char* buffer, int capacity);
  bool begin_message(MessageIterator &mi, char *address, char *typetags);
  void end_message(const MessageIterator &mi);
#ifdef FOU_USE_STD_ARG  
  // bool add_message(char *addr, char *typetags, ...);
#endif
  
  bool begin_bundle(BundleIterator &bundle);
  void end_bundle(BundleIterator &bundle);
  
  
  bool decode(char* buffer, int size);
  bool element(MessageIterator &mi);
  bool element(BundleIterator &bi);
  bool element(char **buffer, int &size);
  
  bool append_data_and_pad(uint8_t *src, uint32_t size);
  bool append_string_and_pad(const char *src);
  
private:
  char *buffer_;
  char *element_;
  int capacity_;              
  int size_; 
  
  // TODO: copy of message iterator.
  inline void pad() {
    while (0 != (size_ & 3)) {
      buffer_[size_] = '\0';
      size_++;
    }
  };
};





} } // end namespace fou / osc


#endif
