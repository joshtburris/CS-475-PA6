OBJS = timer.o
H_FILE = timer.h

EXEC = jac jacOMP jacMPI

all: $(EXEC)

jac: jac.c $(OBJS) $(H_FILE)
	gcc -O3 -o $@ jac.c $(OBJS) $(MATHFLAG)

jacOMP: jacOMP.c $(OBJS) $(H_FILE)
	gcc -O3 -fopenmp -o $@ jacOMP.c $(OBJS) $(MATHFLAG)

jacMPI: jacMPI.c $(OBJS) $(H_FILE)
	mpicc -O3 -o $@ jacMPI.c $(OBJS) $(MATHFLAG)

clean:
	rm -f $(EXEC)

tar:
	tar cvf PA6.tar Makefile jacOMP.c jacMPI.c report.pdf

testOMP:
	clear
	for i in 1 2 3 4 5 6 ; do \
		export OMP_NUM_THREADS=$$i ; \
		echo "threads =" $$i >> results.txt ; \
		./jacOMP 120000 12000 >> results.txt ; \
		echo "" >> results.txt ; \
	done

testMPI:
	clear
	for i in 1 2 3 4 5 6 ; do \
		echo "processes =" $$i >> results.txt ; \
		mpirun -np $$i --hostfile h6 --mca btl_tcp_if_include eno1 jacMPI 120000 12000 1 >> results.txt ; \
		echo "" >> results.txt ; \
	done
	
test:
	k=1 ; while [[ $$k -le 1000 ]] ; do \
		echo "k =" $$k ; \
		((k = k * 2)) ; \
	done

K=1 2 3 4 6 8 10 12 14 16 32 64 128 256 512 1028 2048
testMPI2: #testMPI3
	echo "" > results.txt
	clear
	echo "cod slots=6" > h6
	echo "6 CORES, 1 PROCESS" >> results.txt
	k=2 ; \
	while [[ $$k -le 250 ]] ; do \
		echo "k =" $$k >> results.txt ; \
		mpirun -np 6 --hostfile h6 --mca btl_tcp_if_include eno1 jacMPI 480000 12000 $$k >> results.txt ; \
		((k = k + 2)) ; \
	done

testMPI3:
	echo "anchovy slots=6" >> h6
	echo "" >> results2.txt
	echo "12 CORES, 2 PROCESSES" >> results2.txt
	k=1 ; \
	while [[ $$k -le 250 ]] ; do \
		echo "k =" $$k >> results2.txt ; \
		mpirun -np 12 --hostfile h6 --mca btl_tcp_if_include eno1 jacMPI 480000 12000 $$k >> results2.txt ; \
		((k = k + 2)) ; \
	done
