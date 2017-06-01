#include "fpga_api.h"
#include <cstring>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define DATA_SIZE SIZE*(SIZE+1)*sizeof(float) // fpga bram data size

#define min(x,y) (((x)<(y))?(x):(y))

FPGA::FPGA(off_t data_addr, off_t api_addr)
{
    fd_ = open("/dev/mem", O_RDWR);
    data_ = static_cast<float*>(mmap(NULL, DATA_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd_, data_addr));
    api_ = static_cast<unsigned int*>(mmap(NULL, sizeof(unsigned int), PROT_READ|PROT_WRITE, MAP_SHARED,fd_, api_addr));
}

FPGA::~FPGA()
{
    munmap(data_, DATA_SIZE );
    munmap(api_, sizeof(unsigned int));
    close(fd_);
}

float* FPGA::matrix(void)
{
	return data_ + SIZE;
}

float* FPGA::vector(void)
{
	return data_;
}

const float* __attribute__((optimize("O0"))) FPGA::run()
{
    *api_ = 0x5555;
    while(*api_ == 0x5555);

    return data_;    
}

void FPGA::largeMV(const float* large_mat, const float* input,
		float* output, int M, int N)
{
	float* vec = this->vector();
    float* mat = this->matrix();
	
    int N2 = ((N-1)/SIZE +1)*SIZE;
    int M2 = ((M-1)/SIZE +1)*SIZE;
    float *new_mat = new float[M2*N2];
    for(int j=0;j<M2; j += N2){
      for(int i=0;i<N2;i += 1){
        if(i<N){
          if(j<M){
            new_mat[i+j] = large_mat[i+j];
          }
          else{
            new_mat[i+j] = 0;
          }
        }
        else{
          new_mat[i+j] = 0;         
        }
      }
    }
    float *new_vec = new float[N2];
     for(int i=0;i<N2;i += 1){
        if(i<N){
    	  new_vec[i] = input[i];
        }
        else{
          new_mat[i] = 0;   
        }
      }
    
   float* new_output = new float[N2];
    for(int j=0;j<N2;j=j+SIZE){//vertical
      vec = (new_vec + j);
      for(int i=0;i<M2;i=i+SIZE){//horizental 
         mat = (new_mat + N2*SIZE*i + j*SIZE);
	 *(new_output + j*SIZE) = *(new_output + j*SIZE) + *FPGA::run();
      }
    }
   for(int k=0;k<N;k++){
     output[k] = new_output[k];
   }
  
	
}
