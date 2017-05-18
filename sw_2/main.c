#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>


#define SIZE 16

typedef union {
  float f;
  unsigned int i;
}foo;

int main(int argc, char** argv)
{
  int i, j;
  foo container;

  float flat[(SIZE+1)*SIZE];
  float input_a[SIZE];
  float input_b[SIZE*SIZE];
  float output_cpu[SIZE];
  float output_fpga[SIZE];

  // initialization
  FILE *fd;
  fd = fopen("./input.txt", "r");

  unsigned int a;
  i = 0;
  while ( !feof(fd) )
  {
    if (fscanf(fd, "%X", &a) != EOF)
    {
      container.i = a;
      //printf("%f %X\n", container.f, container.i);
      if (i < SIZE)
        input_a[i] = container.f;
      else
        input_b[i - SIZE] = container.f;

      flat[i] = container.f;
      //printf("%d, %f\n", i, container.f);
      i++;
    }
  }
  fclose(fd);

  //output_cpu = 0.0f;
  for(i = 0; i<SIZE; i++)
    output_cpu[i] = 0.0f;

  // computation
  for (i = 0; i < SIZE; i++){
    for( j = 0; j < SIZE; j++)
      output_cpu[SIZE-1-i] += input_a[SIZE-1-i] * input_b[SIZE*(SIZE-1-i)+SIZE-1-j];
  }

  // FPGA offloading
  // memory load
  int foo;
  foo = open("/dev/mem", O_RDWR | O_NONBLOCK);
  float *fpga_bram = mmap(NULL, ((SIZE+1)*SIZE)* sizeof(float), PROT_WRITE, MAP_SHARED, foo, 0x40000000);
  for (i = 0; i < (SIZE+1)*SIZE; i++)
  {
    *(fpga_bram + i) = flat[i];
  }

  // run
  unsigned int *fpga_ip = mmap(NULL, sizeof(float), PROT_WRITE, MAP_SHARED, foo, 0x43c00000);
  *fpga_ip = 0x5555;
  
  // wait
  while (*fpga_ip == 0x5555);

  // result
  for(i = 0; i<SIZE; i++)
    output_fpga[i]=*(fpga_bram+i);

  // display
  printf("%-10s%-10s%-10s%-10s\n", "index", "CPU", "FPGA", "FPGA(hex)");
  for(i = 0; i<SIZE; i++){
  container.f = output_fpga[i];
  printf("%-10d%-10f%-10f%-10X\n", 0, output_cpu[i], output_fpga[i], container.i);
  }

  close(foo);

  return 0;
}
