# This Makefile will build the GenUI application.

VPATH = ./src ./src/telnet ./res

HEADERS = .resource.h
OBJS = main.o  resource.o
INCLUDE_DIRS = -I../../../usr/include/libxml2

WARNS = -Wall

CC = i686-pc-mingw32-gcc.exe
CFLAGS = -O3 -std=c99  -D _WIN32_IE=0x0500 -D WINVER=0x500 -D _WINDOWS ${WARNS} `xml2-config --cflags`  
LDFLAGS = -s -lcomctl32 -Wl,--subsystem,windows -L/usr/lib
RC = i686-pc-mingw32-windres.exe

all: GenUI.exe

%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} ${INCLUDE_DIRS} -c $< -o $@ 

resource.o: resource.rc Application.manifest Application.ico resource.h
	${RC} -I. -i $< -o $@

GenUI.exe: ${OBJS}
	${CC} -o "$@" ${OBJS} `xml2-config --libs` -ladvapi32 -lcomctl32 -lcomdlg32 -lgdi32 -lwsock32 -lws2_32  \
		-limm32 -lole32 -lshell32 -luser32 -lwinmm -lwinspool -lxml2 ${LDFLAGS} 

clean:
	rm *.o "GenUI.exe"
