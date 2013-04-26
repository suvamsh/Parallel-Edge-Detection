//Suvamsh Shivaprasad - ss56236
//Ankit Tandon - at24473 

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct image
{
	int height, width;
	double size;
	unsigned int data_offset;
	unsigned char *img_buf;
	unsigned char *omg_buf;
	unsigned char *bw_buf;
	unsigned char bits_per_pixel;
	unsigned short byte_per_pixel;
	unsigned char *source_fname, *target_fname;
};

//sobel operator mask
int sobel_mask[2][3][3] = 
{ 
	{{-1,-2,-1},
	{0 , 0, 0}, 
	{1 , 2, 1}},
					  
	{{-1, 0, 1}, 
	{-2, 0, 2}, 
	{-1, 0, 1}} 
};

 // bitmap header
unsigned char header[54] = {
	0x42,        // identity : B
	0x4d,        // identity : M
	0, 0, 0, 0,  // file size
	0, 0,        // reserved1
	0, 0,        // reserved2
	54, 0, 0, 0, // RGB data offset
	40, 0, 0, 0, // struct BITMAPINFOHEADER size
	0, 0, 0, 0,  // bmp width
	0, 0, 0, 0,  // bmp height
	1, 0,        // planes
	24, 0,       // bit per pixel
	0, 0, 0, 0,  // compression
	0, 0, 0, 0,  // data size
	0, 0, 0, 0,  // h resolution
	0, 0, 0, 0,  // v resolution 
	0, 0, 0, 0,  // used colors
	0, 0, 0, 0   // important colors
};

//function to read bmp file 
int read_bmp(struct image *img)
{
	unsigned char bmp[54];
	FILE *f = NULL;

	f = fopen(img->source_fname, "rb");
	if( !f )
	{
		printf("Error opening file!\n");
		exit(-1);
	}
	fread(bmp, sizeof(unsigned char), 54, f);
	
	img->height = *(int *)&bmp[18];
	img->width = *(int *)&bmp[22];
	
	//read bpp
	fseek(f, 28, SEEK_SET);
	fread(&img->bits_per_pixel, sizeof(unsigned int), 1, f);
	img->byte_per_pixel = img->bits_per_pixel / 8;
	
	//as there are 3 values [RGB] per pixel assuming input image is color
	img->size =  img->byte_per_pixel * (double) img->height * (double) img->width ; 
	
	//allocate space for input and output buffer
	img->img_buf = (unsigned char *) malloc((size_t) img->size);
	img->omg_buf = (unsigned char *) malloc((size_t) img->size);
	
	if(!img->img_buf) 
	{
		printf("Could not allocate space for input buffer\n");
		exit(-1);
	}

	if(!img->omg_buf) 
	{
		printf("Could not allocate space for output buffer\n");
		exit(-1);
	}

	//seek to read the actual image data from bmp
	fseek(f, 10, SEEK_SET);
	fread(&img->data_offset, sizeof(unsigned int), 1, f);
	fseek(f, img->data_offset, SEEK_SET);

	//read image data into buffer 'img_in'
	fread(img->img_buf, sizeof(unsigned char), (size_t)(long) img->size, f);
	fclose(f);
	return 0;
}

//function to write bmp file with correct header
int write_bmp(struct image *img) 
{
   unsigned int file_size; // file size
   FILE *f = fopen(img->target_fname, "wb");
   if (f == NULL) {
     printf("fopen fname_t error\n");
     return -1; 
   }   
	 
	 // file size  
   file_size = img->size + img->data_offset;
	 header[2] = (unsigned char)(file_size & 0x000000ff);
   header[3] = (file_size >> 8)  & 0x000000ff;
   header[4] = (file_size >> 16) & 0x000000ff;
   header[5] = (file_size >> 24) & 0x000000ff;
    
   // width
   header[18] = img->width & 0x000000ff;
   header[19] = (img->width >> 8)  & 0x000000ff;
   header[20] = (img->width >> 16) & 0x000000ff;
   header[21] = (img->width >> 24) & 0x000000ff;
    
   // height
   header[22] = img->height &0x000000ff;
   header[23] = (img->height >> 8)  & 0x000000ff;
   header[24] = (img->height >> 16) & 0x000000ff;
   header[25] = (img->height >> 24) & 0x000000ff;
    
   // bit per pixel
   header[28] = img->bits_per_pixel;
    
   // write header
   fwrite(header, sizeof(unsigned char), img->data_offset, f);
   
	 // write image
   fwrite(img->omg_buf, sizeof(unsigned char), (size_t)(long)img->size, f);
    
   fclose(f);
    
   return 0;
 }


//function to apply sobel filter with threshold = lim
int sobel(struct image *img, double lim)
{
	unsigned int i, v, u, x, y;
	unsigned char r, g, b;
	double weight[2] = {0.0};
	double tot;
	
	for(y=0; y < img->height; ++y)
	{
		for(x=0; x < img->width; ++x)
		{
			for(i=0; i < 2; ++i)
			{
				weight[i] = 0.0;
				//loops to apply mask
				for(v = -1; v != 2; ++v)
				{
					for(u = -1; u != 2; ++u)
					{
						if(x + u >= 0 && x + u < img->width && y + v >= 0 && y + v < img->height)
						{
							r = *(img->img_buf + img->byte_per_pixel * (img->width * (y+v) + (x+u)) + 2);
							g = *(img->img_buf + img->byte_per_pixel * (img->width * (y+v) + (x+u)) + 1);
							b = *(img->img_buf + img->byte_per_pixel * (img->width * (y+v) + (x+u)) + 0);
							weight[i] += ((r+g+b)/3) * sobel_mask[i][u + 1][v + 1];
						}
					}
				}
			}//end of i loop
			tot = 0.0;
			//compute sobel value using pythogoras theorem
			for(i=0 ; i < 2; ++i)
			{
				tot += weight[i] * weight[i];
			}
			tot = sqrt(tot);

			if( tot-lim >= 0)
			{
				//if greater then make all black
				*(img->omg_buf +  img->byte_per_pixel*(img->width * y + x) + 2) = 0;
				*(img->omg_buf +  img->byte_per_pixel*(img->width * y + x) + 1) = 0;
				*(img->omg_buf +  img->byte_per_pixel*(img->width * y + x) + 0) = 0;
			}
			else
			{
				//if less then amke all white
				*(img->omg_buf + img->byte_per_pixel*(img->width * y + x) + 2) = 255;
				*(img->omg_buf + img->byte_per_pixel*(img->width * y + x) + 1) = 255;
				*(img->omg_buf + img->byte_per_pixel*(img->width * y + x) + 0) = 255;
			}
		}
	}
	return 0;
}






int main()
{
	struct image *img = (struct image *) malloc( sizeof(struct image));
	img->source_fname = "lena.bmp";	
	img->target_fname = "lena_sobel.bmp";
	int height=0, width=0;
	printf("Welcome to Sobel Filter!\n");

	//read image into struct
	read_bmp(img);

	printf("Image: %s\nHeight: %d\nWidth: %d\nSize: %lf\n",img->source_fname, img->height, img->width, img->size);
	//apply sobel
	printf("Applying sobel...");
	sobel(img, 90.0);
	printf("Done\n");
	//write image to file
	write_bmp(img);	
	free(img);
	return 0;
}
 
/*
void sobel_filter(struct image *img)
{
	int i=0,j=0, roff = 0, coff = 0, weight = 0, a=0, b=0;
	unsigned char *p, x, y;
	unsigned char p1, p2, p3, p4, p5, p6, p7, p8, p9;
	printf("Size in sobel filter = %lf\n", (double) img->size);
	printf("Width = %d\n", img->width);
	img->omg_buf = (unsigned char *) malloc((size_t) img->size);
	int dx[3][3] = {{1,0,-1},{2,0,-2},{1,0,-1}};
	int dy[3][3] = {{1,2,1},{0,0,0},{-1,-2,-1}};
	
	//ATTEMPT 3
	for(i=1; i < img->height*3; i++)
	{
		for(j=3; j < img->width*3; j+=3)
		{
			p1 = *(img->img_buf + ((i-1)*img->width) + (j-3));
			p2 = *(img->img_buf + ((i-1)*img->width) + (j));
			p3 = *(img->img_buf + ((i-1)*img->width) + (j+3));
			p4 = *(img->img_buf + (i*img->width) + (j-3));
			//p5 = *(img->img_buf + (i*img->width) + j);
			p6 = *(img->img_buf + (i*img->width) + (j+3));
			p7 = *(img->img_buf + ((i+1)*img->width) + (j-3));
			p8 = *(img->img_buf + ((i+1)*img->width) + j);		
			p9 = *(img->img_buf + ((i+1)*img->width) + (j+3));
			
			//X = (p1+(p2+p2)+p3-p7-(p8+p8)-p9) 
			x = (p1 + (p2*2) + p3 - p7 - (p8*2) - p9);

			//Y = (p3+(p6+p6)+p9-p1-(p4+p4)-p7)
			y = (p3 + (p6*2) + p9 - p1 - (p4*2) - p7);
			
			p = img->omg_buf + i*img->width*3 + j;
			*p = sqrt( x*x + y*y);
		}
	}
}*/	
	//ATTEMPT 2
	/*for( i=0, a=0; i < img->height*3; i++, a++)
	{
		for( j=0, b=0; j < img->width*3; j++, b++)
		{
			x = 0; y = 0;
			if( (i > 0) && ( i < (img->height*3 - 1)) && (j > 0) && (j < (img->width*3 -1)))
			{
				for(roff = -1; roff <=1; roff++)
				{
					for(coff = -1; coff <= 1; coff++)
					{
						x += (int) *(img->img_buf + (i*img->width + roff ) + (j + coff)) * dx[roff+1][coff+1];//sobel_mask[0][1+roff][1+coff];
						y += (int) *(img->img_buf + (i*img->width + coff) + (j + coff)) * dy[roff+1][coff+1];//sobel_mask[1][1+roff][1+coff];
					}
				}
				weight = abs(x) + abs(y);
				weight = (weight > 255)? 255 : weight;
				p = img->omg_buf + i*img->width + j;
				*p = weight;
			}
		}
	}
*/
//ATTEMPT 1
/*	for( i = 1; i < ( img->height - 1 ); i++ )
	{	
		for( j = 1; j < ( img->width - 1 ); j++ )
		{
			p = (img->omg_buf + ((i-1)*img->width) +(j-1));
*p = (unsigned char) *(img->img_buf + ((i-1)*img->width) + (j+1)) - ( unsigned char )*(img->img_buf + ((i-1)*img->width)+ (j-1)) +
																2*((unsigned char) *(img->img_buf + i*img->width + (j+1)) - (unsigned char) *(img->img_buf + i*img->width + (j-1))) +
																(unsigned char) *(img->img_buf + (i+1)*img->width + (j+1)) - ( unsigned char ) *(img->img_buf + (i+1)*img->width + (j-1));

		}
	}
*/

//function to convert RGB->Grayscale
/*void gray(struct image *img)
{
	int i,j, bw_counter;
	img->bw_buf = (unsigned char *) malloc((size_t) img->size1);
	unsigned char *p, *q, r, g, b, gray;
	for(i = 0; i < img->height; i++)
	{
		for(j=0, bw_counter=0; j < img->width*3; j+=3, bw_counter++)
		{
			p = img->img_buf + i*img->width*3 + j;
			b = *p;
			g = *(p+1);
			r = *(p+2);
			gray = 0.299*r + 0.587*g + 0.114*b;
			q = img->bw_buf + i*img->width + j;
			*q = gray;
		}
	}
	//img->bits_per_pixel = 8;
}
*/
