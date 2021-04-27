
all: t3-procmem/process_memory t1-timeout/parallel_min_max
	

t3-procmem/process_memory:
	gcc t3-procmem/process_memory.c -o t3-procmem/process_memory

t1-timeout/parallel_min_max:
	cd t1-timeout
	make -f t1-timeout/makefile
	cd ..
