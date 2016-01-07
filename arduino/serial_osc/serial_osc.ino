#include "slip.h"
#include "fosc.h"
#include "fosc_print.h"

static char test_message[] = 
	{47,102,111,111,
	 47,98,97,114,		
	 98,105,101,0,		
	 44,105,102,115,		// type
	 0,0,0,0,
	 0,0,0,123,				// int
	 67,228,100,254,		// float
	 116,101,115,116,		// string
	 0,0,0,0};				

static char test_message2[] = 
	{47,102,111,111,
	47,98,97,114,
	98,105,101,0,
	44,115,0,0,
	116,101,115,
	116,53,0,0,
	0};

static char blob_data[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17};

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  
  Serial.println("start");
  
  const int buffer_size = 1024;
  
  char buf[buffer_size];
  
  
  
  fou::osc::MessageIterator mi;
  
  mi.encode(buf, buffer_size, "/foo/", "fisb");
  mi.append_f(12.34);
  mi.append_i(129);
  mi.append_s("daniel");
  mi.append_b( (uint8_t*)&(blob_data[0]) , 25);
  
  printMessage(&(buf[0]), 1024);

  Serial.println();

  printMessage(&(test_message[0]), 9*4);
  
}

void loop() {
  // put your main code here, to run repeatedly:
//  Serial.println("...");

}
