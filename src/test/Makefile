TGT := client_test  tbf_test
CFLAGS += -Wall -D_DEBUG -g -I../include -I../server -pthread 
DEPS := deps

all: $(TGT)

client_test: client_test.c
	$(CC) $(CFLAGS) $^ ../server/tbf.c -o $@

tbf_test: tbf_test.o
	$(CC) $(CFLAGS) $^ ../server/tbf.c -o $@

% : %.c
	$(CC) $(CFLAGS) -c  $<

sinclude $(DEPS)

$(DEPS)	: *.c
	$(CC) $(CFLAGS)  -MM  $^ > $@

.PHONY: clean all
	
clean:
	$(RM) *.o $(TGT) $(DEPS) 
	$(RM) -rf *.dSYM
