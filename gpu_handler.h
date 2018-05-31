#ifndef GPU_HANDLER
#define GPU_HANDLER

#ifdef __APPLE__
  #include <OpenCL/opencl.h>
#else
  #include <CL/cl.h>
#endif

#include <stdio.h>
#include <stdarg.h>

void Fatal(const char* format, ...);

class gpu_handler {
  private:
    size_t work_size;
    size_t max_n_work_items;
    cl_kernel kernel;
    cl_program program;
    cl_device_id device_id;
    cl_context context;
    cl_command_queue queue;
  public:
    gpu_handler(size_t _work_size);
    void init_GPU();
    cl_mem create_buffer(cl_mem_flags flags, size_t size, void* host_ptr);
    void create_kernel(const char* source, const char* name);
    void set_arg(cl_uint num, size_t size, const void* value);
    void run_kernel(size_t width, size_t height);
    void read_buffer(cl_mem buffer, cl_bool blocking, size_t offset, size_t cb, void* ptr, cl_uint num_events, const cl_event *wait_list, cl_event *event);
};

#endif
