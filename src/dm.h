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
/*    P R O T O T Y P E S                                             */
/*====================================================================*/
void   DM_telegram2qry(void);
ASCII *DM_get_qry(void);
ASCII *DM_get_telegram(void);
BOOL   DM_rcv_telegram(ASCII port[]);
void   DM_init(void);
