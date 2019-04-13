#include <stdio.h>          
#include <stdlib.h>
#include <png.h>
#include <mpi.h>
#include <sys/time.h> 
#include <time.h> 
void writeworld(char * filename, char ** my_world, int sz_x, int sz_y) {

   int width = sz_x+2;
   int height = sz_y+2;

   FILE *fp = fopen(filename, "wb");
   if(!fp) abort();

   png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (!png) abort();

   png_infop info = png_create_info_struct(png);
   if (!info) abort();

   if (setjmp(png_jmpbuf(png))) abort();

   png_init_io(png, fp);

   png_set_IHDR(
     png,
     info,
     width, height,
     8,
     PNG_COLOR_TYPE_RGB,
     PNG_INTERLACE_NONE,
     PNG_COMPRESSION_TYPE_DEFAULT,
     PNG_FILTER_TYPE_DEFAULT
   );
   png_write_info(png, info);

   png_bytep row = (png_bytep) malloc(3 * width * sizeof(png_byte));
   printf("writing RGB %d, %d\n",width, height);

   int full_size_with_boarders = (sz_x+2)*(sz_y+2);

   for(int r=0;r <height;r++) {
     for(int i=0;i < width;i++){
        switch (my_world[r][i]){
  	 case 1:
              row[(i*3)+0] = 255;
              row[(i*3)+1] = 255;
              row[(i*3)+2] = 255; 
              break;
	 case 2:
              row[(i*3)+0] = 255;
              row[(i*3)+1] = 0;
              row[(i*3)+2] = 0;
              break;
         case 3:
              row[(i*3)+0] = 0;
              row[(i*3)+1] = 255;
              row[(i*3)+2] = 0;
              break;
         case 4:
              row[(i*3)+0] = 0;
              row[(i*3)+1] = 0;
              row[(i*3)+2] = 255;
              break;
         case 5:
              row[(i*3)+0] = 255;
              row[(i*3)+1] = 255;
              row[(i*3)+2] = 0;
              break;
         case 6:
              row[(i*3)+0] = 255;
              row[(i*3)+1] = 0;
              row[(i*3)+2] = 255;
              break;
         case 7:
              row[(i*3)+0] = 0;
              row[(i*3)+1] = 255;
              row[(i*3)+2] = 255;
              break;
         case 8:
              row[(i*3)+0] = 233;
              row[(i*3)+1] = 131;
              row[(i*3)+2] = 0;
              break;
	 default:
              row[(i*3)+0] = 0;
              row[(i*3)+1] = 0;
              row[(i*3)+2] = 0;
              break;
      }
    }
    png_write_row(png, row);
   }
  png_write_end(png, NULL);
  printf("done writing file\n");

  free(row);
  fclose(fp);
}

int main(int argc, char **argv)
{
   struct timeval timecheck;
   long start,end;
   long total_setup_time = 0;
   long total_sim_time = 0;
   long total_IO_time = 0;
   


   gettimeofday(&timecheck,NULL);
   start = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000;


   srand(0);
   
   
   int sz_x = 1000;
   int sz_y = 500;
   char filename[sizeof "./images/file00000.png"];
   int img_count = 0;

   float pop_prob = 0.75;
   float rumor_prob = 0.25;
   int num_loops = 1000;
   int NUM_RUMORS = 7;

   
   int full_size_with_boarders = (sz_x+2)*(sz_y+2);
   char * world_mem = (char *) malloc(2*full_size_with_boarders*sizeof(char));
   for (int i=0; i<full_size_with_boarders; i++)
   	world_mem[i] = 0; 

   char ** my_world[2];
   my_world[0] = (char **) malloc((sz_y+2) * sizeof(char*));
   my_world[1] = (char **) malloc((sz_y+2) * sizeof(char*));
   for (int which=0; which < 2; which++) {
      for (int r=0; r<(sz_y+2); r++)
         my_world[which][r] = &world_mem[(r*(sz_x+2))+which*full_size_with_boarders];
   }
   printf("After Allocate\n");

   int which = 0;
   

   for (int r=0; r<sz_y+2; r++)
   	for (int c=0; c<sz_x+2; c++)
	{
	   float rd = ((float) rand())/(float)RAND_MAX;
           if (rd < pop_prob) 
        	my_world[which][r][c] = 1;
 	}
   printf("After Inicialize\n");
   
   
   for (int u; u < NUM_RUMORS; u++) {
      int r = (int) ((float)rand())/(float)RAND_MAX*(sz_y+2);
      int c = (int) ((float)rand())/(float)RAND_MAX*(sz_x+2);
      my_world[which][r][c] = u+2; 
      printf("Starting a Rumor %d, %d = %d\n",r,c,u+2); 
   }


   gettimeofday(&timecheck, NULL);
   end = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000;
   total_setup_time = (end-start);




 
  
   printf("After Start location picked \n");

  
   for(int t=0; t<num_loops;t++) {
 
         gettimeofday(&timecheck,NULL);
         start = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000;
   
       
        for (int c=1; c<sz_x+1; c++){
           my_world[which][0][c] = my_world[which][sz_y][c];
           my_world[which][sz_y+1][c] = my_world[which][1][c];
        }
   	for (int r=1; r<sz_y+1; r++){
           my_world[which][r][0] = my_world[which][r][sz_x];
           my_world[which][r][sz_x+1] = my_world[which][r][1];
        }
             
   	printf("Step %d\n",t);
        int rumor_counts[NUM_RUMORS+2];
   	for (int r=1; r<sz_y+1; r++) {
           for (int c=1; c<sz_x+1; c++)
	   { 
	     my_world[!which][r][c] = my_world[which][r][c];
	     if (my_world[which][r][c] >0) { 
		for(int n=0;n<NUM_RUMORS+2;n++)
                   rumor_counts[n] = 0;
                rumor_counts[my_world[which][r-1][c]]++; 
                rumor_counts[my_world[which][r+1][c]]++; 
                rumor_counts[my_world[which][r][c-1]]++; 
                rumor_counts[my_world[which][r][c+1]]++; 

		float rd = ((float) rand())/(float)RAND_MAX; 
		float my_prob = 0;
		for(int n=2;n<NUM_RUMORS+2;n++) {
		   if (rumor_counts[n] > 0) {
			my_prob += rumor_prob*rumor_counts[n]; 
	           	if(rd <= my_prob) {
	      			
		      		my_world[!which][r][c] = n;
 		      		break;
	           	}
		   } 
		}
             }
	   }
        }		
	which = !which;
   
        gettimeofday(&timecheck, NULL);
        end = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000;

        total_sim_time = total_sim_time + (end - start);



	if(t%10==0) { 
           gettimeofday(&timecheck,NULL);
           start = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000;

      
           sprintf(filename, "./images/file%05d.png", img_count);
   	   writeworld(filename, my_world[0], sz_x, sz_y);
           img_count++;
         
         gettimeofday(&timecheck, NULL);
         end = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000;

         total_IO_time = total_IO_time + (end - start);        
	}
   } 


   printf("%ld milliseconds elapsed in setup\n", total_setup_time);
   printf("%ld milliseconds elapsed in simulation\n", total_sim_time);
   printf("%ld milliseconds elapsed in IO\n", total_IO_time);
 
   printf("After Loop\n");
   free(my_world[0]);
   free(my_world[1]);
   printf("After Clean up\n");
   return(0);
}


