#define MAX 1048576
__kernel void montecarlo(                                              
   __global unsigned int* semente,                                     
   __global unsigned int* dentro,                                      
   const unsigned int count)                                           
{                                                                      
   int id = get_global_id(0);                                          
   float x, y;                                                        
                                                                       
  for(int i=0;i<MAX;i++){                                              
     x = (float) rand_r(&semente[id]) / INT_MAX;                      
     y = (float) rand_r(&semente[id]) / INT_MAX;                      
                                                                       
     if( x*x + y*y <= 1.0 ){                                           
       dentro[id]++;                                                  
     }                                                                
  }                                                                   
}                                                                    

