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
#include <stdlib.h>
#include <string.h> 
#include <getopt.h>
#include <time.h>


/*====================================================================*/ 
/*      SQLDB includes                                                */ 
/*====================================================================*/ 
#include <mysql.h>

/*====================================================================*/ 
/*      L O C A L includes                                            */ 
/*====================================================================*/ 
#include "language.h" 

#include "rs232.h"
#include "dm.h"
#include "get_password.h"


/*====================================================================*/ 
/*      L O C A L   S Y M B O L   D E C L A R A T I O N S             */ 
/*====================================================================*/ 
#define DEF_HOST_NAME	NULL_PTR  
#define DEF_USER_NAME	NULL_PTR
#define DEF_COMM_NAME	NULL_PTR
#define DEF_PASSWORD	NULL_PTR
#define DEF_PORT_NUM    3306
#define DEF_DB_NAME	NULL_PTR
#define DEF_SOCKET_NAME NULL_PTR


/*====================================================================*/ 
/*      L O C A L   T Y P E   D E F I N I T I O N S                   */ 
/*====================================================================*/ 
typedef struct option  OPTION_STRUCT;


/*====================================================================*/ 
/*      L O C A L   D A T A   D E F I N I T I O N S                   */ 
/*====================================================================*/ 
static ASCII *sections[] =  /* sections to look for in .my.cnf files  */
{
    "client", 
    NULL_PTR
};
static OPTION_STRUCT long_options[] = 
{
  { "host",     required_argument, NULL_PTR, 'h'},
  { "user",     required_argument, NULL_PTR, 'u'},
  { "comm",     required_argument, NULL_PTR, 'c'},
  { "password", optional_argument, NULL_PTR, 'p'},
  { "port",     required_argument, NULL_PTR, 'P'},
  { "socket",   required_argument, NULL_PTR, 'S'},
  { "daemon",   no_argument,       NULL_PTR, 'd'},
  { NULL_PTR,   0,                 NULL_PTR,  0 }
};
static ASCII  progname[100];                 /* name of this program  */
static BOOL   debug = FALSE;                 /* debug mode            */
static ASCII *comm_name_ptr = DEF_COMM_NAME; /* comm port name        */
static ASCII  sys_time[40];                  /* system time           */

/*====================================================================*/ 
/*      L O C A L   F U N C T I O N S                                 */ 
/*====================================================================*/ 
static void   print_error(MYSQL *c_ptr, ASCII  *m_ptr);
static MYSQL *do_connect(ASCII *hn_ptr,ASCII *un_ptr,ASCII *pwd_ptr,
                         ASCII *dbn_ptr,UWORD  pn, ASCII *sn_ptr, 
                         UWORD flags);
static void   do_disconnect(MYSQL  *ptr);
static WORD   process_result_set(MYSQL *c_ptr, MYSQL_RES *res_ptr);
static WORD   process_query(MYSQL *c_ptr, ASCII *q_ptr, 
                            WORD (*fct)(MYSQL *a, MYSQL_RES *b) );
static void   get_time(ASCII *out_ptr);


/*====================================================================*/
/*    get_time                                                        */ 
/*====================================================================*/ 
static void
  get_time(ASCII *out_ptr)       /* DATE+TIME as YYYY-MM-DD HH:MM:SS   */
{
  time_t rawtime;
  struct tm *timeinfo_ptr;

  time(&rawtime);
  timeinfo_ptr = localtime(&rawtime);

  sprintf(out_ptr, "[%d-%02d-%02d %02d:%02d:%02d]",
                   timeinfo_ptr->tm_year + 1900,
                   timeinfo_ptr->tm_mon + 1,
                   timeinfo_ptr->tm_mday,
                   timeinfo_ptr->tm_hour,
                   timeinfo_ptr->tm_min,
                   timeinfo_ptr->tm_sec);
}


/*====================================================================*/
/*    print_error                                                     */ 
/*====================================================================*/ 
static void
    print_error(
MYSQL           *c_ptr,
ASCII           *m_ptr)
{
  fprintf(stderr, "%s\n", m_ptr);
  if (c_ptr != NULL_PTR )
  {
      get_time(sys_time);
      fprintf(stderr, "%s Error %u (%s)\n",
              sys_time,
              mysql_errno(c_ptr), mysql_error(c_ptr));
  }
}


/*====================================================================*/ 
/*    do_connect                                                      */ 
/*====================================================================*/ 
static MYSQL *
    do_connect(
ASCII          *hn_ptr,
ASCII          *un_ptr,
ASCII          *pwd_ptr,
ASCII          *dbn_ptr,
UWORD           pn,
ASCII          *sn_ptr,
UWORD           flags)
{
  MYSQL *c_ptr;
    
  c_ptr = mysql_init(NULL);
  if ( c_ptr == NULL_PTR )
  {
    print_error(NULL_PTR, "mysql_init() failed");
    return NULL_PTR;
  }

  if ( mysql_real_connect(c_ptr, hn_ptr, un_ptr, pwd_ptr,
                          dbn_ptr, pn, sn_ptr, flags) == NULL_PTR )
  {
    print_error(c_ptr, "mysql_real_connect() failed");
    return NULL_PTR;
  }        

  return(c_ptr);
}

    
/*====================================================================*/ 
/*    do_disconnect                                                   */ 
/*====================================================================*/ 
static void
    do_disconnect(
MYSQL             *ptr)
{
  mysql_close(ptr);
}


/*====================================================================*/ 
/*    process_result_set                                              */ 
/*====================================================================*/ 
static WORD
    process_result_set(
MYSQL                 *c_ptr,
MYSQL_RES             *res_ptr)
{
  MYSQL_ROW  row;
  UWORD      i;
    
  while (( row = mysql_fetch_row(res_ptr)) != NULL_PTR )
  {
    for ( i = 0; i < mysql_num_fields(res_ptr); i++ )
    {
      if ( i > 0 )
        fputc('\t', stdout);
      printf("%s", row[i] != NULL_PTR ? row[i] : "NULL");
    }
    fputc('\n', stdout);
  }
  
  if ( mysql_errno(c_ptr) != 0 )
    print_error(c_ptr, "mysql_fetch_row() failed");
  else
  {
    get_time(sys_time);
    printf("%s %lu rows returned\n", sys_time, (ULONG) mysql_num_rows(res_ptr));
  }
  return 1;
};


/*====================================================================*/ 
/*    process_query                                                   */ 
/*====================================================================*/ 
static WORD
    process_query(
MYSQL             *c_ptr,
ASCII             *q_ptr,
BOOL              (*fct)(MYSQL *a, MYSQL_RES *b))
{
  MYSQL_RES *res_ptr;
  BOOL       retval;
    
  if ( mysql_query(c_ptr, q_ptr) != 0 )
  {
    print_error(c_ptr, "process_query() failed");
    fprintf(stderr, "QRY: %s\n", q_ptr);
    return 0;
  }
    
  res_ptr = mysql_store_result(c_ptr);
  if ( res_ptr == NULL_PTR )
  {
    if ( mysql_field_count(c_ptr) > 0 )
      /*=====================================================*/
      /* a result set was expected, by mysql_store_result () */
      /* did not return one' this means an error occured     */
      /*=====================================================*/
      print_error(c_ptr, "Problem processing result set");
    else
    {
      /*=====================================================*/
      /* no result set was returned; query returned no data  */
      /* (it was not a SELECT, SHOW, DESCRIBE, or EXPLAIN(,  */
      /* so just report number of rows affected by query     */
      /*=====================================================*/
      get_time(sys_time);
      printf("%s %lu rows affected\n",
             sys_time,
             (unsigned long) mysql_affected_rows(c_ptr));
    }
  }
  else
  {
    retval = (*fct)(c_ptr, res_ptr);
    mysql_free_result(res_ptr);
  }
  return retval;
}


/*====================================================================*/ 
/*    print_usage                                                     */ 
/*====================================================================*/ 
void print_usage()
{
  printf("Usage: %s [-h host] - [u username] [-P port] [-ppassword] "\
         "-c comport [-d] database\n", progname);
}

/*====================================================================*/
/* init()                                                          */
/*====================================================================*/ 
MYSQL *                      /* ptr to SQLDB or NULL_PTR if failed    */
      init(
int      argc,               /* number of commanline arguments        */ 
ASCII   *argv[])             /* the commanline arguments              */ 
{
  ASCII *host_name_ptr   = DEF_HOST_NAME;
  ASCII *user_name_ptr   = DEF_USER_NAME;
  ASCII *password_ptr    = DEF_PASSWORD;
  UWORD  port_num        = DEF_PORT_NUM;
  ASCII *socket_name_ptr = DEF_SOCKET_NAME;
  ASCII *db_name_ptr     = DEF_DB_NAME;
  ASCII  passbuf[100];
  BOOL   ask_password    = FALSE;
  int    c;
  
  strncpy(progname, argv[0], 90);
   
  while (TRUE)
  {
    int option_index = 0;
    
    c = getopt_long(argc, argv, "h:p::u:P:dc:",
                    long_options, &option_index);
    if (c == -1)
      break;
    
    switch (c)
    {
      case 'd':
        debug = TRUE;
        break;
      case 'c':
        comm_name_ptr = optarg;
        break;
      case 'h':
        host_name_ptr = optarg;
        break;
      case 'u':
        user_name_ptr = optarg;
        break;
      case 'S':
        socket_name_ptr = optarg;
        break;      
      case 'p':
        if (!optarg)
        {
          ask_password = TRUE;
        }
        else
        {
          strncpy(passbuf, optarg, sizeof(passbuf)-1);
          passbuf[sizeof(passbuf)-1] = EOS;
          password_ptr = passbuf;

          while (*optarg)
            *optarg++ = ' ';
        }
        break;
      case 'P':
        port_num = (UWORD) atoi(optarg);
        break;
      default:
        print_usage(); 
        return NULL_PTR;
      }
  }
    
  /* advance past the argument that were processed by getopt_long */
  /* now database name is expected!!                              */
  
  argc -= optind;
  argv += optind;
    
  if (argc > 0 )
  {
    db_name_ptr = argv[0];
    --argc;
    ++argv;
  }
   
  if (ask_password)
      password_ptr = get_password_tty("", passbuf, 100);

  if ( comm_name_ptr == NULL_PTR )
  {
    get_time(sys_time);
    fprintf(stderr, "%s %s: comport missing\n", sys_time, progname);
    print_usage();
    exit(1);
  }
    
  if ( db_name_ptr == NULL_PTR )
  {
    get_time(sys_time);
    fprintf(stderr, "%s %s: database missing\n", sys_time, progname);
    print_usage();
    exit(1);
  }
  
  return do_connect(host_name_ptr, user_name_ptr, 
                    password_ptr, db_name_ptr, 
                    port_num, socket_name_ptr, 0);
}


/*====================================================================*/ 
/*   Returns portnr if it  can be opened, -1 otherwise                */
/*                                                                    */
int  open_comport(                    /* comport kan geopend worden   */
                   ASCII port[],      /* RS232 poort om van te lezen  */
                   int   bdrate,      /* baudrate                     */
                   ASCII  mode[])     /* RS232 mode                   */
{
  int comport = -1; /* COM port nr */

  comport = RS232_GetPortnr(port);
  if (RS232_OpenComport(comport, bdrate, mode))
  {
    printf("Cannot open COM-port %d.\n", comport);
    return -1;
  }
  else
  {
    RS232_CloseComport(comport);
    return comport;
  }
}


/*====================================================================*/
/* main                                                               */
/*====================================================================*/ 
int
    main(
int      argc,
ASCII   *argv[])
{
  MYSQL *sqldb_ptr = NULL_PTR;   /* the SQL connection */

  mysql_library_init(argc, argv, sections);

  sqldb_ptr = init(argc, argv);

  if (sqldb_ptr != NULL_PTR && 
      comm_name_ptr != NULL_PTR &&
      open_comport(comm_name_ptr, 115200, "8N1") >= 0 )
  {  
    DM_init();
  
    if (!DM_rcv_telegram(comm_name_ptr))
    {
      sleep(1);  /* sleep 1 second */
      
      if (!DM_rcv_telegram(comm_name_ptr))
      {
        get_time(sys_time);

        fprintf(stderr, "%s no telegram received after 2 tries\n", sys_time);
        return 1; 
      }
    }
 
    if (debug == TRUE)
    {
      get_time(sys_time);
      fprintf(stderr, "%s", DM_get_telegram());
    }

    DM_telegram2qry(); 
  
    if (debug == TRUE)
    {
      get_time(sys_time);
      printf("%s %s\n", sys_time, DM_get_qry());
    }
    else
      process_query(sqldb_ptr, DM_get_qry(), process_result_set);
    
    do_disconnect(sqldb_ptr);
  }
  else
  {
    printf("No SQL DB or comport name or could not open comport.\n");
  }

    
  mysql_library_end();
  
  exit (0);
}


