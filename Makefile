PROG = video2gif
PROG_SRC = $(PROG).c
PROG_RES = $(PROG).rc
PROG_OBJ_SRC = $(PROG).o
PROG_OBJ_RES = $(PROG)_rc.o
PROG_EXE = bin/$(PROG)

CC = gcc
#-lavformat -lavdevice -lavfilter -lavcodec -lavutil -pthread -ldl -lpostproc -lswresample -lswscale -lbz2 -lz -lm
CLIBSFLAGS = -lavformat -lavfilter -lavcodec -lavutil -lswscale -lbcrypt -pthread -static
CFLAGS = -Iinclude/
LDFLAGS = -Llib/

all: $(PROG_EXE)
$(PROG_EXE): $(PROG_OBJ_SRC) $(PROG_OBJ_RES)
	$(CC) $(PROG_OBJ_SRC) $(PROG_OBJ_RES) -o $(PROG_EXE) $(LDFLAGS) $(CLIBSFLAGS)
$(PROG_OBJ_SRC): $(PROG_SRC)
	$(CC) -c $(PROG_SRC) -o $(PROG_OBJ_SRC) $(CFLAGS)
$(PROG_OBJ_RES): $(PROG_RES)
	windres -i $(PROG_RES) -o $(PROG_OBJ_RES)

clean:
	rm $(PROG_EXE) $(PROG_OBJ_SRC) $(PROG_OBJ_RES)