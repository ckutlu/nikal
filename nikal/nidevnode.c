/*
 * nidevnode - tool called directly by the kernel to construct
 *             device node in /dev
 *
 * Copyright (c) 2012 National Instruments Corporation.
 * All Rights Reserved.
 */
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/sysmacros.h>

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

static void recurse_mkdir(char *path)
{
   char *p;

   p = strrchr(path, '/');

   if (!p || p == path)
      return;

   *p = '\0';

   recurse_mkdir(path);

   mkdir(path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

   *p = '/';
}

static int __notify_add_event(int argc, char **argv)
{
   int maj, min;
   dev_t dev;

   if (argc < 4) {
      fprintf(stderr, "add <path> <maj> <min>\n");
      return EXIT_FAILURE;
   }

   maj = atoi(argv[2]);
   min = atoi(argv[3]);
   dev = makedev(maj, min);

   recurse_mkdir(argv[1]);

   return mknod(argv[1], S_IFCHR | S_IRUSR | S_IWUSR
                                 | S_IRGRP | S_IWGRP
                                 | S_IROTH | S_IWOTH, dev);
}

static int notify_add_event(int argc, char **argv)
{
   /* When we're making new directory, the directory might be destroyed by
    * something else so do this below in a loop, till we succeed */
   int err, watchdog = 8;

   do {
      err = __notify_add_event(argc, argv);
   } while (err && --watchdog);

   return err;
}

static int notify_remove_event(int argc, char **argv)
{
   if (argc < 2) {
      fprintf(stderr, "remove <path>\n");
      return EXIT_FAILURE;
   }

   return unlink(argv[1]);
}

int main(int argc, char *argv[])
{
   int i;
   static struct {
      const char name[16];
      int (*fn)(int, char **);
   } commands[] = {
      {    "add", notify_add_event    },
      { "remove", notify_remove_event },
   };

   if (argc < 3) {
      fprintf(stderr, "Usage: %s command devnodepath [major] [minor]\n", argv[0]);
      return EXIT_FAILURE;
   }

   /* Set umask() to 0, so that I can just specify exactly what the desired
    * permissions on the node are (and they won't be masked away by the
    * process's umask */
   umask(0);

   for (i = 0; i < ARRAY_SIZE(commands); i++)
      if (!strcmp(argv[1], commands[i].name))
         return commands[i].fn(--argc, ++argv);

   return EXIT_FAILURE;
}
