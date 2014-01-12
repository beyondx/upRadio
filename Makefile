TGT := server client
DIR := $(shell pwd)

all:
	for dir in $(TGT); do \
		make -C src/$$dir ; done

.PHONY: clean all

clean:
	for dir in ${TGT}; do\
		make -C src/$$dir clean; done

