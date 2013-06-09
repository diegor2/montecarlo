#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <time.h>

#define MAX 1073741824 

int main(int argc, char** argv){

  double x, y, qpi;
  int i, dentro, seed;

#pragma omp parallel private(seed)
{
  seed = time(NULL) + omp_get_thread_num();
#pragma omp for private(x,y) reduction(+:dentro)
  for(i = 0; i < MAX; i++){
    x = (double) rand_r(&seed) / RAND_MAX;
    y = (double) rand_r(&seed) / RAND_MAX;

   if( x*x + y*y <= 1.0 ){
      dentro++; 
    }
  }
}
  qpi = (double) dentro / MAX;
  printf("%lf\n", 4*qpi);

  return EXIT_SUCCESS;

}
