#include <stdio.h>
#include <linux/input.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <dirent.h>

static int touchX=0;
static int touchY=0;

#define TOUCH_EVENT_START 1
#define TOUCH_EVENT_END   2
#define TOUCH_EVENT_MOVE   3

void runTouch(char *touch_path, void (*touch_callback)(int type, int x, int y));
int readTouch(int touch_fd);

#define INOTIFY_FD_BUFFER_LENGTH        (64 * (sizeof(struct inotify_event) + NAME_MAX + 1))

void runTouch(char *touch_path, void (*touch_callback)(int type, int x, int y))
{
  int inotify_fd, r;
  int touch_fd, touch_type;

  /* Check that file_path exists */
  struct stat file_st;
  if(stat(touch_path, &file_st) < 0)
  {
    fprintf(stderr, "inotify: File path does not exist: %s\n", touch_path);
    return;
  }

  /* Set up inotify */
  inotify_fd = inotify_init();
  if(inotify_fd == -1)
  {
    fprintf(stderr, "inotify: inotify_init() returned error: %s\n", strerror(inotify_fd));
    return;
  }

  /* Start watching file for changes */
  r = inotify_add_watch(inotify_fd, touch_path, IN_ACCESS);
  if(r < 0)
  {
    fprintf(stderr, "inotify: inotify_add_watch() returned error: %s\n", strerror(r));
    return;
  }

  /* Open touch file for data read */
  touch_fd = open(touch_path, O_RDONLY);
  if(touch_fd < 0)
  {
    fprintf(stderr, "inotify: open() touch file returned error: %s\n", strerror(touch_fd));
    return;
  }

  char *p;
  ssize_t pending_length;
  struct inotify_event *event;
  char buf[INOTIFY_FD_BUFFER_LENGTH] __attribute__ ((aligned(8)));
  while (1)
  {
    /* Wait for new files */
    pending_length = read(inotify_fd, buf, INOTIFY_FD_BUFFER_LENGTH);
    if (pending_length <= 0)
    {
      continue;
    }

    /* Process buffer of new files */
    for (p = buf; p < buf + pending_length; )
    {
      event = (struct inotify_event *) p;

      /* Pass filename to supplied callback */
      touch_type = readTouch(touch_fd);
      touch_callback(touch_type, touchX, touchY);

      p += sizeof(struct inotify_event) + event->len;
    }
  }

  close(touch_fd);
  close(inotify_fd);
}

//Returns 0 if no touch available. 1 if Touch start. 2 if touch end. 3 if touch move
int readTouch(int touch_fd)
{
  size_t i, rb;
  struct input_event ev[64];
  int retval;

  retval = -1;

  rb=read(touch_fd, ev, sizeof(struct input_event)*64);

  for (i = 0;  i <  (rb / sizeof(struct input_event)); i++)
  {
    if (ev[i].type ==  EV_SYN) 
    {
      if(retval == -1)
      {
        retval = TOUCH_EVENT_MOVE;
      }
      break;  //action
    }
    else if (ev[i].type == EV_KEY && ev[i].code == 330 && ev[i].value == 1)
    {
      retval = TOUCH_EVENT_START;
    }
    else if (ev[i].type == EV_KEY && ev[i].code == 330 && ev[i].value == 0)
    {
      retval = TOUCH_EVENT_END;
    }
    else if (ev[i].type == EV_ABS && ev[i].code == 0 && ev[i].value > 0)
    {
      touchX = ev[i].value;
    }
    else if (ev[i].type == EV_ABS  && ev[i].code == 1 && ev[i].value > 0)
    {
      touchY = ev[i].value;
    }
  }

  return retval;
}