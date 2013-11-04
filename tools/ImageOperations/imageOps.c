#include "imageOps.h"
#include <stdio.h>
#include <stdlib.h>


#define MEMPLACE1(x,y,width) ( y * ( width  ) + x )
#define MEMPLACE3(x,y,width) ( y * ( width * 3 ) + x*3 )


int saveRawImageToFile(char * filename,char * pixels , unsigned int width , unsigned int height , unsigned int channels , unsigned int bitsperpixel)
{
    //fprintf(stderr,"saveRawImageToFile(%s) called\n",filename);

    if ( (width==0) || (height==0) || (channels==0) || (bitsperpixel==0) ) { fprintf(stderr,"saveRawImageToFile(%s) called with zero dimensions\n",filename); return 0;}
    if(pixels==0) { fprintf(stderr,"saveRawImageToFile(%s) called for an unallocated (empty) frame , will not write any file output\n",filename); return 0; }
    if (bitsperpixel>16) { fprintf(stderr,"PNM does not support more than 2 bytes per pixel..!\n"); return 0; }

    FILE *fd=0;
    fd = fopen(filename,"wb");

    if (fd!=0)
    {
        unsigned int n;
        if (channels==3) fprintf(fd, "P6\n");
        else if (channels==1) fprintf(fd, "P5\n");
        else
        {
            fprintf(stderr,"Invalid channels arg (%u) for SaveRawImageToFile\n",channels);
            fclose(fd);
            return 1;
        }

        char output[256]={0};
      /*GetDateString(output,"TIMESTAMP",1,0,0,0,0,0,0,0);
fprintf(fd, "#%s\n", output );*/

        fprintf(fd, "#TIMESTAMP %u\n",GetTickCount());


        fprintf(fd, "%d %d\n%u\n", width, height , simplePow(2 ,bitsperpixel)-1);

        float tmp_n = (float) bitsperpixel/ 8;
        tmp_n = tmp_n * width * height * channels ;
        n = (unsigned int) tmp_n;

        fwrite(pixels, 1 , n , fd);
        fflush(fd);
        fclose(fd);
        return 1;
    }
    else
    {
        fprintf(stderr,"SaveRawImageToFile could not open output file %s\n",filename);
        return 0;
    }
    return 0;
}


int mixbltRGB(unsigned char * target,  unsigned int tX,  unsigned int tY , unsigned int targetWidth , unsigned int targetHeight ,
              unsigned char * source , unsigned int sX, unsigned int sY  , unsigned int sourceWidth , unsigned int sourceHeight ,
              unsigned int width , unsigned int height)
{
  if ( (target==0)||(source==0) ) { return 0; }
  if ( (width==0)&&(height==0) ) { return 0; }
  if ( (sourceWidth==0)&&(sourceHeight==0) ) { return 0; }

  //Check for bounds -----------------------------------------
  if (tX+width>=targetWidth) { width=targetWidth-tX;  }
  if (tY+height>=targetHeight) { height=targetHeight-tY;  }

  if (sX+width>=sourceWidth) { width=sourceWidth-sX;  }
  if (sY+height>=sourceHeight) { height=sourceHeight-sY;  }
  //----------------------------------------------------------

  unsigned char * sourcePTR; unsigned char * sourceLineLimitPTR; unsigned char * sourceLimitPTR; unsigned int sourceLineSkip;
  unsigned char * targetPTR;  unsigned char * targetLimitPTR;  unsigned int targetLineSkip;


  sourcePTR      = source+ MEMPLACE3(sX,sY,sourceWidth);
  sourceLimitPTR = source+ MEMPLACE3((sX+width),(sY+height),sourceWidth);
  sourceLineSkip = (sourceWidth-width) * 3;
  sourceLineLimitPTR = sourcePTR + (width*3);
  fprintf(stderr,"SOURCE (RGB %u/%u)  Starts at %u,%u and ends at %u,%u\n",sourceWidth,sourceHeight,sX,sY,sX+width,sY+height);

  targetPTR      = target + MEMPLACE3(tX,tY,targetWidth);
  targetLimitPTR = target + MEMPLACE3((tX+width),(tY+height),targetWidth);
  targetLineSkip = (targetWidth-width) * 3;
  fprintf(stderr,"TARGET (RGB %u/%u)  Starts at %u,%u and ends at %u,%u\n",targetWidth,targetHeight,tX,tY,tX+width,tY+height);

  unsigned int color = 0;

  while (sourcePTR < sourceLimitPTR)
  {
     while (sourcePTR < sourceLineLimitPTR)
     {
        if (*sourcePTR==0) { color = *targetPTR; } else { color = (unsigned int) ( *targetPTR + *sourcePTR ) / 2; }
        *targetPTR =  (unsigned char ) color;
        ++targetPTR; ++sourcePTR;

        if (*sourcePTR==0) { color = *targetPTR; } else { color = (unsigned int) ( *targetPTR + *sourcePTR ) / 2; }
        *targetPTR =  (unsigned char ) color;
        ++targetPTR; ++sourcePTR;

        if (*sourcePTR==0) { color = *targetPTR; } else { color = (unsigned int) ( *targetPTR + *sourcePTR ) / 2; }
        *targetPTR =  (unsigned char ) color;
        ++targetPTR; ++sourcePTR;
     }


    sourceLineLimitPTR+= sourceWidth*3;//*3;
    targetPTR+=targetLineSkip;
    sourcePTR+=sourceLineSkip;
  }
 return 1;
}





int bitbltRGB(unsigned char * target,  unsigned int tX,  unsigned int tY , unsigned int targetWidth , unsigned int targetHeight ,
              unsigned char * source , unsigned int sX, unsigned int sY  , unsigned int sourceWidth , unsigned int sourceHeight ,
              unsigned int width , unsigned int height)
{
  if ( (target==0)||(source==0) ) { return 0; }
  if ( (width==0)&&(height==0) ) { return 0; }
  if ( (sourceWidth==0)&&(sourceHeight==0) ) { return 0; }

  //Check for bounds -----------------------------------------
  if (tX+width>=targetWidth) { width=targetWidth-tX;  }
  if (tY+height>=targetHeight) { height=targetHeight-tY;  }

  if (sX+width>=sourceWidth) { width=sourceWidth-sX;  }
  if (sY+height>=sourceHeight) { height=sourceHeight-sY;  }
  //----------------------------------------------------------


  unsigned char * sourcePTR; unsigned char * sourceLineLimitPTR; unsigned char * sourceLimitPTR; unsigned int sourceLineSkip;
  unsigned char * targetPTR;  unsigned char * targetLimitPTR;  unsigned int targetLineSkip;


  sourcePTR      = source+ MEMPLACE3(sX,sY,sourceWidth);
  sourceLimitPTR = source+ MEMPLACE3((sX+width),(sY+height),sourceWidth);
  sourceLineSkip = (sourceWidth-width) * 3;
  sourceLineLimitPTR = sourcePTR + (width*3);
  fprintf(stderr,"SOURCE (RGB %u/%u)  Starts at %u,%u and ends at %u,%u\n",sourceWidth,sourceHeight,sX,sY,sX+width,sY+height);

  targetPTR      = target + MEMPLACE3(tX,tY,targetWidth);
  targetLimitPTR = target + MEMPLACE3((tX+width),(tY+height),targetWidth);
  targetLineSkip = (targetWidth-width) * 3;
  fprintf(stderr,"TARGET (RGB %u/%u)  Starts at %u,%u and ends at %u,%u\n",targetWidth,targetHeight,tX,tY,tX+width,tY+height);

  unsigned int color = 0;

  while (sourcePTR < sourceLimitPTR)
  {
     while (sourcePTR < sourceLineLimitPTR)
     {
        *targetPTR = *sourcePTR; ++targetPTR; ++sourcePTR;
        *targetPTR = *sourcePTR; ++targetPTR; ++sourcePTR;
        *targetPTR = *sourcePTR; ++targetPTR; ++sourcePTR;
     }

    sourceLineLimitPTR+= sourceWidth*3;//*3;
    targetPTR+=targetLineSkip;
    sourcePTR+=sourceLineSkip;
  }
 return 1;
}













int bitbltDepth(unsigned short * target,  unsigned int tX,  unsigned int tY  , unsigned int targetWidth , unsigned int targetHeight ,
                unsigned short * source , unsigned int sX,  unsigned int sY  , unsigned int sourceWidth , unsigned int sourceHeight ,
                unsigned int width , unsigned int height)
{
  if ( (target==0)||(source==0) ) { return 0; }
  if ( (width==0)&&(height==0) ) { return 0; }
  if ( (sourceWidth==0)&&(sourceHeight==0) ) { return 0; }

  //Check for bounds -----------------------------------------
  if (tX+width>=targetWidth) { width=targetWidth-tX;  }
  if (tY+height>=targetHeight) { height=targetHeight-tY;  }

  if (sX+width>=sourceWidth) { width=sourceWidth-sX;  }
  if (sY+height>=sourceHeight) { height=sourceHeight-sY;  }
  //----------------------------------------------------------

  unsigned short * sourcePTR;  unsigned short* sourceLineLimitPTR; unsigned short * sourceLimitPTR; unsigned int sourceLineSkip;
  unsigned short * targetPTR;  unsigned short * targetLimitPTR;    unsigned int targetLineSkip;


  sourcePTR      = source+ MEMPLACE1(sX,sY,sourceWidth);
  sourceLimitPTR = source+ MEMPLACE1((sX+width),(sY+height),sourceWidth);
  sourceLineSkip = (sourceWidth-width)  ;
  sourceLineLimitPTR = sourcePTR + (width);
  fprintf(stderr,"SOURCE (Depth %u/%u)  Starts at %u,%u and ends at %u,%u\n",sourceWidth,sourceHeight,sX,sY,sX+width,sY+height);

  targetPTR      = target + MEMPLACE1(tX,tY,targetWidth);
  targetLimitPTR = target + MEMPLACE1((tX+width),(tY+height),targetWidth);
  targetLineSkip = (targetWidth-width)  ;
  fprintf(stderr,"TARGET (Depth %u/%u)  Starts at %u,%u and ends at %u,%u\n",targetWidth,targetHeight,tX,tY,tX+width,tY+height);

  while (sourcePTR < sourceLimitPTR)
  {
     while (sourcePTR < sourceLineLimitPTR)
     {
        *targetPTR =  *sourcePTR;
        ++targetPTR; ++sourcePTR;
     }

    sourceLineLimitPTR+= sourceWidth;
    targetPTR+=targetLineSkip;
    sourcePTR+=sourceLineSkip;
  }
 return 1;
}






int saveTileRGBToFile(  unsigned int solutionColumn , unsigned int solutionRow ,
                        unsigned char * source , unsigned int sX, unsigned int sY  , unsigned int sourceWidth , unsigned int sourceHeight ,
                        unsigned int width , unsigned int height)
{

 char filename[512];
 sprintf(filename,"tiles/rgb_tile%u_%u.pnm",solutionColumn,solutionRow);


 char * tile = (char*) malloc((width+1)*(height+1)*3*sizeof(unsigned char));
 if (tile == 0 ) { return 0; }
 bitbltDepth(tile,0,0,width,height,source,sX,sY,sourceWidth,sourceHeight,width, height);


 saveRawImageToFile(filename ,tile , width , height, 3 , 8);
 free(tile);

 return 1;
}

int saveTileDepthToFile(  unsigned int solutionColumn , unsigned int solutionRow ,
                          unsigned char * source , unsigned int sX, unsigned int sY  , unsigned int sourceWidth , unsigned int sourceHeight ,
                          unsigned int width , unsigned int height)
{

 char filename[512];
 sprintf(filename,"tiles/depth_tile%u_%u.pnm",solutionColumn,solutionRow);


 char * tile = (char*) malloc((width+1)*(height+1)*1*sizeof(unsigned short));
 if (tile == 0 ) { return 0; }
 bitbltDepth(tile,0,0,width,height,source,sX,sY,sourceWidth,sourceHeight,width, height);


 saveRawImageToFile(filename ,tile , width , height, 1 , 16);
 free(tile);

 return 1;
}








int bitBltRGBToFile(  char * name  ,
                      unsigned char * source , unsigned int sX, unsigned int sY  , unsigned int sourceWidth , unsigned int sourceHeight ,
                      unsigned int width , unsigned int height)
{

 char filename[512];
 sprintf(filename,"tiles/rgb_%s.pnm",name);


 char * tile = (char*) malloc((width+1)*(height+1)*3*sizeof(unsigned char));
 if (tile == 0 ) { return 0; }
 bitbltRGB(tile,0,0,width,height,source,sX,sY,sourceWidth,sourceHeight,width, height);


 saveRawImageToFile(filename ,tile , width , height, 3 , 8);
 free(tile);

 return 1;
}




int bitBltDepthToFile(  char * name  ,
                        unsigned short * source , unsigned int sX, unsigned int sY  , unsigned int sourceWidth , unsigned int sourceHeight ,
                        unsigned int width , unsigned int height)
{

 char filename[512];
 sprintf(filename,"tiles/depth_%s.pnm",name);


 char * tile = (char*) malloc((width+1)*(height+1)*1*sizeof(unsigned short));
 if (tile == 0 ) { return 0; }
 bitbltDepth(tile,0,0,width,height,source,sX,sY,sourceWidth,sourceHeight,width, height);


 saveRawImageToFile(filename ,tile , width , height, 1  , 16);
 free(tile);

 return 1;
}


