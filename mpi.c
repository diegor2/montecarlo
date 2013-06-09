#include <mpi.h>
#include <stdio.h> 
#include <stdlib.h>

#define MAX 1073741824 

int main(int argc, char** argv){

	int numtasks, rank, sendcount, recvcount, source;
	int i, j, dentro, seed;
   	double x, y, qpi;	
	int *contadores;

	//inicializa MPI
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

	contadores = (int*) malloc(sizeof(int) * numtasks);

       	source = 1;
       	dentro = 0;

       	for(i=0; i < (MAX/numtasks) ; i++){
       	    x = (double) rand() / RAND_MAX;
       	    y = (double) rand() / RAND_MAX;

       	    if( x*x + y*y <= 1.0 ){
       	      dentro++; 
       	    }
       	}

       	MPI_Gather (&dentro, 1, MPI_INT, contadores, 1,
       			 MPI_INT, source, MPI_COMM_WORLD);

	// imprime resultado
	if(source == rank){
	    int total = 0;
	    for(i=0; i<numtasks; i++){
	        total += contadores[i];
	    }
	    qpi = (double) total/ MAX;
	    printf("%lf\n", 4*qpi);
	}

	MPI_Finalize();
	return EXIT_SUCCESS;	
}
