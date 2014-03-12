//
//  irani.c
//  
//
//  Created by Clément on 07/03/2014.
//
//

// Program to register three images with the parametric method


#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "iio.h"

// auxiliary function to get the value of an image at any point (i,j)
// (points outisde the original domain get the value 0)
//
// x: image data
// w: width
// h: height
// i: horizontal position
// j: vertical position
//
// return value: color of the requested pixel
//
float getpixel_0(float *x, int w, int h, int i, int j)
{
	if (i < 0 || j < 0 || i >= w || j >= h)
		return 0;
	else
		return x[j*w+i];
}

//create parametric transformation matrix (quadratic)
void fill_matrix_deg2(float *X, int i, int j)
{
    float tmp[] = { 1,i,j,0,0,0,i*i,i*j,0,0,0,1,i,j,i*j,j*j };
    for (int k = 0; k<16; k++)
    {
        X[k]=tmp[k];
    }
}

// apply a translation to the given image
//
// in: input image
// w: width
// h: height
// dx: horizontal displacement
// dy: vertical displacement
// out: output image, to be filled-in
//
void apply_translation(float *out, int dx, int dy, float *in, int w, int h)
{
	for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
        {
            int ii = i - dx;
            int jj = j - dy;
            out[j*w+i] = getpixel_0(in, w, h, ii, jj);
        }
}

// compute the four directional derivatives
//
// in : input image
// w : width
// h : height
// out : four output images

void compute_directional_derivatives(float *im, int w, int h, float *out[4])
{
    float *dec[4];
    for (int k=0; k<4; k++)
    {
        out[k] = malloc(w*h*sizeof(float));
        dec[k] = malloc(w*h*sizeof(float));
    }


// create translated images
    apply_translation(dec[0], 1, 0, im, w, h);
    apply_translation(dec[1], 0, 1, im, w, h);
    apply_translation(dec[2], -1, 1, im, w, h);
    apply_translation(dec[3], -1, -1, im, w, h);

// compute derivatives
    for(int j = 0 ; j < h ; j++)
        for(int i = 0 ; i < w ; i++)
        {
            out[0][j*w+i] = (dec[0][j*w+i] - im[j*w+i])*(dec[1][j*w+i] - im[j*w+i]);
            out[1][j*w+i] = (dec[1][j*w+i] - im[j*w+i])*(dec[1][j*w+i] - im[j*w+i]);
            out[2][j*w+i] = (dec[2][j*w+i] - im[j*w+i])*(dec[2][j*w+i] - im[j*w+i])/2;
            out[3][j*w+i] = (dec[3][j*w+i] - im[j*w+i])*(dec[3][j*w+i] - im[j*w+i])/2;
        }
    
    free(dec[0]);
    free(dec[1]);
    free(dec[2]);
    free(dec[3]);
}

// warp the image according to a 8-parameter transformation
//
// im : input image
// p : parameter
// w : width
// h : height
// out : warped image

void warping_8_parameters(float *im, float p[8], int w, int h, float *out)
{
    for(int j = 0 ; j < h ; j++)
        for(int i = 0 ; i < w ; i++)
        {
            float u;
            float v;
            u=p[0]+p[1]*i+p[2]*j+p[6]*i*i+p[7]*i*j;
            v=p[3]+p[4]*i+p[5]*j+p[6]*i*j+p[7]*j*j;
                                   
            out[j*w+i]=interpolate_image(im, w, h, i+u, j+v);
                                   
        }
}

// compute the mean in a 11-pixel window centered on each pixel
//
// im : input image
// w : width
// h : height
// out : mean image
                                   
void mean_window_11(float *im, int w, int h, float *out)
{
    for(int j = 0 ; j < h ; j++)
        for(int i = 0 ; i < w ; i++)
        {
            float S=0;
        
            for(int k = -5; k <= 5 ; k++)
                for(int l = -5; l <= 5 ; l++)
                {
                    int ii=i+l;
                    int jj=j+k;
                    S = S + getpixel_0(im, w, h, ii, jj);
                }
            out[j*w+i]=S/121;
        }
}

//compute the norm 2 of the difference between a window and its mean for each pixel of an image
//
// im : input image
// mean : image of the means (obtained for example with mean_window_11)
// w : width
// h : height
//

void norm_2_window_11(float *im, float *mean, int w, int h, float *out)
{
    for(int j = 0 ; j < h ; j++)
        for(int i = 0 ; i < w ; i++)
        {
            float S=0;
            )
            for(int k = -5; k <= 5 ; k++)
                for(int l = -5; l <= 5 ; l++)
                {
                    int ii=i+l;
                    int jj=j+k;
                    S = S + (getpixel_0(im, w, h, ii, jj)-getpixel_0(mean, w, h, i, j))*(getpixel_0(im, w, h, ii, jj)-getpixel_0(mean, w, h, i, j));
                }
            out[j*w+i]=sqrt(S);
        }
}

// compute the correlation surface between two images on every pixel for a given displacement
//
// im1, im2 : input images
// w : width
// h : height
// u,v : coordinates of the displacement
// out : image of the correlation surface

void correlation_surface(float *im1, float *im2, int w, int h, int u, int v, float *out)
{
    mean_window_11(im1, w, h, mean1);
    mean_window_11(im2, w, h, mean2);
    norm_2_window_11(im1, mean1, w, h, norm1);
    norm_2_window_11(im2, mean2, w, h, norm2);
    
    for(int j = 0 ; j < h ; j++)
        for(int i = 0 ; i < w ; i++)
        {
            float S=0;
            int ii=i+u;
            int jj=j+v;
            
            for(int k = -5; k <= 5 ; k++)
                for(int l = -5; l <= 5 ; l++)
                {
                    
                    S = S + (getpixel_0(im1, w, h, i, j)-getpixel_0(mean1, w, h, i, j))*(getpixel_0(im2, w, h, ii, jj)-getpixel_0(mean2, w, h, ii, jj));
                }
            
            out[j*w+i] = S/(norm1[j*w+i]*norm2[jj*w+ii]);
        }
}

//compute the gradient of the correlation surface in (0,0)
//
// im : input image
// w, h : dimensions
// out : gradient image (a 2x1 vector for each pixel)

void grad_correlation_surface(float *im, int w, int h, float *out[2])
{
    correlation_surface(im, w, h, 0, 0, surf00);
    correlation_surface(im, w, h, 1, 0, surf10);
    correlation_surface(im, w, h, 0, 1, surf01);
    
    for(int j = 0 ; j < h ; j++)
        for(int i = 0 ; i < w ; i++)
        {
            out[0][j*w+i] = surf10[j*w+i]-surf00[j*w+i];
            out[1][j*w+i] = surf01[j*w+i]-surf00[j*w+i];
        }
    
}

// compute the hessian matrix of the correlation surface in (0,0)
//
// im : input image
// w, h : dimensions
// out : hessian image (a 2x2 matrix for each pixel)

void hessian_correlation_surface(float *im, int w, int h, float *out[4])
{
    correlation_surface(im, w, h, 0, 0, surf00);
    correlation_surface(im, w, h, 1, 0, surf10);
    correlation_surface(im, w, h, 0, 1, surf01);
    correlation_surface(im, w, h, -1, 0, surf_10);
    correlation_surface(im, w, h, 0, -1, surf0_1);
    correlation_surface(im, w, h, 1, 1, surf11);
    
    for(int j = 0 ; j < h ; j++)
        for(int i = 0 ; i < w ; i++)
        {
            out[0][j*w+i] = surf10[j*w+i]+surf_10[j*w+i]-2*surf00[j*w+i];
            out[1][j*w+i] = surf00[j*w+i]+surf11[j*w+i]-surf10[j*w+i]-surf01[j*w+i];
            out[2][j*w+i] = surf00[j*w+i]+surf11[j*w+i]-surf10[j*w+i]-surf01[j*w+i];
            out[3][j*w+i] = surf01[j*w+i]+surf0_1[j*w+i]-2*surf00[j*w+i];
        }
    
}

// compute A (square matrix with length = number of parameters, here 8) with the hessian and the matrix X(i,j)
//

void A_matrix_8(float *im, int w, int h, float out[64])
{
    hessian_correlation_surface(im, w, h, hess);
    
    for(int k = 0 ; k < 8 ; k++)
        for(int l = 0 ; l < 8 ; l++)
        {
            out[8*k+l] = 0;
        }
                
    for(int j = 0 ; j < h ; j++)
        for(int i = 0 ; i < w ; i++)
        {
            fill_matrix_deg2(X, i, j);
            
            for(int k = 0 ; k < 8 ; k++)
                for(int l = 0 ; l < 8 ; l++)
                {
                    out[8*k+l] = out[8*k+l] + X[k]*(X[l]*hess[0][j*x+i]+X[8+l]*hess[1][j*w+i])+X[8+k]*(X[l]*hess[2][j*w+i]+X[8+l]*hess[3][j*w+i]);
                }
        }
}


//compute B (vector with same length as the number of parameters, here 8) cith the gradient and the matrix X(i,j)
//

void B_matrix_8(float *im, int w, int h, float out[8])
{
    grad_correlation_surface(im, w, h, grad);
    
    for(int l = 0 ; l < 8 ; l++)
    {
        out[l] = 0;
    }
    
    for(int j = 0 ; j < h ; j++)
        for(int i = 0 ; i < w ; i++)
        {
            fill_matrix_deg2(X, i, j);
            
            for(int k = 0 ; k < 8 ; k++)
                {
                    out[k] = out[k] + grad[0][j*w+i]*X[k] + grad[1][j*w+i]*X[8+k];
                }
        }

}

//find displacement
//
// A, B : matrices computed with A_matrix and B_matrix

void delta_displacement_8(float *A, float *B, float out[8])
{
    inverse_matrix(A, A1);
    
    //matricial product between A-1 and B
    for(int j = 0 ; j < 8 ; j++)
    {
        out2[j] = 0;
        for(int k = 0 ; k < 8 ; k++)
        {
            out2[j] = out2[j]-A1[8*j+k]*B[k];
        }
    }
}

// main program

int main(int argc, char **argv)
{
    // process input arguments
    if (argc != 4) {
        frprintf(stderr, "usage:\n \t%s imageIn1 imageIn2 imageWarped2 \n");
        //                         0     1       2        3
    }
    
    char *filename_ImgIn1 = argv[1];
    char *filename_ImgIn2 = argv[2];
    char *filename_ImgOut = argv[3];
    
    //read input images
    int w, h;
    float *im1 = iio_read_image_float(filename_ImgIn1, &w, &h);
    float *im2 = iio_read_image_float(filename_ImgIn2, &w, &h);
    
    //compute derivatives
    compute_directional_derivatives(im1, w, h, der1);
    compute_directional_derivatives(im2, w, h, der2);
    
    //compute A and B for each derivative and sum it
    float *A[4];
    float *B[4];
    float *AA ;
    float *BB ;
    
    for(int i=0 ; i < 4 ; i++)
    {
        A_matrix_8(der1[i], w, h, A[i]);
        B_matrix_8(der2[i], w, h, B[i])
    }
    
    for(int j = 0 ; j < h ; j++)
        for(int i = 0 ; i < w ; i++)
        {
            AA[j*w+i]=A[0][j*w+i]+A[1][j*w+i]+A[2][j*w+i]+A[3][j*w+i];
            BB[j*w+i]=B[0][j*w+i]+B[1][j*w+i]+B[2][j*w+i]+B[3][j*w+i];
        }

    // compute displacement
    delta_displacement_8(AA, BB, delta);
    
    //allocate space for the output image
    float *out2 = malloc(w*h*sizeof(float));
    
    // warp image
    warping_8_parameters(im2, delta, w, h, out2);
    
    //save the output image
    iio_save_image_float(filename_ImgOut, &w, &h);
    
    //cleanup and exit
    free(out2);
    
    return 0;
}


