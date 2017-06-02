#include "fpga_api.h"

#include <iostream>
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

        for(int j = 0 ; j < SIZE; ++j){
           out[i] += vec[j] * mat[SIZE*i + j];
	}
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
    std::cout << "N2 : " << N2 <<std::endl;
    std::cout << "M2 : " << M2 <<std::endl;

    for(int j=0;j<M2*N2; j += N2){
      for(int i=0;i<N2;i += 1){
        if(i<N){
          if(j<M*N){
            new_mat[i+j] = *(large_mat+i+j);
           // std::cout <<i <<", " << j << " : " << *(new_mat+i+j) <<std::endl;
          }
          else{
            new_mat[i+j] = 0;
            //std::cout <<"0 input :"<<i <<", " << j << " : " << *(new_mat+i+j) <<std::endl;
          }
        }
        else{
          new_mat[i+j] = 0;         
          //std::cout <<"0 input :" <<i <<", " << j << " : " << *(new_mat+i+j) <<std::endl;
        }
      }
    }

    float *new_vec = new float[N2];
     for(int i=0;i<N2;i += 1){
        if(i<N){
    	  new_vec[i] = input[i];
          std::cout << "input " << i <<" : " << new_vec[i] <<std::endl;
        }
        else{
          new_vec[i] = 0;   
        }
      }
   
   float* new_output = new float[N2];
   const float* fpga_result = new float[SIZE];
    for(int j=0;j<N2;j=j+SIZE){//vertical
      for(int index = 0;index < SIZE; index ++){      
        vec[index] = new_vec[index + j];
      }

      for(int k =0; k<SIZE ; k++) std::cout << "vec" <<k <<" : "<< vec[k] <<std::endl;
     
      for(int i=0;i<M2;i=i+N2*SIZE){//horizental 
	 for(int index = 0, tmp_index =0 ;index < SIZE*SIZE;index++){
           //std::cout <<"tmp_index : "<<tmp_index <<std::endl;
           //std::cout <<"index" << tmp_index +i+j <<std::endl;
           mat[index] = new_mat[tmp_index + i + j];

  
           if(index %SIZE==(SIZE-1)){
             tmp_index += N2 - SIZE + 1;
           }
           else{
             tmp_index++;
           }
	 }

         for(int k=0 ;k<SIZE*SIZE;k++) std::cout << "mat "<<k<<": "<<mat[k] <<std::endl;
         fpga_result = FPGA::run();
         
	 for(int index =0; index < N2; index++){
           new_output[index] += fpga_result[index];
          std::cout << "fpgaresult: " << fpga_result[index] << std::endl;
          std::cout << "index : " << index << std::endl;
         }
 
	// for(int index = 0;index < SIZE; index ++){      
	// *(new_output + index + j*SIZE) = *(new_output + index + j*SIZE) + *(FPGA::run()+index);	
	// }

      }
    }

    for(int j = 0; j <M2*N2; j++){
      std::cout << "origin_met " << j << ": " << large_mat[j] << std::endl;
//      std::cout << "new_met " << j << ": " << new_mat[j] << std::endl;
    } 
/*    for(int i = 0; i < 8 ; i++){
      std::cout << "origin_input " << i << ": " << input[i]<< std::endl;
    }
 */
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
