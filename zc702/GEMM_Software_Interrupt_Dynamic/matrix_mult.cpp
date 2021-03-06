#include <stdio.h>
#include <sds_lib.h>
#include <stdlib.h>
#include <cstdlib>
#include <iostream>
#include <fstream>

#include <fcntl.h>
#include <sys/mman.h>

#include "tbb/task_scheduler_init.h"
#include "tbb/tbb.h"
#include "matrix_mult.h"
#ifndef __SDSVHLS__
//#include "uio_axi_gpio.h"
#endif


//#define DEBUG
//#define DEEP_CPU_REPORT
//#define DEEP_GPU_REPORT
#define DYNAMIC
//#define ORACLE
//#define FIXEDCHUNK
//#define CONCORD
//#define HDSSS
//#define LOGFIT
//#define PJTRACER
//#define OVERHEAD_STUDY

int main_thread=0;
int main_core=0;

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

#ifdef ORACLE
#include "Oracle.h"
#endif
#ifdef DYNAMIC
#include "../Schedulers/DynamicScheduler.h"
HSchedulerDynamic hs;
#define EXTRA 1
typedef int extraType;
#endif
#ifdef FIXEDCHUNK
#include "FixedScheduler.h"
#endif
#ifdef CONCORD
#include "Concord.h"
#endif
#ifdef HDSSS
#include "HDSS.h"
#endif
#ifdef LOGFIT
#include "LogFit.h"
#endif


#include "Body.h"

using namespace std;
using namespace tbb;

char **argv;


/*****************************************************************************
 * Global variables
 * **************************************************************************/
ofstream resultfile;



const unsigned AOCL_ALIGNMENT = 64;
void *alignedMalloc(size_t size) {
  void *result = NULL;
  posix_memalign (&result, AOCL_ALIGNMENT, size);
  return result;
}

void alignedFree(void * ptr) {
  free (ptr);
}

// Returns the current system time in microseconds
long long get_time() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000000) + tv.tv_usec;
}

void fatal(const char *s) {
	fprintf(stderr, "Error: %s\n", s);
	exit(1);
}


unsigned golden_MMM(float *a, float *b, float *c)
{
	int i,j,k;

	for(i=0;i<N;i++)
		for(j=0;j<P;j++)
			for(k=0;k<M;k++)
		        c[i*P+j] += a[i*M+k]*b[k*P+j];
	return 1;
}



/*
void writeoutput(float *vect, int grid_rows, int grid_cols, char *file) {

	int i,j, index=0;
	FILE *fp;
	char str[STR_SIZE];

	if( (fp = fopen(file, "w" )) == 0 )
          printf( "The file was not opened\n" );


	for (i=0; i < grid_rows; i++) 
	 for (j=0; j < grid_cols; j++)
	 {

		 sprintf(str, "%d\t%g\n", index, vect[i*grid_cols+j]);
		 fputs(str,fp);
		 index++;
	 }
		
      fclose(fp);	
}


void readinput(float *vect, int grid_rows, int grid_cols, char *file) {

  	int i,j;
	FILE *fp;
	char str[STR_SIZE];
	float val;

	if( (fp  = fopen(file, "r" )) ==0 )
            fatal( "The file was not opened" );


	for (i=0; i < grid_rows; i++) 
	 for (j=0; j < grid_cols; j++)
	 {
 		if(i==0 || i==(grid_rows-1) || j==0 || j==(grid_cols-1))
		{
			  vect[i*grid_cols+j] = 0.0;
		}
		else
		{
			fgets(str, STR_SIZE, fp);
		
			if (feof(fp))
			{
				rewind(fp);
				fgets(str, STR_SIZE, fp);
				printf("not enough lines in file\n");
			}
		
			//if ((sscanf(str, "%d%f", &index, &val) != 2) || (index != ((i-1)*(grid_cols-2)+j-1)))	
			if ((sscanf(str, "%f", &val) != 1))
				fatal("invalid file format");
		
			vect[i*grid_cols+j] = val;
		}	
	}

	fclose(fp);	

}
*/

/*
   compute N time steps
*/


int compute_matrix_mult(float *array_a,float* array_b, float *array_c, int* interrupt)
 
{ 

	//variables
	NbodyTask body;
	int numcpus = atoi(argv[1]);
	int numgpus = atoi(argv[2]);

	int t;

	//printf("call init\n");
	
	task_scheduler_init init(numcpus+numgpus);
	hs.Initialize(numcpus, numgpus, atoi(argv[3]));//,interrupt);
	// Begin iterations
	hs.startTimeAndEnergy();
        int total_iterations = 1;

	//printf("being interactions\n");

	for (t = 0; t < total_iterations; t += 1) {
		
		mystep = t;
	//	printf("call parallel for\n");
		hs.heterogeneous_parallel_for(0, rows, &body);
		
	}
	

	

	hs.endTimeAndEnergy();
	hs.saveResultsForBench();
	return 0;
}

void usage(int argc, char **argv) {
	fprintf(stderr, "Usage: %s <numcpus> <numgpus> <chunkGPU>\n", argv[0]);
	fprintf(stderr, "\t<numcpus> - \n");
	fprintf(stderr, "\t<numgpus> - \n");
	fprintf(stderr, "\t<chunkGPU> - (Fixed or Dynamic) fixed size size assigned to GPU (CPU adapts dynamically to match effort)\n");
	exit(1);
}



int main(int argc, char** argvv) {

  
   static char receive[10];

    printf("Starting\n");
	//end variables
	argv = argvv;

   // #ifndef __SDSVHLS__
  /*      printf("Checking interrupt LEDs ON\n");
	axi_gpio axi_gpio_0;
	int status_gpio = axi_gpio_init(&axi_gpio_0, "gpio");
	if (status_gpio < 0) {
		perror("axi_gpio_init");
		return -1;}*/

    int memfd;
    void *mapped_base, *mapped_dev_base;
    off_t dev_base = HW_ADDR_GPIO; //GPIO hardware


    memfd = open("/dev/mem", O_RDWR | O_SYNC);
    if (memfd == -1) {
    	printf("Can't open /dev/mem.\n");
        exit(0);
    }
    printf("/dev/mem opened for gpio.\n");

    // Map one page of memory into user space such that the device is in that page, but it may not
      // be at the start of the page.


      mapped_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, dev_base & ~MAP_MASK);
          if (mapped_base == (void *) -1) {
          printf("Can't map the memory to user space.\n");
          exit(0);
      }
       printf("GPIO mapped at address %p.\n", mapped_base);



      // get the address of the device in user space which will be an offset from the base
      // that was mapped as memory is mapped at the start of a page

      mapped_dev_base = mapped_base + (dev_base & MAP_MASK);




  file_desc = open("/dev/my_driver",O_RDWR);
  if (file_desc < 0)
  {	
		printf("cant open device: /dev/my_driver\n");
		exit(0);
  }
  
  // #endif
   char led_value = 255;

    #ifndef __SDSVHLS__
               //   axi_gpio_write_reg(axi_gpio_0.baseaddr, 0, (unsigned char) led_value);
    #endif


   // interrupt = (int *) sds_alloc(sizeof(int));


    interrupt = (int*)mapped_dev_base;
    /*int* enable_channel = (int*)mapped_dev_base + fpga_mmio_ier;
    int* enable_general = (int*)mapped_dev_base + fpga_mmio_gier;*/

    status = (int*)mapped_dev_base+fpga_mmio_gpio; //(int*)read(file_desc,receive,10); //axi_gpio_0.baseaddr;

    //status = (int*)mapped_dev_base;

    sds_mmap((void *)HW_ADDR_GPIO_INT,4,(void *)status);

    //sds_mmap((void *)HW_ADDR_GPIO,4,(void *)status);

    printf("Interrupt located at address %x\n",(int)status);

    //printf("Switch on leds\n");
    
    //*interrupt = (unsigned char) led_value; //leds on


   /* *enable_channel = 0xFFFFFFFF; //leds on
    *enable_general = 0xFFFFFFFF; //leds on*/

     //printf("Force interrupt\n");  
     //getchar();
     //*status = 255;
    
     printf("Switch off leds\n");

     led_value = 0;

    *interrupt = (unsigned char) led_value; //leds off
    
     int total_iterations,i,j,ret;

	
	if (argc < 4)
		usage(argc, argv);





    #ifdef HOSTA7
	main_thread=syscall(__NR_gettid);
	main_core=sched_getcpu();
	cout << "Main thread id: " << main_thread << endl;
	cout << "          core: " << main_core << endl;
    #endif

    printf("Matrix size= %d * %d\r\n", N, N);

   /* array_a = (float *) alignedMalloc(N*M*sizeof(float));
    array_b = (float *) alignedMalloc(M*P*sizeof(float));
    array_c = (float *) alignedMalloc(N*P*sizeof(float));*/
    c_golden=(float *)alignedMalloc(N*P*sizeof(float));


    array_a = (float *) sds_alloc(N*M*sizeof(float));
    array_b = array_a; //(float *) sds_alloc(M*P*sizeof(float));

    array_c = (float *) sds_alloc(N*P*sizeof(float));
   
    //array_temp = (float *) sds_alloc(size*sizeof(float));
    //array_power = (float *) sds_alloc(size*sizeof(float));
    //array_out = (float *) sds_alloc(size*sizeof(float));

    
    if( !array_a)
    {
        printf("unable to allocate memory a\n");
        exit(1);
    }

    if( !array_b)
    {
        printf("unable to allocate memory b\n");
        exit(1);
    }

    if( !array_c)
    {
        printf("unable to allocate memory c\n");
        exit(1);
    }

    printf("Memories allocated\n");

     for(i=0;i<N;i++){
	for(j=0;j<M;j++) {
		float t = rand()/(1.0*RAND_MAX);
	    array_a[i*M+j]=t;
	}
     }

     for(i=0;i<M;i++) {
	for(j=0;j<P;j++) {
		float t = rand()/(1.0*RAND_MAX);
	    array_b[i*P+j]=t;
	}
      }

     for(i=0;i<N;i++)
	for(j=0;j<P;j++) {
		float t = rand()/(1.0*RAND_MAX);
	    array_c[i*P+j]=0;
	}
  
        printf("Starting computation\n");
        local_interrupt = 0x34;

       // interrupt = &local_interrupt;

	// Perform the computation
	compute_matrix_mult(array_a, array_b,array_c,interrupt);

	

   	printf("Verifying\n");
/*
	golden_MMM(array_a, array_b, c_golden);

        int status_op = 0;	

	for(i = 0; i < N*P; i++) {
		float fc = array_c[i];
		float gc = c_golden[i];
		float diff = fabs(fc-gc);
		if (diff > 0.001 || diff != diff) {
			printf("Error at %d Fast_MMM=%f  Golden MMM=%f\n", i, fc, gc);
			status_op = -1;
			break;
		}
	}

	if (!status_op) {
		printf("Evaluation PASSED\n");
	} else {
		printf("Evaluation FAILED\n");
	}
		
 */
    

	printf("releasing memory\n");
	sds_free(array_a);
	sds_free(array_c);
	free(c_golden);
  	sds_munmap((void *)status);
	munmap(mapped_base, MAP_SIZE);
	close(memfd);
	close(file_desc);


	return 0;
}
