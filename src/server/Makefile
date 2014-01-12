TGT := server
SRCS := main.c upmedia.c ../lib/tbf.c
CFLAGS += -Wall -D_DEBUG -g -I../include -I../lib
DEPS := deps
#TEST := test_server

all: $(TGT)

$(TGT) : $(SRCS:%.c=%.o)
	$(CC) $(CFLAGS) $^ -o $@

%.o : %.c
	$(CC)	$(CFLAGS) -c $<



sinclude $(DEPS)

$(DEPS)	: $(SRCS)
	$(CC) $(CFLAGS)  -MM  $^ > $@

.PHONY: clean all
	
clean:
	$(RM) *.o $(TGT)  $(DEPS) 
	$(RM) -rf *.dSYM
