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
typedef char           ASCII; 
typedef unsigned char  BYTE; 
typedef short	       BOOL; 
typedef unsigned char  BOOLEAN_8; 
typedef short          WORD; 
typedef unsigned short UWORD; 
typedef long           LONG; 
typedef unsigned long  ULONG; 
 
 
#define MAX(a,b)    ((a) > (b) ? (a) : (b)) 
#define MIN(a,b)    ((a) < (b) ? (a) : (b)) 
 
#if !defined (FALSE)
#define FALSE 0 
#endif

#if !defined (TRUE)
#define TRUE  1 
#endif

#define EOS                '\0'       /* End Of String character */ 
#define NULL_L             0L 
#define NULL_PTR ((void *) 0) 

