#
#	Makefile -- Build instructions for user level apps
#


#
# Include architecture specific build rules.
#


DIRS = wmac

all:
	for i in $(DIRS) ; do make -C $$i $@ || exit $?; done


