#
# build dsm2sql
#
# This file is part of dsm2sql                                       
#                                                                    
# dsm2sql is free software: you can redistribute it and/or           
# modify it under the terms of the GNU General Public License as     
# published by the Free Software Foundation, either version 3 of the 
# License, or (at your option) any later version.                    
#                                                                    
# dsm2sql is distributed in the hope that it will be useful,         
# but WITHOUT ANY WARRANTY; without even the implied warranty of     
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the      
# GNU General Public License for more details.                       
#                                                                    
# You should have received a copy of the GNU General Public License  
# along with this program. If not see <http://www.gnu.org/licenses/>.
#

prefix = /usr

CFLAGS = -Wall -Wextra  -I/usr/include/mysql
OBJ    = src/dm.o  src/rs232.o  src/db.o  src/get_password.o
LIBS   =  -lmariadbclient

all:	src/dsm2sql

src/dsm2sql:	$(OBJ)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDCFLAGS) -o $@ $^  $(LIBS)

src/dm.o:	src/dm.c  src/language.h   src/rs232.h  src/dm.h

src/db.o:	src/db.c  src/language.h   src/rs232.h  src/dm.h

src/rs232.o:	src/rs232.c  src/language.h src/rs232.h

src/get_password.o:	src/get_password.c  src/get_password.h

clean:
	-rm -f src/dsm2sql  src/*.o

install:  src/dsm2sql
	install -D src/dsm2sql \
		$(DESTDIR)$(prefix)/bin/dsm2sql

distclean: clean

uninstall:
	-rm -f $(DESTDIR)$(prefix)/bin/dsm2sql

.PHONY:	all install clean distclean uninstall
