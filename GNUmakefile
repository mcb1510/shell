prog=shell

ldflags:=-lreadline -lhistory -lncurses

include ../GNUmakefile

objs += deq.o

try: $(objs)
	$(CC) -o $@ $(objs) $(ldflags)

trytest: try
	Test/run

test: $(prog)
	Test/run
