#include "fpga_api.h"
#include <cstring>
#define DATA_SIZE SIZE*(SIZE+1) // fpga bram data size

#define min(x,y) (((x)<(y))?(x):(y))

FPGA::FPGA(off_t data_addr, off_t api_addr)
{
    api_ = new unsigned int[SIZE];    // use api_ as tempolar output 
    data_ = new float[DATA_SIZE];	
}

FPGA::~FPGA()
{
    delete[] api_;
    delete[] data_;
}

float* FPGA::matrix(void)
{
	return data_ + SIZE;
}

float* FPGA::vector(void)
{
	return data_;
}

const float* FPGA::run()
{
    float* vec = this->vector();
    float* mat = this->matrix();
    float* out  = reinterpret_cast<float*>(api_);  

    for(int i = 0 ; i < SIZE; ++i)
    {
        out[i] = 0;

        for(int j = 0 ; j < SIZE; ++j)
           out[i] += vec[j] * mat[SIZE*i + j];
    }

	for(int i = 0 ; i < SIZE; ++i)
	{
		data_[i] = out[i];
	}

    return data_;    
}

void FPGA::largeMV(const float* large_mat, const float* input,
		float* output, int M, int N)
{
    float* vec = this->vector(); //64
    float* mat = this->matrix(); //64x64

    
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
  
   
/*
    for(int j=0;j<(N/SIZE)*SIZE +1;j=j+SIZE){
	if(j == (N/SIZE)*SIZE){
	  
	}
	else{
	  for(int i=0;i<(M/SIZE)*SIZE +1;i=i+SIZE){		
	    if(i == ((M/SIZE)*SIZE){
              mat = *(large_mat + N*SIZE*i + j*SIZE);
	      for(int k = (M%SIZE)*SIZE ; k < SIZE*SIZE ; k++){
                *(mat + k) = 0;
              }
	      *(output + j*SIZE) = *(output + j*SIZE) + FPGA::run();
	    }
	    else{
              mat = *(large_mat + N*SIZE*i + j*SIZE);
	      *(output + j*SIZE) = *(output + j*SIZE) + FPGA::run();
 	    }
   	  }
        }

    }

   for(int i=0 ; i < N*M ; i= i + SIZE){
	
	


        vec = *(input + i);


        if(i == (M/SIZE) ){
             
        }
        for(int j=0; j < (N/SIZE) ; j++){
            mat = *(large_mat + j*SIZE*SIZE);
            //*(output + j*SIZE) = vec * mat;
	    *(output + j*SIZE) = *(output + j*SIZE) + FPGA::run();
        }
    }

    vec = input;
    mat = large_mat;
*/

}
