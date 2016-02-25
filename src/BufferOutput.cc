//BufferOutput.cc
#include "CreateTreeFromFilter.h"
#include "BufferOutput.h" 

void BufferOutput(FILE *fout, const char*a, int len ){
   //defined static so that it doesn't get inizialized every time
   static char buf[B_LEN]; 
   static int count = 0 ;

   // empties the buffer if there is no "a" or is buffer is full or if len == 0
   if (count + len+1>= B_LEN || a==0 || len==0){
      //fwrite(buf, 1, count, fp);
      fwrite(buf, 1, count, fout);
      count = 0 ; 
   }
   memcpy(buf+count,a, len);
   buf[len+count]='\n';
   count+= len +1; 
}
