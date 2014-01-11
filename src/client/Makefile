TGT := client
SRCS := main.c
CFLAGS += -Wall -D_DEBUG -g -I../include -pthread 
DEPS := deps
TEST := test_server

all: $(TGT) $(TEST)

$(TGT) : $(SRCS:%.c=%.o)
	$(CC) $(CFLAGS) $^ -o $@

%.o : %.c
	$(CC) $(CFLAGS) -c $<

$(TEST) : 
	$(CC)  $(CFLAGS) test_server.c  ../lib/tbf.c -o $@

sinclude $(DEPS)

$(DEPS)	: $(SRCS)
	$(CC) $(CFLAGS)  -MM  $^ > $@

.PHONY: clean all $(TEST)
	
clean:
	$(RM) *.o $(TGT) $(TEST) $(DEPS) 
	$(RM) -rf *.dSYM
