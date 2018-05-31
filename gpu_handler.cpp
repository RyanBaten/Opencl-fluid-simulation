#include "gpu_handler.h"

void Fatal(const char* format, ...) {
  va_list args;
  va_start(args,format);
  fprintf(stderr, format, args);
  va_end(args);
  exit(1);
}

gpu_handler::gpu_handler(size_t _work_size) {
  work_size = _work_size;
  init_GPU();
}

// Initialize the gpu for use
void gpu_handler::init_GPU() {
  int n_entries = 1024;
  cl_uint n_platforms;
  cl_platform_id platforms[n_entries];
  // Get the CL platforms 
  if (clGetPlatformIDs(n_entries,platforms,&n_platforms))
    Fatal("Failed to get CL platforms\n");
  else if (n_platforms < 1)
    Fatal("Did not find OpenCL platform\n");
  // Find fastest platform
  int max_Gflops = -1;
  for (unsigned int platform=0; platform<n_platforms; platform++) {
    // Get the devices per platform
    cl_uint n_devices;
    cl_device_id devices[n_entries];
    if (clGetDeviceIDs(platforms[platform],CL_DEVICE_TYPE_GPU,n_entries,devices,&n_devices))
      Fatal("Failed to get device IDs\n");
    else if (n_devices<1)
      Fatal("Did not find available device\n");
    // Find the fastest device
    for (unsigned int device=0; device<n_devices; device++) {
      size_t n_cores;
      size_t max_MHz;
      // Get device # cores and max clock frequency
      if (clGetDeviceInfo(devices[device],CL_DEVICE_MAX_COMPUTE_UNITS,sizeof(n_cores),&n_cores,NULL)) Fatal("Could not get # parallel compute cores\n");
      if (clGetDeviceInfo(devices[device],CL_DEVICE_MAX_CLOCK_FREQUENCY,sizeof(max_MHz),&max_MHz,NULL)) Fatal("Could not get max configured clock frequency of the device\n");
      // Calculate Gflops as number of cores * max clock frequency
      int Gflops = n_cores*max_MHz;
      // Update result if device is faster
      if(Gflops > max_Gflops) {
        device_id = devices[device];
        max_Gflops = Gflops;
      }
    }
  }
  // Check thread count
  if (clGetDeviceInfo(device_id,CL_DEVICE_MAX_WORK_GROUP_SIZE,sizeof(max_n_work_items),&max_n_work_items,NULL)) Fatal("Could not get max work group size\n");
  // Create OpenCL context
  cl_int error;
  context = clCreateContext(0,1,&device_id,NULL,NULL,&error);
  if (!context || error) Fatal("Cannot create OpenCL context\n");
  // Create OpenCL command queue
  queue = clCreateCommandQueue(context,device_id,0,&error);
  if (!queue || error) Fatal("Cannot create OpenCL command queue\n");
}

// Allocate device memory
cl_mem gpu_handler::create_buffer(cl_mem_flags flags, size_t size, void* host_ptr) {
  cl_int error;
  cl_mem ret = clCreateBuffer(context, flags, size, host_ptr, &error);
  if (error) Fatal("Cannot allocate device memory");
  return ret;
}

// If the mode changed, create a new kernel
void gpu_handler::create_kernel(const char* source, const char* name) {
  cl_int error;
  // Compile Kernel  
  program = clCreateProgramWithSource(context,1,&source,0,&error);
  if (error) Fatal("Cannot create program\n");
  int ret;
  if ((ret = clBuildProgram(program,0,NULL,NULL,NULL,NULL))) {
    // If error occurred, get reason why
    char log[1048576];
    if ((ret = clGetProgramBuildInfo(program,device_id,CL_PROGRAM_BUILD_LOG,sizeof(log),log,NULL))) {
      Fatal("Cannot get build log\n");
    } else {
      Fatal("Cannot build program\n%s\n",log);
    }
  }
  kernel = clCreateKernel(program,name,&error);
  if (error) Fatal("Cannot create kernel\n");
}

// Set an argument for the kernel
void gpu_handler::set_arg(cl_uint num, size_t size, const void* value) {
  if(clSetKernelArg(kernel,num,size,value)) Fatal("Cannot set kernel parameter");
}

// Run the kernel
void gpu_handler::run_kernel(size_t width, size_t height) {
  size_t Global[2] = {width, height};
  size_t Local[2] = {work_size, work_size};
  if (clEnqueueNDRangeKernel(queue,kernel,2,NULL,Global,Local,0,NULL,NULL)) Fatal("Cannot run kernel\n");
  // Release kernel and program
  if (clReleaseKernel(kernel)) Fatal("Cannot release kernel\n");
  if (clReleaseProgram(program)) Fatal("Cannot release program\n");
}

// Read back from device to host
void gpu_handler::read_buffer(cl_mem buffer, cl_bool blocking, size_t offset, size_t cb, void* ptr, cl_uint num_events, const cl_event *wait_list, cl_event *event) {
  unsigned int err;
  if((err =clEnqueueReadBuffer(queue,buffer,blocking,offset,cb,ptr,num_events,wait_list,event))) {
    printf("%d\n",err);
    Fatal("Cannot copy back from device");
  }
}
