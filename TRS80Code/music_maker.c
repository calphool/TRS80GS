#include <stdio.h>
#include <stdlib.h>


void clearTRS80Screen() {
  char * p = (char *)0x3c00;
  for(int i=0;i<1024;i++)
  	{
  		*(p+i) = ' ';
  	}

}


int main()
{
   clearTRS80Screen();
}