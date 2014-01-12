TGT := server client
DIR := $(shell pwd)

all:
	for dir in $(TGT); do \
		make -C src/$$dir ; done

.PHONY: clean

clean:
	for dir in ${TGT}; do\
		make -C src/$$dir clean; done

