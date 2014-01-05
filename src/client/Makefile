TGT := client
SRCS := main.c
CFLAGS += -Wall -D_DEBUG -g -I../include 
DEPS := deps
TEST := test_server

all: $(TGT) $(TEST)

$(TGT) : $(SRCS:%.c=%.o)
	$(CC) $(CFLAGS) $^ -o $@

%.o : %.c
	$(CC)	$(CFLAGS) -c $<



sinclude $(DEPS)

$(DEPS)	: $(SRCS)
	$(CC) $(CFLAGS)  -MM  $^ > $@

.PHONY: clean all
	
clean:
	$(RM) *.o $(TGT) $(TEST) $(DEPS)
