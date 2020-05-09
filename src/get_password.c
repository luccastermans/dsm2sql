/*====================================================================*/ 
/*                                                                    */
/* This file is part of dsm2sql                                       */
/*                                                                    */
/* dsm2sql is free software: you can redistribute it and/or           */
/* modify it under the terms of the GNU General Public License as     */
/* published by the Free Software Foundation, either version 3 of the */
/* License, or (at your option) any later version.                    */
/*                                                                    */
/* dsm2sql is distributed in the hope that it will be useful,         */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of     */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the      */
/* GNU General Public License for more details.                       */
/*                                                                    */
/* You should have received a copy of the GNU General Public License  */
/* along with this program. If not see <http://www.gnu.org/licenses/>.*/
/*====================================================================*/ 

/*====================================================================*/ 
/*      I N C L U D E S                                               */ 
/*====================================================================*/ 
#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <termios.h>

/*====================================================================*/ 
/*      L O C A L includes                                            */ 
/*====================================================================*/ 
#include "get_password.h"


/*====================================================================*/ 
/*   read password from device                                        */
/*                                                                    */

static char *       /* Return:  zero terminated input buffer          */
  get_password(
FILE *file,         /* file handle                                    */
char *buffer,       /* input buffer                                   */
int length)         /* buffer length                                  */
{
  char inChar;
  int  CharsProcessed= 1;
  int  Offset= 0;
  memset(buffer, 0, length);

  do
  {
    inChar= (char)fgetc(file);

    switch(inChar) {
    case '\b': /* backslash */
      if (Offset)
      {
        /* cursor is always at the end */
        Offset--;
        buffer[Offset]= 0;
      }
      break;
    case '\n':
    case '\r':
      break;
    default:
      buffer[Offset]= inChar;
      if (Offset < length - 2)
        Offset++;
      break;
    }  
  } while (CharsProcessed && inChar != '\n' && inChar != '\r');
  return buffer;
}

/*====================================================================*/ 
/*  Reads password from TTY                                           */
/*                                                                    */
/*  Reads a password from console tty without echoing                 */
/*  it's characters. Input buffer must be allocated by calling        */
/*  function.                                                         */
         
char *                  /* pointer to input buffer                    */
   get_password_tty(
char *prompt,           /* prompt for password                        */
char *buffer,           /* password buffer                            */
int   length)           /* password buffer length                     */
{
  struct termios term_old, 
                 term_new;
  FILE *readfrom;

  if (prompt && isatty(fileno(stderr)))
    fputs(prompt, stderr);

  if (!(readfrom = fopen("/dev/tty", "r")))
    readfrom = stdin;

  /* try to disable echo */
  tcgetattr(fileno(readfrom), &term_old);
  term_new = term_old;
  term_new.c_cc[VMIN]  = 1;
  term_new.c_cc[VTIME] = 0;
  term_new.c_lflag &= ~(ECHO | ISIG | ICANON | ECHONL);
  tcsetattr(fileno(readfrom), TCSADRAIN, &term_new);

  buffer = get_password(readfrom, buffer, length);

  if (isatty(fileno(readfrom)))
    tcsetattr(fileno(readfrom), TCSADRAIN, &term_old);

  fclose(readfrom);

  return buffer;
}

