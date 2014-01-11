TGT := client_test  tbf_test
CFLAGS += -Wall -D_DEBUG -g -I../include -I../lib -pthread 
DEPS := deps

all: $(TGT)

client_test: client_test.c
	$(CC) $(CFLAGS) $^ ../lib/tbf.c -o $@

tbf_test: tbf_test.o
	$(CC) $(CFLAGS) $^ ../lib/tbf.c -o $@

% : %.c
	$(CC) $(CFLAGS) -c  $<

sinclude $(DEPS)

$(DEPS)	: *.c
	$(CC) $(CFLAGS)  -MM  $^ > $@

.PHONY: clean all $(TEST)
	
clean:
	$(RM) *.o $(TGT) $(DEPS) 
	$(RM) -rf *.dSYM
