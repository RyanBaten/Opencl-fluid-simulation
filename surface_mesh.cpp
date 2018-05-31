#include "surface_mesh.h"
#include <math.h>

// Initialize a mesh of w*h points separated by a spacing amount
surfaceMesh::surfaceMesh(int w, int h, float s) {
  mesh_mode = 0;
  width = w;
  height = h;
  spacing = s;
  gpu = new gpu_handler(32);
  mesh = new float[width*height*4];
  heightf = new float[width*height];
  obstacle = new int[(width+2)*(height+2)];
  // Fill mesh with equally spaced points, spaced by a certain amount
  for (int j=0; j<height; j++) {
    for (int i=0; i<width; i++) {
      mesh[4*(j*width+i)] = i*spacing - height*spacing/2;
      mesh[4*(j*width+i)+1] = 0;
      mesh[4*(j*width+i)+2] = j*spacing - width*spacing/2;
      mesh[4*(j*width+i)+3] = 1;
      heightf[j*width+i] = 0;
      if (abs(i-width/2) < width/6 && abs(j-height/2) < height/6) 
         obstacle[(j+1)*width+i+1] = 1;
    }
  }
  // Make the walls part of obstacle
  for (int j=0; j<height+2; j++) {
    obstacle[j*width] = 1;
    obstacle[j*width+width+1] = 1;
  }
  for (int i=0; i<width+2; i++) {
    obstacle[i] = 1;
    obstacle[(height+1)*width+i] = 1;
  }
}

surfaceMesh::~surfaceMesh() {
  delete mesh;
  delete heightf;
}

// Resets the mesh
void surfaceMesh::reset() {
  for (int j=0; j<height; j++) {
    for (int i=0; i<width; i++) {
      mesh[4*(j*width+i)+1] = 0;
      heightf[j*width+i] = 0;
    }
  }
}

// Procedural wave generation on the cpu
void surfaceMesh::procedural(float time) {
  // Vary the height of the points using overlapping sine waves of differing wavelengths
  for (int j=0; j<height; j++) {
    for (int i=0; i<width; i++) {
      mesh[4*(j*width+i)+1] = 0.1*sin(0.01*time+i*spacing)+0.15*sin(0.02*time+j*spacing)+0.2*sin(0.03*time+(i+j)*spacing);
    }
  }
}

const char* procedural_source = 
  "__kernel void procedural(float time, float spacing, int width, __global float mesh[])\n"
  "{\n"
  "  unsigned int i = get_global_id(0);\n"
  "  unsigned int j = get_global_id(1);\n"
  "  mesh[4*(j*width+i)+1] = 0.1*sin(0.01*time+i*spacing)+0.15*sin(0.02*time+j*spacing)+0.2*sin(0.03*time+(i+j)*spacing);\n"
  "}\n";

// Procedural wave generation on the gpu
void surfaceMesh::proceduralDevice(float time) {
  // Size of mesh
  unsigned int N = width*height*4*sizeof(float);
  // Allocate space on device
  cl_mem mesh_d = gpu->create_buffer(CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,N,mesh);
  // Create kernel
  gpu->create_kernel(procedural_source,"procedural");
  // Set arguments
  gpu->set_arg(0,sizeof(float),&time);
  gpu->set_arg(1,sizeof(float),&spacing);
  gpu->set_arg(2,sizeof(int),&width);
  gpu->set_arg(3,sizeof(cl_mem),&mesh_d);
  // Run the kernel
  gpu->run_kernel(width,height);
  // Read back results
  gpu->read_buffer(mesh_d,CL_TRUE,0,N,mesh,0,NULL,NULL);
  // Free device memory
  clReleaseMemObject(mesh_d);
}

// Heightfield approximations on the cpu
// This is the helloworld algorithm outlined in https://www.cs.ubc.ca/~rbridson/fluidsimulation/fluids_notes.pdf
// I plan to update this to be the full example
void surfaceMesh::heightfield() {
  int li, lj, hi, hj;
  for (int j=0; j<height; j++) {
    for (int i=0; i<width; i++) {
      li = i==0 ? 1 : i;
      hi = i==width-1 ? width-2 : i;
      lj = j==0 ? 1 : j;
      hj = j==height-1 ? height-2 : j;
      heightf[j*width+i] += (mesh[4*(j*width+(li-1))+1] + mesh[4*(j*width+(hi+1))+1] + mesh[4*((lj-1)*width+i)+1] + mesh[4*((hj+1)*width+i)+1])/4 - mesh[4*(j*width+i)+1];
      heightf[j*width+i] *= 0.998;
    }
  }
  for (int j=0; j<height; j++) {
    for (int i=0; i<width; i++) {
      mesh[4*(j*width+i)+1] += heightf[j*width+i];
    }
  }
}

const char* heightfield_p1_source = 
  "__kernel void heightfield_p1(int width, int height, __global float mesh[], __global float heightf[])\n"
  "{\n"
  "  int i = get_global_id(0);\n"
  "  int j = get_global_id(1);\n"
  "  int li = max(1,i);\n"
  "  int hi = min(width-2,i);\n"
  "  int lj = max(1,j);\n"
  "  int hj = min(height-2,j);\n"
  "  heightf[j*width+i] += (mesh[4*(j*width+(li-1))+1] + mesh[4*(j*width+(hi+1))+1] + mesh[4*((lj-1)*width+i)+1] + mesh[4*((hj+1)*width+i)+1])/4 - mesh[4*(j*width+i)+1];\n"
  "  heightf[j*width+i] *= 0.998;\n"
  "}\n";

const char* heightfield_p2_source = 
  "__kernel void heightfield_p2(int width, __global float mesh[], __global float heightf[])\n"
  "{\n"
  "  unsigned int j = get_global_id(0);\n"
  "  unsigned int i = get_global_id(1);\n"
  "  mesh[4*(j*width+i)+1] += heightf[j*width+i];\n"
  "}\n";
 

// Heightfield approximations on the gpu
void surfaceMesh::heightfieldDevice() {
  // Size of mesh
  unsigned int N = width*height*4*sizeof(float);
  // Size of buffer for heightfield
  unsigned int M = width*height*sizeof(float);
  // Allocate space on device
  cl_mem mesh_d = gpu->create_buffer(CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,N,mesh);
  cl_mem heightf_d = gpu->create_buffer(CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,M,heightf);
  // Create kernel p1
  gpu->create_kernel(heightfield_p1_source, "heightfield_p1");
  // Arguments to part 1
  gpu->set_arg(0,sizeof(int),&width);
  gpu->set_arg(1,sizeof(int),&height);
  gpu->set_arg(2,sizeof(cl_mem),&mesh_d);
  gpu->set_arg(3,sizeof(cl_mem),&heightf_d);
  // Run kernel
  gpu->run_kernel(width,height);
  // Read back results
  gpu->read_buffer(heightf_d,CL_TRUE,0,M,heightf,0,NULL,NULL);
  // Add in the heights
  for (int j=0; j<height; j++) {
    for (int i=0; i<width; i++) {
      mesh[4*(j*width+i)+1] += heightf[j*width+i];
    }
  }
  // Free memory
  clReleaseMemObject(mesh_d);
  clReleaseMemObject(heightf_d);
}

// Add some disturbance when using heightfield
void surfaceMesh::addHFRipple(int x, int y) {
  heightf[y*width+x] += 20;
}


void surfaceMesh::heightfieldObstacle() {
  int li, lj, hi, hj;
  for (int j=0; j<height; j++) {
    for (int i=0; i<width; i++) {
      li = obstacle[(j+1)*width+i] ? i : i-1;
      hi = obstacle[(j+1)*width+i+2] ? i : i+1;
      lj = obstacle[(j)*width+i+1] ? j : j-1;
      hj = obstacle[(j+2)*width+i+1] ? j : j+1;
      if (!obstacle[(j+1)*width+i+1]) {
        heightf[j*width+i] += (mesh[4*(j*width+li)+1] + mesh[4*(j*width+hi)+1] + mesh[4*(lj*width+i)+1] + mesh[4*(hj*width+i)+1])/4 - mesh[4*(j*width+i)+1];
        heightf[j*width+i] *= 0.998;
      }
    }
  }
  for (int j=0; j<height; j++) {
    for (int i=0; i<width; i++) {
      mesh[4*(j*width+i)+1] += heightf[j*width+i];
    }
  }
}
const char* heightfield_obstacle_source = 
  "__kernel void heightfield_obs(int width, int height, __global float mesh[], __global float heightf[], __global int obstacle[])\n"
  "{\n"
  "  int i = get_global_id(0);\n"
  "  int j = get_global_id(1);\n"
  "  int li = obstacle[(j+1)*width+i] ? i : i-1;\n"
  "  int hi = obstacle[(j+1)*width+i+2] ? i : i+1;\n"
  "  int lj = obstacle[(j)*width+i+1] ? j : j-1;\n"
  "  int hj = obstacle[(j+2)*width+i+1] ? j : j+1;\n"
  "  heightf[j*width+i] += ((mesh[4*(j*width+li)+1] + mesh[4*(j*width+hi)+1] + mesh[4*(lj*width+i)+1] + mesh[4*(hj*width+i)+1])/4 - mesh[4*(j*width+i)+1]) * (1-obstacle[(j+1)*width+i+1]);\n"
  "  heightf[j*width+i] *= 0.998;\n"
  "}\n";

void surfaceMesh::heightfieldObstacleDevice() {
  // Size of mesh
  unsigned int N = width*height*4*sizeof(float);
  // Size of buffer for heightfield
  unsigned int M = width*height*sizeof(float);
  // Size of buffer for obstacle
  unsigned int O = (width+2)*(height+2)*sizeof(int);
  // Allocate space on device
  cl_mem mesh_d = gpu->create_buffer(CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,N,mesh);
  cl_mem heightf_d = gpu->create_buffer(CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,M,heightf);
  cl_mem obstacle_d = gpu->create_buffer(CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,O,obstacle);
  // Create kernel p1
  gpu->create_kernel(heightfield_obstacle_source, "heightfield_obs");
  // Arguments to part 1
  gpu->set_arg(0,sizeof(int),&width);
  gpu->set_arg(1,sizeof(int),&height);
  gpu->set_arg(2,sizeof(cl_mem),&mesh_d);
  gpu->set_arg(3,sizeof(cl_mem),&heightf_d);
  gpu->set_arg(4,sizeof(cl_mem),&obstacle_d);
  // Run kernel
  gpu->run_kernel(width,height);
  // Read back results
  gpu->read_buffer(heightf_d,CL_TRUE,0,M,heightf,0,NULL,NULL);
  // Add in the heights
  for (int j=0; j<height; j++) {
    for (int i=0; i<width; i++) {
      mesh[4*(j*width+i)+1] += heightf[j*width+i];
    }
  }
  // Free memory
  clReleaseMemObject(mesh_d);
  clReleaseMemObject(heightf_d);
  clReleaseMemObject(obstacle_d);
}

void surfaceMesh::drawMesh() {
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(4,GL_FLOAT,0,mesh);
  // Draw mesh
  glDrawArrays(GL_POINTS,0,width*height);
  glDisableClientState(GL_VERTEX_ARRAY);
}

void surfaceMesh::toggleMeshMode() {
  mesh_mode = !mesh_mode;
}
