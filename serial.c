#include <stdlib.h>
#include <stdio.h>

#define MAX 1073741824 

int main(int argc, char** argv){

  double x, y, qpi;
  int i, dentro;

  for(i = 0; i < MAX; i++){
    
    x = (double) rand() / RAND_MAX;
    y = (double) rand() / RAND_MAX;

    if( x*x + y*y <= 1.0 ){
      dentro++; 
    }

  }

  qpi = (double) dentro / MAX;
  printf("%lf\n", 4*qpi);

}
