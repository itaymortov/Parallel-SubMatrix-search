build:	
	mpicxx -fopenmp -c main.c -o main.o
	nvcc -I./Common  -gencode arch=compute_61,code=sm_61 -c ComputeMatch.cu -o ComputeMatch.o
	mpicxx -fopenmp -o work  main.o ComputeMatch.o  -L/usr/local/cuda/lib -L/usr/local/cuda/lib64 -lcudart

clean:
	rm -f *.o ./work

run:
	mpiexec -np 2 ./work

runOnMultibleComputers:
	mpiexec -np 2 -hostfile hosts.txt -map-by node ./work
