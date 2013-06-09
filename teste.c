
#include <stdlib.h>                                                    
                                                                       
__kernel void square(                                                  
   __global unsigned int* seed,                                        
   __global float* output,                                             
   const unsigned int count)                                           
{                                                                      
   int i = get_global_id(0);                                           
   int x, y;                                                           
                                                                       
   x = (double) rand_r(seed) / RAND_MAX;                              
   y = (double) rand_r(seed) / RAND_MAX;                              
}                                                                      

Error: Failed to build program executable!
<program source>:2:10: fatal error: 'stdlib.h' file not found
#include <stdlib.h>                                                    
         ^

