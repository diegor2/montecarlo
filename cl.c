#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define DATA_SIZE 1024
#define MAX  1048576

////////////////////////////////////////////////////////////////////////////////
const char *KernelSource = "\n" \
"#define MAX 1000000                                                    \n" \
"int rand_r (__global unsigned int *seed)                               \n" \
"{                                                                      \n" \
"    unsigned int next = *seed;                                         \n" \
"    int result;                                                        \n" \
"                                                                       \n" \
"    next *= 1103515245;                                                \n" \
"    next += 12345;                                                     \n" \
"    result = (unsigned int) (next / 65536) % 2048;                     \n" \
"                                                                       \n" \
"    next *= 1103515245;                                                \n" \
"    next += 12345;                                                     \n" \
"    result <<= 10;                                                     \n" \
"    result ^= (unsigned int) (next / 65536) % 1024;                    \n" \
"                                                                       \n" \
"    next *= 1103515245;                                                \n" \
"    next += 12345;                                                     \n" \
"    result <<= 10;                                                     \n" \
"    result ^= (unsigned int) (next / 65536) % 1024;                    \n" \
"                                                                       \n" \
"    *seed = next;                                                      \n" \
"                                                                       \n" \
"    return result;                                                     \n" \
"}                                                                      \n" \
"                                                                       \n" \
"__kernel void montecarlo(                                              \n" \
"   __global unsigned int* semente,                                     \n" \
"   __global unsigned int* dentro,                                      \n" \
"   const unsigned int count)                                           \n" \
"{                                                                      \n" \
"   int id = get_global_id(0);                                          \n" \
"   float x, y;                                                         \n" \
"                                                                       \n" \
"                                                                       \n" \
"  for(int i=0;i<MAX;i++){                                              \n" \
"     x = (float) rand_r(&semente[id]) / INT_MAX;                       \n" \
"     y = (float) rand_r(&semente[id]) / INT_MAX;                       \n" \
"                                                                       \n" \
"     if( x*x + y*y <= 1.0 ){                                           \n" \
"       dentro[id]++;                                                   \n" \
"     }                                                                 \n" \
"  }                                                                    \n" \
" }                                                                     \n" \
"                                                                       \n" \
"\n";
////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv){

  double x, y, qpi;
  int i ;

    int err;                            // error code returned from api calls
    
    unsigned int contadores[DATA_SIZE]; // conta os pontos dentro do circulo
    unsigned int sementes[DATA_SIZE];   // sementes para o gerador aleatorio

    size_t global;                      // global domain size for our calculation
    size_t local;                       // local domain size for our calculation

    cl_device_id device_id;             // compute device id 
    cl_context context;                 // compute context
    cl_command_queue commands;          // compute command queue
    cl_program program;                 // compute program
    cl_kernel kernel;                   // compute kernel

    cl_mem work;
    cl_mem result;

    cl_device_type device_type = CL_DEVICE_TYPE_CPU;
    int platform = 0;

    if(argc>2){    
	platform = atoi(argv[1]);
	if(strcmp(argv[2],"gpu")==0){
          device_type = CL_DEVICE_TYPE_GPU;
	}
    }

    //printf("%s", KernelSource);
    unsigned int count = DATA_SIZE;

    for(i=0; i<DATA_SIZE; i++){
	sementes[i]=i;
    }

    for(i=0; i<DATA_SIZE; i++){
	contadores[i]=0;
    }

    // Find the platform
    cl_platform_id platforms[10];
    cl_uint *num_platforms;
    err = clGetPlatformIDs(10, platforms, num_platforms);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to find a platform!\n");
        return EXIT_FAILURE;
    }
    
    char platform_name[1024];
    err = clGetPlatformInfo(platforms[platform], CL_PLATFORM_NAME, 1024, platform_name, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to get platform information!\n");
        return EXIT_FAILURE;
    } else {
	//printf("platform :: %s\n", platform_name);
    }


    // Connect to a compute device
    err = clGetDeviceIDs(platforms[platform], device_type, 1, &device_id, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to create a device group!\n");
        return EXIT_FAILURE;
    }

    char device_name[1024];
    err = clGetDeviceInfo(device_id, CL_DEVICE_NAME, 1024, device_name, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to get device information!\n");
        return EXIT_FAILURE;
    } else {
	//printf("Device :: %s\n", device_name);
    }
  
    // Create a compute context 
    context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
    if (!context)
    {
        printf("Error: Failed to create a compute context!\n");
        return EXIT_FAILURE;
    }

    // Create a command queue 
    commands = clCreateCommandQueue(context, device_id, 0, &err);
    if (!commands)
    {
        printf("Error: Failed to create a command commands!\n");
        return EXIT_FAILURE;
    }

    // Create the compute program from the source buffer
    program = clCreateProgramWithSource(context, 1, (const char **) & KernelSource, NULL, &err);
    if (!program)
    {
        printf("Error: Failed to create compute program!\n");
        return EXIT_FAILURE;
    }

    // Build the program executable
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        size_t len;
        char buffer[2048];

        printf("Error: Failed to build program executable!\n");
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        printf("%s\n", buffer);
        exit(1);
    }

    // Create the compute kernel in the program we wish to run
    //
    kernel = clCreateKernel(program, "montecarlo", &err);
    if (!kernel || err != CL_SUCCESS)
    {
        printf("Error: Failed to create compute kernel!\n");
        exit(1);
    }

    // Create the input and output arrays in device memory for our calculation
    //
    work   = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(unsigned int) * count, NULL, NULL);
    result = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(unsigned int) * count, NULL, NULL);
    if (!work || !result)
    {
        printf("Error: Failed to allocate device memory!\n");
        exit(1);
    }  

    // Write our data set into the input array in device memory 
    //
    err = clEnqueueWriteBuffer(commands, work, CL_TRUE, 0, sizeof(unsigned int) * count, sementes, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to write to work rray!\n");
        exit(1);
    }

    err = clEnqueueWriteBuffer(commands, result, CL_TRUE, 0, sizeof(unsigned int) * count, contadores, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to write to result array!\n");
        exit(1);
    }

    // Set the arguments to our compute kernel
    //
    err = 0;
    err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &work);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &result);
    err |= clSetKernelArg(kernel, 2, sizeof(unsigned int), &count);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to set kernel arguments! %d\n", err);
        exit(1);
    }

    // Get the maximum work group size for executing the kernel on the device
    //
    err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to retrieve kernel work group info! %d\n", err);
        exit(1);
    }

    // Execute the kernel over the entire range of our 1d input data set
    // using the maximum number of work group items for this device
    //
    global = count;
    err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, &local, 0, NULL, NULL);
    if (err)
    {
        printf("Error: Failed to execute kernel! %d\n", err);
        return EXIT_FAILURE;
    }

    // Wait for the command commands to get serviced before reading back results
    //
    clFinish(commands);

    // Read back the results from the device to verify the output
    //
    err = clEnqueueReadBuffer(commands, result, CL_TRUE, 0, sizeof(unsigned int) * count, contadores, 0, NULL, NULL );  
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to read output array! %d\n", err);
        exit(1);
    }
    
    
    
    // Shutdown and cleanup
    //
    clReleaseMemObject(work);
    clReleaseMemObject(result);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(commands);
    clReleaseContext(context);

    int total = 0;
    for(i=0; i<count; i++){
       total += contadores[i];
    }

  qpi = (double) total/ count/MAX;
  printf("%lf %d %d %d\n", 4*qpi, total, count, MAX);

}
