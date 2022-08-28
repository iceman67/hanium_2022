#
# falinux 라이블러리를 사용하는 어플용 Makefile
#

CROSS_PREFIX		= 

CP_DIR			= ./

## 사용자 소스
TARGET			= serverTest
SRCS			= server.cpp command_parser.cpp main_func.cpp
#---------------------------------------------------------------------
## 소스레벨의 라이브러리

## 인클루드 디렉토리
DIR_INC			=

INCLUDEDIRS		+= -I.
LIVDIRS			+= 

OBJS			= $(SRCS:.cpp=.o)

CFLAGS			= $(INCLUDEDIRS) -o -W -Wall -O2
LDFLAGS			= $(LIVDIRS)
LIBS			= -lpthread

#---------------------------------------------------------------------
CC			= $(CROSS_PREFIX)-gcc
CXX			= $(CROSS_PREFIX)-g++
AR			= $(CROSS_PREFIX)-ar rc
AR2			= $(CROSS_PREFIX)-ranlib
RANLIB			= $(CROSS_PREFIX)-ranlib
LD			= $(CROSS_PREFIX)-ld
NM			= $(CROSS_PREFIX)-nm
STRIP			= $(CROSS_PREFIX)-strip
OBJCOPY		 	= $(CROSS_PREFIX)-objcopy
CP	= cp
MV	= mv
#--------------------------------------------------------------------

all : $(TARGET)


$(TARGET) : $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) -o $@ $(LIBS) -g
	$(NM) $(TARGET) > $(TARGET).map

%.o:%.c
	@echo "Compiling $< ..."
	$(CC) -c $(CFLAGS) -o $@ $<

%.o:%.cc
	@echo "C++ compiling $< ..."
	$(CXX) -c $(CFLAGS) $(CXXFLAGS) -o $@ $<

dep :
	$(CXX)	-M	$(INCLUDEDIRS)	$(SRCS) > .depend

clean:
	rm -f *.bak
	rm -f *.map
	rm -f *.o
	rm -f $(DIR_COMMON)*.o
	rm -f $(DIR_FANET)*.o
	rm -f $(OBJS)
	rm -f $(TARGET) core

distclean: clean
	rm -rf .depend	

gdb : main_func.cpp
	g++ main_func.cpp -g -lpthread


ifeq (.depend,$(wildcard .depend))
include .depend
endif
