#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>

typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned int LONG;
struct tagBITMAPFILEHEADER
{
    WORD bfType;  //specifies the file type
    DWORD bfSize;  //specifies the size in bytes of the bitmap file
    WORD bfReserved1;  //reserved; must be 0
    WORD bfReserved2;  //reserved;must be 0
    DWORD bfOffBits;  //species the offset in bytes from the bitmapfileheader to the bitmap bits}
};

struct tagBITMAPINFOHEADER
{
    DWORD biSize;  //specifies the number of bytes required by the struct
    LONG biWidth;  //specifies width in pixels
    LONG biHeight;  //species height in pixels
    WORD biPlanes; //specifies the number of color planes, must be 1
    WORD biBitCount; //specifies the number of bit per pixel
    DWORD biCompression;//spcifies the type of compression
    DWORD biSizeImage;  //size of image in bytes
    LONG biXPelsPerMeter;  //number of pixels per meter in x axis
    LONG biYPelsPerMeter;  //number of pixels per meter in y axis
    DWORD biClrUsed;  //number of colors used by th ebitmap
    DWORD biClrImportant;  //number of colors that are important
};

unsigned char get_color(unsigned char *data, float x, float y, int imagewidth, int color_offset)
{
    int edit;
    int bplo = imagewidth*3;
    int padding = 0;
    if (bplo%4 != 0)
    {
        padding = (4-bplo%4);
    }
    edit = x*3 + (y*imagewidth*3) + padding*y + color_offset;
    return data[edit];
}

int set_color(float x, float y, int imagewidth, int color_offset)
{
    int edit;
    int bplo = imagewidth*3;
    int padding = 0;
    if (bplo%4 != 0)
    {
        padding = (4-bplo%4);
    }
    edit = x*3 + (y*imagewidth*3) + padding*y + color_offset;
    return edit;

}

int main(int argc, char *argv[])
{
    char message[80] = "Enter [imagefile] [brightness] [parallel][outputfile]\n";
    char message2[80] = "ex.) ./blendimages blendimages flowers.bmp jar.bmp 0.5 result3.bmp";
    char message1[80] = "Enter ratio between 0 and 1!\n";
    if(argc < 5)
    {
        printf("%s",message);
        printf("%s\n",message2);
        return 0;
    }
    
    FILE * foo;
    FILE * file;
    struct tagBITMAPFILEHEADER fh;
    struct tagBITMAPINFOHEADER fih;
    float ratio;
    ratio = atof(argv[2]);
    //ratio = 0.4;
    if(ratio > 1 || ratio < 0)
    {
        printf("%s",message1);
        printf("%s",message);
        printf("%s\n",message2);
        return 0;
    }
    int parallel = atoi(argv[3]);
    int g = 0;

    if ((foo = fopen(argv[1],"rb")) == NULL)
    {
        printf("imagefile1 is not a real file, please enter a valid file\n");
        printf("%s",message);
        printf("%s\n",message2);
        return 0;
    }
    fread(&fh.bfType,1,2,foo);
    fread(&fh.bfSize,1,4,foo);
    fread(&fh.bfReserved1,1,2,foo);
    fread(&fh.bfReserved2,1,2,foo);
    fread(&fh.bfOffBits,1,4,foo);
    fread(&fih,1,sizeof(struct tagBITMAPINFOHEADER),foo);
    unsigned char *data = (unsigned char*)mmap(NULL,fih.biSizeImage,PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,-1,0);
    fread(data,1,fih.biSizeImage,foo);
    fclose(foo);

    unsigned char *data3 = (unsigned char*)mmap(NULL,fih.biSizeImage,PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS,-1,0); //makes new picture the same resolution as the bigger image passed in
    
    fflush(0);
    clock_t start = clock();
    if (parallel == 1)
    { 
        int height = (int)(fih.biHeight/2);
        if (fork() == 0)
        {
            for(int y = 0; y < height; y++)
            {
                for (int x = 0; x < (fih).biWidth; x++)
                {
                    unsigned char pix = get_color(data, x, y, (fih).biWidth,0); //access blue for the larger one 
                    unsigned char pix1 = get_color(data, x, y, (fih).biWidth,1); //access green
                    unsigned char pix2 = get_color(data, x, y, (fih).biWidth,2); //access red

                    int new = (int)pix + (ratio*255);
                    if(new > 255)
                    {
                        new = 255;
                    }
                    int new1 = (int)pix1 + (ratio*255);
                    if(new1 > 255)
                    {
                        new1 = 255;
                    }
                    int new2 = (int)pix2 + (ratio*255);
                    if(new2 > 255)
                    {
                        new2 = 255;
                    }

                    int pos = set_color(x, y, (fih).biWidth,0);
                    int pos1 = set_color(x, y, (fih).biWidth,1);
                    int pos2 = set_color(x, y, (fih).biWidth,2);

                    data3[pos] = (unsigned char)new;
                    data3[pos1] = (unsigned char)new1;
                    data3[pos2] = (unsigned char)new2; 
                }
            }
            return 1;
        }
        else
        {
            wait(&g);
            for(int y = height; y < fih.biHeight; y++)
            {
                for (int x = 0; x < (fih).biWidth; x++)
                {
                    unsigned char pix = get_color(data, x, y, (fih).biWidth,0); //access blue for the larger one 
                    unsigned char pix1 = get_color(data, x, y, (fih).biWidth,1); //access green
                    unsigned char pix2 = get_color(data, x, y, (fih).biWidth,2); //access red

                    int new = (int)pix + (ratio*255);
                    if(new > 255)
                    {
                        new = 255;
                    }
                    int new1 = (int)pix1 + (ratio*255);
                    if(new1 > 255)
                    {
                        new1 = 255;
                    }
                    int new2 = (int)pix2 + (ratio*255);
                    if(new2 > 255)
                    {
                        new2 = 255;
                    }

                    int pos = set_color(x, y, (fih).biWidth,0);
                    int pos1 = set_color(x, y, (fih).biWidth,1);
                    int pos2 = set_color(x, y, (fih).biWidth,2);

                    data3[pos] = (unsigned char)new;
                    data3[pos1] = (unsigned char)new1;
                    data3[pos2] = (unsigned char)new2; 
                }
            }
        }
    }
    
    if (parallel == 0)
    {
        for(int y = 0; y < (fih).biHeight; y++)
        {
            for (int x = 0; x < (fih).biWidth; x++)
            {
                unsigned char pix = get_color(data, x, y, (fih).biWidth,0); //access blue for the larger one 
                unsigned char pix1 = get_color(data, x, y, (fih).biWidth,1); //access green
                unsigned char pix2 = get_color(data, x, y, (fih).biWidth,2); //access red

                int new = (int)pix + (ratio*255);
                if(new > 255)
                {
                    new = 255;
                }
                int new1 = (int)pix1 + (ratio*255);
                if(new1 > 255)
                {
                    new1 = 255;
                }
                int new2 = (int)pix2 + (ratio*255);
                if(new2 > 255)
                {
                    new2 = 255;
                }

                int pos = set_color(x, y, (fih).biWidth,0);
                int pos1 = set_color(x, y, (fih).biWidth,1);
                int pos2 = set_color(x, y, (fih).biWidth,2);

                data3[pos] = (unsigned char)new;
                data3[pos1] = (unsigned char)new1;
                data3[pos2] = (unsigned char)new2; 

            }
        }
    }
    clock_t end = clock();
    double total_t = ((double)(end-start));//CLOCKS_PER_SEC;
    printf("total time taken: %f\n",total_t);
    
    file = fopen(argv[4], "wb+");
    fwrite(&fh.bfType,1,2,file);
    fwrite(&fh.bfSize,1,4,file);
    fwrite(&fh.bfReserved1,1,2,file);
    fwrite(&fh.bfReserved2,1,2,file);
    fwrite(&fh.bfOffBits,1,4,file);
    fwrite(&fih,1,sizeof(struct tagBITMAPINFOHEADER),file);
    fwrite(data3,1,(fih).biSizeImage,file);
    fclose(file);

    munmap(data,fih.biSizeImage);
    munmap(data3,fih.biSizeImage);

}