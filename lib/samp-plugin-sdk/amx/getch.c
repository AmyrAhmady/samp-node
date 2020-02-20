/* Extremely inefficient but portable POSIX getch() */
#ifndef WIN32

#include <stdio.h>
#include <string.h>
#include <termios.h>    /* for tcgetattr() and tcsetattr() */
#include <unistd.h>     /* for read() */
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include "getch.h"

#ifndef STDIN_FILENO
# define STDIN_FILENO 0
#endif

int
getch (void)
{
  struct termios save_termios;
  struct termios ios;
  int c = 0;

  if (!isatty (STDIN_FILENO))
    return EOF;

  if (tcgetattr (STDIN_FILENO, &save_termios) < 0)
    return EOF;

  ios = save_termios;
  ios.c_lflag &= ~(ICANON | ECHO | ISIG);
  ios.c_cc[VMIN] = 1;           /* read() will return with one char */
  ios.c_cc[VTIME] = 0;          /* read() blocks forever */

  if (tcsetattr (STDIN_FILENO, TCSANOW, &ios) < 0)
    return EOF;

  if (read (STDIN_FILENO, &c, 1) != 1)
    c = EOF;

  tcsetattr (STDIN_FILENO, TCSANOW, &save_termios);

  return c;
}

int
kbhit (void)
{
  struct termios save_termios;
  struct termios ios;
  fd_set inp;
  struct timeval timeout = {0, 0};
  int result;

  if (!isatty (STDIN_FILENO))
    return 0;

  if (tcgetattr (STDIN_FILENO, &save_termios) < 0)
    return 0;

  ios = save_termios;
  ios.c_lflag &= ~(ICANON | ECHO | ISIG);
  ios.c_cc[VMIN] = 1;           /* read() will return with one char */
  ios.c_cc[VTIME] = 0;          /* read() blocks forever */

  if (tcsetattr (STDIN_FILENO, TCSANOW, &ios) < 0)
    return 0;

  /* set up select() args */
  FD_ZERO(&inp);
  FD_SET(STDIN_FILENO, &inp);

  result = select (STDIN_FILENO+1, &inp, NULL, NULL, &timeout) == 1;

  tcsetattr (STDIN_FILENO, TCSANOW, &save_termios);

  return result;
}

#endif
