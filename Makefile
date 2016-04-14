.PHONY: all .FORCE clean

all: cmd/errmark

cmd/errmark: .FORCE
	cd libcscript && make
	cd liberrmark && make
	cd cmd && make

clean:
	cd liberrmark && make clean
	cd libcscript && make clean
	cd cmd && make clean

.FORCE:

show-targets:
	@show-makefile-targets

show-%:
	@echo $*=$($*)
