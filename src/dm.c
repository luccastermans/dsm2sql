/*====================================================================*/ 
/*                                                                    */
/* Author: Luc Castermans                                             */
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
#include <string.h>
#include <unistd.h>

#include "language.h"
#include "rs232.h"

#include "dm.h"

/*====================================================================*/
/*      L O C A L   S Y M B O L   D E C L A R A T I O N S             */
/*====================================================================*/
#define INTELEGRAM  1
#define OUTTELEGRAM 0
#define CRC16       0x8005
#define NBRLINES    50
#define TELSIZE     1024

/*====================================================================*/
/*      L O C A L   D A T A   D E F I N I T I O N S                   */
/*====================================================================*/
static ASCII  telegram[TELSIZE];       /* Dutch Smart Meter telegram  */
static ASCII *telegram_ptr[NBRLINES];  /* Lines in the telegram       */
static ASCII  qry[TELSIZE];            /* SQL query                   */

/*====================================================================*/
/*      L O C A L   F U N C T I O N S   P R O T O T Y P E S           */
/*====================================================================*/



/*====================================================================*/ 
/* Convert telegram to lines of text:                                 */
/*     "a b c d LF CR"   =>   "a b c d \0 \0"                         */    
/*     "e f g h LF CR"   =>   "e f g h \0 \0"                         */
/*     etc.                                                           */    
/*  and compensate for lines with only LF CR, i.e. empty lines        */
/*====================================================================*/ 
static void telegram2lines(void)
{
  int i;       /* index in telegram[]     */
  int j = 0;   /* index telegram_ptrtr[]  */
  int len;     /* length of telegram[]    */

  len = strlen(telegram);
  telegram_ptr[j++] = &telegram[0];  /* de 1ste regel! */ 
  
  for ( i = 1 ; i < len; i++ )
  {
    if ( telegram[i] == '\r' )            /* LF found            */
    {
      telegram[i++] = EOS;                /* LF -> '\0'          */
      telegram[i++] = EOS;                /* CR -> '\0'          */
      if ( telegram[i] != '\r' )
        telegram_ptr[j++] = &telegram[i]; /* store start of line */
      else
        i--;                              /* 'empty'-line        */
    }
  }
}


/*====================================================================*/ 
/* calculate CRC-16:                                                  */
/*     http://www.lammertbies.nl/comm/info/nl_crc-calculation.html    */
/* Algorithme: http://stackoverflow.com/questions/21939392/           */
/*             crc-16-program-to-calculate-check-sum                  */
/*====================================================================*/ 
static UWORD gen_crc16(ASCII *data, int size)
{
    UWORD out = 0;
    int bits_read = 0, bit_flag;

    while(size > 0)
    {
        bit_flag = out >> 15;

        /* Get next bit: */
        out <<= 1;
        out |= (*data >> bits_read) & 1; 

        /* Increment bit counter: */
        bits_read++;
        if(bits_read > 7)
        {
            bits_read = 0;
            data++;
            size--;
        }

        /* Cycle check: */
        if(bit_flag)
            out ^= CRC16;

    }

    // item b) "push out" the last 16 bits
    int i;
    for (i = 0; i < 16; ++i) {
        bit_flag = out >> 15;
        out <<= 1;
        if(bit_flag)
            out ^= CRC16;
    }

    // item c) reverse the bits
    UWORD crc = 0;
    i = 0x8000;
    int j = 0x0001;
    for (; i != 0; i >>=1, j <<= 1) {
        if (i & out) crc |= j;
    }

    return crc;
}


/*====================================================================*/ 
/*                                                                    */
BOOL DM_rcv_telegram(                 /* valid telegram received?     */
                     ASCII port[])    /* read from this RS232 port    */
/*====================================================================*/ 
{
  int   comport   = -1;               /* COM port nr            */
  BYTE  buf[4096];                    /* RS232 ontvangst buffer */
  int   i;                            /* index in buf[]         */
  int   n;                            /* #received characters   */
  int   t = 0;                        /* index in telegram[]    */
  UWORD crc_cal;                      /* calculated CRC         */
  UWORD crc_rcv;                      /* received CRC           */

  comport = RS232_GetPortnr(port);
  if (RS232_OpenComport(comport, 115200, "8N1"))
  {
    printf("Cannot open COM-port %d.\n", comport);
    return FALSE;
  }
  
  while (TRUE)
  {
    static BOOL state = OUTTELEGRAM; 

    n = RS232_PollComport(comport, buf, 4095);
    if (n > 0)
    {
      buf[n] = EOS;

      for (i = 0; i < n; i++)
      {
        if (buf[i] == '/' )
        {
          state = INTELEGRAM;
        }
        else if ( buf[i] == '!' )
        {
          strcpy((ASCII *) &telegram[t], (ASCII *) buf);
          t += n;
          state = OUTTELEGRAM;
          goto telegram_received;
        }
      }

      if ( state == INTELEGRAM )
      {
        strcpy((ASCII *) &telegram[t], (ASCII *) buf);
        t += n;
      }
    }

    usleep(100000);  /* sleep for 100 milliSeconds */
  }

  telegram_received:

  RS232_CloseComport(comport);

  /*=====================================*/
  /* calc CRC of the telegram,           */
  /* read the CRC from received telegram */ 
  /* and compare the CRC´s               */
  /*=====================================*/

  crc_cal = gen_crc16((ASCII *)&telegram[0], strlen(telegram)-6);
  sscanf(&telegram[strlen(telegram)-6],"%hx", &crc_rcv);
  
  return ( crc_cal == crc_rcv ) ? TRUE : FALSE;
}


/*====================================================================*/ 
/* convert date to MySQL format                                       */
static void date2mysql(
                ASCII *in_ptr,         /* input:  YYMMDDhhmmssX       */
                ASCII *out_ptr)        /* output: YYYY-MM-DD HH:MM:SS */
/*====================================================================*/ 
{
    
    strcpy(&out_ptr[0], "20");              /* 20 */
    strncat(&out_ptr[2], &in_ptr[0], 2);    /* YY */
    out_ptr[4] = '-';
    strncat(&out_ptr[5], &in_ptr[2], 2);    /* MM */
    out_ptr[7] = '-';
    strncat(&out_ptr[8], &in_ptr[4], 2);    /* DD */

    out_ptr[10] = ' ';
    
    strncat(&out_ptr[11], &in_ptr[6], 2);   /* hh */
    out_ptr[13] = ':';
    strncat(&out_ptr[14], &in_ptr[8], 2);   /* hh */
    out_ptr[16] = ':';
    strncat(&out_ptr[17], &in_ptr[10], 2);  /* hh */
}


/*====================================================================*/ 
/*                                                                    */
static void clear_str(ASCII *ptr,     /* string to clear              */
                      int    len)     /* length of string             */
/*====================================================================*/ 
{
  int i;
  
  for ( i = 0; i < len; i++ )
    *ptr++ = EOS;
}


/*====================================================================*/ 
/* initialise global variabels                                        */
/*====================================================================*/ 
void DM_init(void)
{
  int i;
  
  for ( i = 0; i < NBRLINES; i++ )
    telegram_ptr[i] = NULL_PTR;
  
  clear_str(telegram, TELSIZE);
  clear_str(qry, TELSIZE);
}


/*====================================================================*/ 
/*                                                                    */
/*====================================================================*/ 
static void lines2qry(void)
{
  int i;
  ASCII dat_in[20];
  ASCII dat_out[20];
  ASCII h[7], d[5];

  strcat(qry, "INSERT INTO emeter "\
                 "(e_dattijd,"\
                 " e_ver_t1,e_ver_t2,e_ter_t1,e_ter_t2,t,"\
                 " e_ver_mom,e_ter_mom,g_dattijd,g_ver) "\
                 "VALUES (");
 
  for ( i = 0;  telegram_ptr[i] != NULL_PTR; i++ )
  {
    clear_str(dat_in, 20);
    clear_str(dat_out, 20);
    clear_str(h, 7);
    clear_str(d, 5);

    if ( sscanf(telegram_ptr[i], "0-0:1.0.0(%12sS)", dat_in) == 1 ||
         sscanf(telegram_ptr[i], "0-0:1.0.0(%12sW)", dat_in) == 1    )
    {
      date2mysql(dat_in, dat_out);
    
      strcat(qry, "\"");
      strcat(qry, dat_out);
      strcat(qry, "\",");
    }
    else if ( sscanf(telegram_ptr[i], "0-0:96.14.0(%4s)", h) == 1 )
    {
      strcat(qry, h);
      strcat(qry, ",");
    }
    else if ( sscanf(telegram_ptr[i], "1-0:1.8.1(%6s.%3s*kWh)", h, d) == 2 )
    {
      strcat(qry, h);
      strcat(qry, ".");
      strcat(qry, d);
      strcat(qry, ",");
    }
    else if ( sscanf(telegram_ptr[i], "1-0:1.8.2(%6s.%3s*kWh)", h, d) == 2 )
    {
      strcat(qry, h);
      strcat(qry, ".");
      strcat(qry, d);
      strcat(qry, ",");
    }
    else if ( sscanf(telegram_ptr[i], "1-0:2.8.1(%6s.%3s*kWh)", h, d) == 2 )
    {
      strcat(qry, h);
      strcat(qry, ".");
      strcat(qry, d);
      strcat(qry, ",");
    }
    else if ( sscanf(telegram_ptr[i], "1-0:2.8.2(%6s.%3s*kWh)", h, d) == 2 )
    {
      strcat(qry, h);
      strcat(qry, ".");
      strcat(qry, d);
      strcat(qry, ",");
    }
    else if ( sscanf(telegram_ptr[i], "1-0:1.7.0(%2s.%3s*kW)", h, d) == 2 )
    {
      strcat(qry, h);
      strcat(qry, ".");
      strcat(qry, d);
      strcat(qry, ",");
    }
    else if ( sscanf(telegram_ptr[i], "1-0:2.7.0(%2s.%3s*kW)", h, d) == 2 )
    {
      strcat(qry, h);
      strcat(qry, ".");
      strcat(qry, d);
      strcat(qry, ",");
    }
    else if ( sscanf(telegram_ptr[i], "0-1:24.2.1(%12sS)(%5s.%3s*m3)", dat_in, h, d) == 3 ||
              sscanf(telegram_ptr[i], "0-1:24.2.1(%12sW)(%5s.%3s*m3)", dat_in, h, d) == 3   )
    {
      date2mysql(dat_in, dat_out);
    
      strcat(qry, "\"");
      strcat(qry, dat_out);
      strcat(qry, "\",");
      strcat(qry, h);
      strcat(qry, ".");
      strcat(qry, d);
      strcat(qry, ");");
    }
  }
}


/*====================================================================*/ 
/*                                                                    */
void DM_telegram2qry(
                     void)
/*====================================================================*/ 
{
  if ( strlen(telegram) > 0 )  /* check if telegram receveid ? */
  {
    telegram2lines(); 
    lines2qry();
  }
}  


/*====================================================================*/ 
/*                                                                    */
ASCII *DM_get_qry(
                  void)
/*====================================================================*/ 
{
  return qry;
}  


/*====================================================================*/ 
/*                                                                    */
ASCII *DM_get_telegram(
                       void)
/*====================================================================*/ 
{
  return telegram;
}  
