#
# http://www.gnu.org/software/make/manual/make.html
#
CC:=gcc
INCLUDES:=$(shell pkg-config --cflags libavformat libavcodec libswscale libavutil)
CFLAGS:=-Wall -ggdb
LDFLAGS:=$(shell pkg-config --libs libavformat libavcodec libswscale libavutil) -lm -lrt
EXE:=test1.out test2.out transcodetoh264mp4.out transcodetovp8rtp.out


#
# This is here to prevent Make from deleting secondary files.
#
.SECONDARY:


#
# $< is the first dependency in the dependency list
# $@ is the target name
#
all: dirs obj/io.o obj/codec.o $(addprefix bin/, $(EXE)) tags

dirs:
	mkdir -p obj
	mkdir -p bin

tags: *.c
	ctags *.c

bin/%.out: obj/%.o
	$(CC) $(CFLAGS) $< $(LDFLAGS) obj/io.o obj/codec.o -o $@

obj/%.o : %.c
	$(CC) $(CFLAGS) $< $(INCLUDES) -c -o $@

clean:
	rm -f obj/*
	rm -f bin/*
	rm -f tags

