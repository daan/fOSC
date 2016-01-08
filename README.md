# fOSC
a very minimal Open Sound Control library for embedded use. No address parsing.

Revived from 2010! out of the infamous FOU project. Hence the name f. It used to run on the MBed platform, but this revived version is only tested with Arduino. 


simple example of encoding a message with the message iterator.

```c++
 const int buffer_size = 1024;
 char buf[buffer_size];
  
 fou::osc::MessageIterator mi;
  
 mi.encode(buf, buffer_size, "/foo/", "fisb");
 mi.append_f(12.34);
 mi.append_i(129);
 mi.append_s("daniel");
 mi.append_b( (uint8_t*)&(blob_data[0]) , blob_size);
```

buffer contains a message of length 

```c++
 int length = mi.size();
``` 

