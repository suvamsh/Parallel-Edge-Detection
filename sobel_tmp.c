//Suvamsh Shivaprasad - ss56236
//Ankit Tandon - 
//sources - stackoverflow

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


struct image
{
	int height, width, size;
	unsigned int data_offset;
	unsigned char *img_buf;
	unsigned char *omg_buf;
	unsigned char bits_per_pixel;
	unsigned char *source_fname, *target_fname;
};

//int read_bmp(unsigned char *, char *, int *, int *, int *);
//int sobel(unsigned char *, unsigned char *);
//int write_bmp(struct image *);
//sobel operator mask
 // sobel mask
int sobel_mask[2][3][3] = 
{ 
	{{-1,-0,1},
	{-2 , 0, 2}, 
	{-1 , 0, 1}},
					  
	{{-1, 2, 1}, 
	{0, 0, 0}, 
	{1, 2, -1}} 
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
//unsigned char *img_in = NULL;
//int bpp; //bits per pixel

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
	//*height = *(int *)&bmp[18];
	//*width = *(int *)&bmp[22];
	img->height = *(int *)&bmp[18];
	img->width = *(int *)&bmp[22];
	printf("height = %d, width43534 = %d\n",img->height, img->width);
	//as there are 3 values [RGB] per pixel assuming input image is color
	img->size = 3 * img->height * img->width; 
	img->img_buf = (unsigned char *) malloc((size_t) img->size);

//	img_out = (unsigned char *) malloc((size_t) size);
	if(!img->img_buf) //&& !img_out)
	{
		printf("Could not allocate space for image\n");
		exit(-1);
	}
	//read bpp
	fseek(f, 28, SEEK_SET);
	fread(&img->bits_per_pixel, sizeof(unsigned int), 1, f);

	//seek to read the actual image data from bmp
	fseek(f, 10, SEEK_SET);
	fread(&img->data_offset, sizeof(unsigned int), 1, f);
	fseek(f, img->data_offset, SEEK_SET);

	//read image data into buffer 'img_in'
	fread(img->img_buf, sizeof(unsigned char), (size_t)(long) img->size, f);
	fclose(f);
	return 0;
}

int write_bmp(struct image *img) {
   unsigned int file_size; // file size
   FILE *fp_t = fopen(img->target_fname, "wb");
   if (fp_t == NULL) {
     printf("fopen fname_t error\n");
     return -1; 
   }   
	 printf("Opened file for writing~\n");  
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
   fwrite(header, sizeof(unsigned char), img->data_offset, fp_t);
  printf("Wrote header!\n"); 
   // write image
   fwrite(img->omg_buf, sizeof(unsigned char), (size_t)(long)img->size, fp_t);
    
   //fclose(fp_s);
   fclose(fp_t);
    
   return 0;
 }


void sobel_filter(struct image *img)
{
	int i=1,j=1;
	unsigned char *p;
	img->omg_buf = (unsigned char *) malloc((size_t) img->size);
	for( i = 1; i < ( img->height - 1 ); i++ )
	{	
		for( j = 1; j < ( img->width - 1 ); j++ )
		{
//			img->omg_buf[i-1][j-1] = (short) img->img_buf[i-1][j+1] - ( short )img->img_buf[i-1][j-1] +
//																2*((short) img->img_buf[i][j+1] - (short) img->img_buf[i][j-1]) +
//																(short)img->img_buf[i+1][j+1] - ( short )img->img_buf[i+1][j-1];
			p = (img->omg_buf + ((i-1)*img->width) +(j-1));
*p = (unsigned char) *(img->img_buf + ((i-1)*img->width) + (j+1)) - ( unsigned char )*(img->img_buf + ((i-1)*img->width)+ (j-1)) +
																2*((unsigned char) *(img->img_buf + i*img->width + (j+1)) - (unsigned char) *(img->img_buf + i*img->width + (j-1))) +
																(unsigned char) *(img->img_buf + (i+1)*img->width + (j+1)) - ( unsigned char ) *(img->img_buf + (i+1)*img->width + (j-1));

		}
	}
}


int main()
{
	struct image *img = (struct image *) malloc( sizeof(struct image));
	img->source_fname = "lena.bmp";	
	int height=0, width=0;
	static unsigned char  *img_out = NULL;
	unsigned int data_offs;
	printf("welcome to sobel filter!\n");

//read image into struct
	read_bmp(img);

	int i=0;
	for(; i < img->size; i++)
//		printf("%d ",img->img_buf[i]);
	img->target_fname = "lena_sobel.bmp";
	printf("height = %d, width = %d\n", img->height, img->width);
	//apply sobel
	printf("About to sobel!\n");
	sobel_filter(img);

	//write image to file
	write_bmp(img);	

	return 0;
}
 


