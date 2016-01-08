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

#include "fosc.h"

#include <string.h>   // strlen

#ifdef FOU_USE_STD_ARG
#include <stdarg.h> // provides variable arguments
#endif

#include <assert.h>

#define PADL(x) ((x-1)&~3)

// #define DEBUG Serial.print

using namespace fou::osc;

/*
 *  arduino uses Little Endian byte order whereas OSC uses big endian AKA network byte order/
 *  these functions convert 32 bit data accordingly. 
 */

// Network to Host
inline void copyNTOHL(char *dst, char *src) {
  dst[0] = src[3];
  dst[1] = src[2];
  dst[2] = src[1];
  dst[3] = src[0];
}

// Host to Network
inline void copyHTONL(char *dst, char *src) {
  dst[0] = src[3];
  dst[1] = src[2];
  dst[2] = src[1];
  dst[3] = src[0];
}



/*
 uint32_t fosc_blob_size(fosc_blob b) {
 return b.size;
 }
 void *fosc_blob_ptr(fosc_blob b) {
 return (void*)b.data;
 }
 */


bool MessageIterator::append_data_and_pad(uint8_t *src, uint32_t size) {
  // TODO:make a proper size check. %3 is not correct
  if (capacity_ < (int)size + mesg_size_ + (int)(size%3)) return false;
  memcpy(&buffer_[mesg_size_],src,size);
  mesg_size_+=size;
  pad();
  return true;
}

bool MessageIterator::append_string_and_pad(const char *src) {
  // TODO:make a proper size check. %3 is not correct
  if (capacity_ < (mesg_size_+(int)strlen(src)+(int)(strlen(src)%3))) return false;
  while ( *src != '\0') {
    buffer_[mesg_size_] = *src;
    src++;
    mesg_size_++;
  }
  // append 0
  buffer_[mesg_size_] = 0; mesg_size_++;
  pad();
  return true;
}



TypeTag_t MessageIterator::arg_type() const {
  if ( arg_types_==NULL) return kFOSC_UNKNOWN;
  // DEBUG("arg types, index :"); 
  // DEBUG(arg_types_);
  // DEBUG(args_index_);
  switch( arg_types_[args_index_]) {
    case 'f':
    case 'i':
    case 's':
    case 'b':
      return (TypeTag_t)arg_types_[args_index_];
      break;
    default:
      return kFOSC_UNKNOWN;
      break;
  }
}

/**
 *  Decode an OSC message.
 *  @param buffer the input buffer.
 *  @param capacity the input buffer capacity.
 *  @return true on success. 
 */
bool MessageIterator::decode(char* buf, int size) {
  // assert(buf!=NULL);
  buffer_ = buf;
  mesg_size_ = 0;
  // DEBUG("decode:addres: "); DEBUG(buffer_); DEBUG("\n");
  
  mesg_size_+=strlen(buffer_)+1; // including the '\0' character
  pad();
  if (buffer_[mesg_size_] != ',') {
    // DEBUG("fosc error: Ignoring incoming message. No typetag string\n");
    return false;
  }
  mesg_size_++;
  args_index_=0;
  arg_types_ = &buffer_[mesg_size_];
  // DEBUG("decode:arg_types: "); DEBUG(arg_types_); DEBUG("\n");
  
  arg_types_size_ = strlen(&buffer_[mesg_size_]);
  mesg_size_+=arg_types_size_+1;
  pad();
  return true;
}

/**
 *  Retrieve an int. Use when decoding a message, order does matter.
 *  @param i the int.
 *  @return true on success.
 *  @see decode()
 */  
bool MessageIterator::i(int32_t &i) {
  copyHTONL((char*)&i,&buffer_[mesg_size_]);
  args_index_++;
  mesg_size_+=4;
  return true;
}

/**
 *  Retrieve a float. Use when decoding a message, order does matter.
 *  @param f the float.
 *  @return true on success.
 *  @see decode()
 */    
bool MessageIterator::f(float &f) {
  copyHTONL((char*)&f,&buffer_[mesg_size_]);
  args_index_++;
  mesg_size_+=4;
  return true;
}

/**
 *  Retrieve a string. Use when decoding a message, order does matter.
 *  @param s a pointer to the string. 
 *  @return the size of the string.
 *  @see decode()
 */    
int MessageIterator::s(char **s) {
  // TODO: restrain size
  *s = &buffer_[mesg_size_];
  // DEBUG("string....."); DEBUG(&buffer_[mesg_size_]);
  int len = strlen(&buffer_[mesg_size_]);
  mesg_size_+=len+1;
  // pad
  if (mesg_size_ & 3) mesg_size_ += 4 - (mesg_size_ & 3); // add padding.
  args_index_++;
  return len;
}

/**
 *  Retrieve a blob. Use when decoding a message, order does matter.
 *  @param b the string.
 *  @return true on success.
 *  @see decode()
 */    
int32_t MessageIterator::b(uint8_t *data) {
  // copy the blob
  int32_t size;
  copyHTONL((char *)&size,&buffer_[mesg_size_]);	// append size
  mesg_size_+=4+size;
  // TODO foscAppendDataAndPad(&(args_), blob->size, blob->data);
  args_index_++;
  return size;
}

/**
 *  Encode an OSC message.
 *  @param buffer the output buffer.
 *  @param capacity the output buffer capacity.
 *  @param addr is the OSC address.
 *  @param type_tag is the string with arguments.
 *  @return true on success. 
 *  @see encode()
 */
bool MessageIterator::encode(char* out_buffer, int capacity, const char *addr, const char *typetags) {
  // assert(out_buffer!=NULL);
  // assert( *addr == '/'); // is it a message ?
  capacity_ = capacity;
  mesg_size_ = 0;
  buffer_ = out_buffer;
  args_index_ = 0;
  append_string_and_pad(addr);// insert address
  buffer_[mesg_size_] = ',';
  mesg_size_++; // add , to begin type tag string
  append_string_and_pad(typetags);
  args_ = out_buffer+mesg_size_;
  return true;
}



/**
 *  Append an int argument. Use when encoding a message.
 *  @param i the int.
 *  @return true on success.
 */
bool MessageIterator::append_i(int32_t i) {
  // DEBUG("append: "); DEBUG(i); DEBUG("\n");
  copyHTONL(&buffer_[mesg_size_],(char*)&i);
  args_index_++;
  mesg_size_+=4;
  return true;
}

/**
 *  Append a float. Use when encoding a message, order does matter.
 *  @param i the float.
 *  @return true on success.
 *  @see encode()
 */  
bool MessageIterator::append_f(float f) {
  copyHTONL(&buffer_[mesg_size_],(char*)&f);
  args_index_++;
  mesg_size_+=4;
  return true;
}

/**
 *  Append a string. Use when encoding a message, order does matter.
 *  @param s the string.
 *  @return true on success.
 *  @see encode()
 */  
bool MessageIterator::append_s(const char *s) {
  // DEBUG("append "); DEBUG(s); DEBUG("\n");
  append_string_and_pad(s);
  args_index_++;
  return true;
}

/**
 *  Append a blob. Use when encoding a message
 *  @param data the data of the blob.
 *  @param size the size of the blob.
 *  @return true on success.
 *  @see encode()
 */  
bool MessageIterator::append_b(uint8_t *data, int32_t size) {
  // copy the blob TODO:capicity check....
  copyHTONL(&buffer_[mesg_size_],(char *)&(size));	// append size
  mesg_size_+=4;
  memcpy(&buffer_[mesg_size_],data,size);
  mesg_size_+=(int)size;
  // TODO foscAppendDataAndPad(&(args_), blob->size, blob->data);
  args_index_++;
  return true;
}



/***-------------------- BUNDLE ITERATOR ----------------------------------***/

// TODO:this is a copy of the message iterator.
bool BundleIterator::append_data_and_pad(uint8_t *src, uint32_t size) {
  // TODO:make a proper size check. %3 is not correct
  if (capacity_ < (int)size + size_ + (int)(size%3)) return false;
  memcpy(&buffer_[size_],src,size);
  size_+=size;
  pad();
  return true;
}
// TODO:this is a copy of the message iterator.
bool BundleIterator::append_string_and_pad(const char *src) {
  // TODO:make a proper size check. %3 is not correct
  if (capacity_ < (size_+(int)strlen(src)+(int)(strlen(src)%3))) return false;
  while ( *src != '\0') {
    buffer_[size_] = *src;
    src++;
    size_++;
  }
  // append 0
  buffer_[size_] = 0; size_++;
  pad();
  return true;
}


/**
 *  Return the timetag of this bundle
 *  @param sec seconds.
 *  @param frac fraction of a second.
 */  
void BundleIterator::timetag(int32_t &sec, int32_t &frac) {
  copyNTOHL((char*)&sec,buffer_+8);
  copyNTOHL((char*)&frac,buffer_+12);
}
/**
 *  Set the timetag of this bundle
 *  @param sec seconds.
 *  @param frac fraction of a second.
 */  
void BundleIterator::set_timetag(int32_t sec, int32_t frac) {
  copyHTONL(buffer_+8,(char*)&sec);
  copyHTONL(buffer_+12,(char*)&frac);
}

/**
 *  Encode an OSC bundle.
 *  @param buffer the output buffer.
 *  @param capacity the output buffer capacity.
 */
bool BundleIterator::encode(char* buffer, int capacity) {
  // TODO: assert alignment
  
  if (capacity < 16) return false;
  
  buffer_ = buffer;
  capacity_ = capacity;
  append_string_and_pad("#bundle");
  // TODO: default time tag ?
	size_ = 16;
  return true;
}

/**
 *  Begin a message within OSC bundle.
 *  @param addr is the OSC address.
 *  @param type_tag is the string with arguments.
 *  @return message Iterator.
 */
bool BundleIterator::begin_message(MessageIterator &mi, char *address, char *typetags) {
	// skip over the size and insert it later
  return mi.encode(buffer_+size_+4,capacity_-size_,address, typetags);
}

/**
 *  End a message within OSC bundle.
 *  @param message the message iterator
 */
void  BundleIterator::end_message(const MessageIterator &mi) {
  // insert the size of the message and append the message.
	uint32_t size;
	size = mi.size() +4;
  copyHTONL((buffer_+size_),(char *)&size); // size
	size_ += size;
}
#ifdef FOU_USE_STD_ARG
// bool add_message(char *addr, char *typetags, ...);
#endif

/**
 *  Begin a bundle within OSC bundle.
 *  @return bundle Iterator.
 */
bool BundleIterator::begin_bundle(BundleIterator &bi) {
  return bi.encode(buffer_+4, capacity_ - size_);
}

/**
 *  End a bundle within OSC bundle.
 *  @param bundle the bundle.
 */
void BundleIterator::end_bundle(BundleIterator &bi) {
  // insert the size of the message and append the bundle.
	uint32_t size;
	size = bi.size() +4;
  copyHTONL((buffer_+size_),(char *)&size); // size
	size_ += size;
  
}

/**
 *  Decode an OSC bundle.
 *  @param buffer the output buffer.
 *  @param size the output buffer size.
 */
bool BundleIterator::decode(char* buffer, int size) {
  if (size < 16) return false;
  buffer_ = buffer;
  capacity_ = size;
  element_ = buffer+16;
  return true;
}

/**
 *  Retrieve an encapsulated message when decoding a bundle. The message 
 *  iterator is initialized for decoding.
 *  @param mi the message iterator
 *  @return true on succes, false on error.
 */
bool BundleIterator::element(MessageIterator &mi) {
  if (element_is_bundle()) return false;
  uint32_t size = 0;
  
  return mi.decode(element_+4, size-4); // TODO HTON??????
}

/**
 *  Retrieve an encapsulated bundle when decoding a bundle. The bundle 
 *  iterator is initialized for decoding.
 *  @param bi the bundle iterator
 *  @return true on succes, false on error.
 */
bool BundleIterator::element(BundleIterator &bi) {
  if (element_is_message()) return false;
  uint32_t size=0;
  return bi.decode(element_+4, size-4); // TODO HTON??????
}

bool BundleIterator::element(char **buffer, int &size) {
  // uint32_t s;
  // TODO::implement
  return false;
}





