#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "png_util.h"
#include <cuda.h>
#define min(X,Y) ((X) < (Y) ? (X) : (Y))
#define max(X,Y) ((X) > (Y) ? (X) : (Y))

#define W 500
#define H 500


#define CUDA_CALL(x) {cudaError_t cuda_error__ = (x); if (cuda_error__) printf("CUDA error: " #x " returned \"%s\"\n", cudaGetErrorString(cuda_error__));}

__global__ void calcAccel(double dx2inv, double dy2inv, int nx, int ny, double *d_z, double *d_a){
    int i = blockDim.x * blockIdx.x + threadIdx.x;
    int j = blockDim.y * blockIdx.y + threadIdx.y;
    
    int P = i + j*nx;           
    int n = i + (j+1)*nx;       
    int s = i + (j-1)*nx;       
    int e = (i+1) + j*nx;      
    int w = (i-1) + j*nx; 

    if (i > 0 && i < nx-1 && j > 0 && j < ny-1){
        d_a[P] = dx2inv*(d_z[s] + d_z[n] - 2.0*d_z[P]) + dy2inv*(d_z[w] + d_z[e] - 2.0*d_z[P]);
d_a[P] = 0.5*d_a[P];
    }
}

__global__ void calcVelAndPos(double dt, int nx, int ny, double *d_z, double *d_v, double *d_a){
    int i = blockDim.x * blockIdx.x + threadIdx.x;
    int j = blockDim.y * blockIdx.y + threadIdx.y;

    int P = i + j*nx;

    d_v[P] = d_v[P] + dt*d_a[P];
    d_z[P] = d_z[P] + dt*d_v[P];
}

int main(int argc, char ** argv) {
    int nx = W;
    int ny = H;
    int nt = 10000; 
    int frame=0;
    //int nt = 1000000;
    int r,c,it;
    double *d_z, *d_a, *d_v;
    double dx,dy,dt;
    double max,min;
    double tmax;
    double dx2inv, dy2inv;
    char filename[sizeof "./images/file00000.png"];

    image_size_t sz; 
    sz.width=nx;
    sz.height=ny;

    //make mesh
    double z[W][H];

    //Velocity
    double v[W][H];

    //Accelleration
    double a[W][H];

    //output image
    char * o_img = (char *) malloc(sz.width*sz.height*sizeof(char));
    char ** output = (char **) malloc(sz.height * sizeof(char*));
    for (int r=0; r<sz.height; r++)
        output[r] = &o_img[r*sz.width];

    max=10.0;
    min=0.0;
    dx = (max-min)/(double)(nx-1);
    dy = (max-min)/(double)(ny-1);
    
    tmax=20.0;
    dt= (tmax-0.0)/(double)(nt-1);

    double x,y; 
    for (r=0;r<ny;r++)  {
        for (c=0;c<nx;c++)  {
        x = min+(double)c*dx;
        y = min+(double)r*dy;
            z[r][c] = exp(-(sqrt((x-5.0)*(x-5.0)+(y-5.0)*(y-5.0))));
            a[r][c] = 0.0;
            v[r][c] = 0.0;
}
    }
    
    dx2inv=1.0/(dx*dx);
    dy2inv=1.0/(dy*dy);


    //We have initialized everthing, so now we want to do some memalloc and memcopying
    CUDA_CALL(cudaMalloc((void **)&d_z, nx*ny*sizeof(double)));
    CUDA_CALL(cudaMalloc((void **)&d_v, nx*ny*sizeof(double)));
    CUDA_CALL(cudaMalloc((void **)&d_a, nx*ny*sizeof(double)));

    CUDA_CALL(cudaMemcpy(d_z,z,nx*sizeof(double)*ny,cudaMemcpyHostToDevice));
    CUDA_CALL(cudaMemcpy(d_v,v,nx*sizeof(double)*ny,cudaMemcpyHostToDevice));
    CUDA_CALL(cudaMemcpy(d_a,a,nx*sizeof(double)*ny,cudaMemcpyHostToDevice));

    //make dim3's
    int block_size = 32;
    int nblocks_x = nx/block_size;
    int nblocks_y = ny/block_size;
    dim3 dimGrid(nblocks_x, nblocks_y, 1);
    dim3 dimBlock(block_size, block_size, 1);



    for(it=0;it<nt-1;it++) {
    //printf("%d\n",it);
        
            calcAccel<<<dimGrid,dimBlock>>>(dx2inv, dy2inv, nx, ny, d_z, d_a);
            calcVelAndPos<<<dimGrid, dimBlock>>>(dt, nx, ny, d_z, d_v, d_a);

    if (it % 100 ==0)
    {
            CUDA_CALL(cudaMemcpy(z, d_z, nx*ny*sizeof(double), cudaMemcpyDeviceToHost));
            double mx,mn;
            mx = -999999;
            mn = 999999;
            for(r=0;r<ny;r++)
                for(c=0;c<nx;c++){
                   mx = max(mx, z[r][c]);
                   mn = min(mn, z[r][c]);
            }
            for(r=0;r<ny;r++)
                for(c=0;c<nx;c++)
                    output[r][c] = (char) round((z[r][c]-mn)/(mx-mn)*255);

            sprintf(filename, "./images/file%05d.png", frame);
            printf("Writing %s\n",filename);    
            write_png_file(filename,(unsigned char *) o_img,sz);
        frame+=1;
        }

    }
    
    double mx,mn;
    mx = -999999;
    mn = 999999;
    for(r=0;r<ny;r++)
        for(c=0;c<nx;c++){
       mx = max(mx, z[r][c]);
       mn = min(mn, z[r][c]);
        }

    printf("%f, %f\n", mn,mx);

    for(r=0;r<ny;r++)
        for(c=0;c<nx;c++){  
       output[r][c] = (char) round((z[r][c]-mn)/(mx-mn)*255);  
    }

    sprintf(filename, "./images/file%05d.png", it);
    printf("Writing %s\n",filename);    
    //Write out output image using 1D serial pointer
    write_png_file(filename,(unsigned char *) o_img,sz);
    return 0;
}
