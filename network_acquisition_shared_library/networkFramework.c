#include "networkFramework.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/uio.h>



#define NORMAL   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */




#define TRANSPORT_STRUCTURE_VERSION 0

struct transportImage
{
    unsigned char headerI;
    unsigned char headerT;
    unsigned char version;

    unsigned char streamID;
    unsigned int width;
    unsigned int height;
    unsigned char channels;
    unsigned char bitsperpixel;
    unsigned int compressedSize;
};

struct transportBorder
{
    unsigned char headerN;
    unsigned char headerE;
    unsigned char headerX;
    unsigned char headerT;
};


unsigned int simplePowNet(unsigned int base,unsigned int exp)
{
    if (exp==0) return 1;
    unsigned int retres=base;
    unsigned int i=0;
    for (i=0; i<exp-1; i++)
    {
        retres*=base;
    }
    return retres;
}



int transmitPart(int sock,char * message,unsigned int message_size)
{
  int opres=send(sock,message,message_size,MSG_WAITALL|MSG_NOSIGNAL);
  if (opres<=0) { fprintf(stderr,"Failed sending `%u bytes` , error %s ..!\n",message_size,strerror(errno)); return 0; }
              else
  if ((unsigned int) opres!=message_size)
                {
                  fprintf(stderr,"Failed sending the whole message wanted to send %u bytes\n",message_size);
                  fprintf(stderr,"Sent %0.2f %% , remaining `%u bytes`..!\n", (float) ( (100*(message_size-opres))/message_size ) , message_size-opres);
                  return 0;
                }
  return 1;
}

int receivePart(int sock,char * message,unsigned int message_size)
{
  int opres=recv(sock,message,message_size,0);
  if (opres<=0) { fprintf(stderr,"Failed receiving `%u bytes`..!\n",message_size); return 0; } else
  if ((unsigned int) opres!=message_size) { fprintf(stderr,"Failed receiving the whole message `%u bytes`..!\n",message_size); return 0; }
  return 1;
}

int sendImageSocket(int sock ,unsigned  char * pixels , unsigned int width , unsigned int height , unsigned int channels , unsigned int bitsperpixel , unsigned int compressedSize )
{
  //fprintf(stderr,"sendImageSocket %ux%u %u channels %u bitsperpixel , %u bytes per pixel\n",width,height,channels,bitsperpixel, (bitsperpixel/8));

  struct transportBorder trBorder={0}; trBorder.headerN = 'N'; trBorder.headerE = 'E'; trBorder.headerX = 'X'; trBorder.headerT = 'T';

  struct transportImage trImage={0};
  trImage.headerI='I';
  trImage.headerT='T';
  trImage.version=TRANSPORT_STRUCTURE_VERSION;

  trImage.width=width;
  trImage.height=height;
  trImage.channels=channels;
  trImage.bitsperpixel=bitsperpixel;
  trImage.compressedSize = compressedSize;

  unsigned int messageSize = width * height * channels * (bitsperpixel/8);
  if (compressedSize!=0) { messageSize=compressedSize; }

  transmitPart(sock,(char*) &trImage,sizeof(struct transportImage));

  transmitPart(sock,(char*) pixels,messageSize);

  transmitPart(sock,(char*) &trBorder,sizeof(struct transportBorder));

  return 1;
}





unsigned char * recvImageSocket(int sock , unsigned int * width , unsigned int * height , unsigned int channels , unsigned int bitsperpixel )
{
  unsigned char *imgPtr=0;
  struct transportBorder trBorder={0};
  struct transportImage trImage={0};


  receivePart(sock,(char*) &trImage,sizeof(struct transportImage));

  unsigned int messageSize = trImage.width * trImage.height * trImage.channels * (simplePowNet(2,trImage.bitsperpixel)/8);

  imgPtr = (unsigned  char* ) malloc(sizeof(unsigned char) * messageSize);
  if (imgPtr==0)
  {
      fprintf(stderr,RED "Could not allocate space for recvImageSocket call ( %u bytes ) " NORMAL , messageSize );
      return 0;
  }
  receivePart(sock,(char*) imgPtr,messageSize);

  receivePart(sock,(char*) &trBorder,sizeof(struct transportBorder));
   if ( (trBorder.headerN!='N') || (trBorder.headerE!='E') || (trBorder.headerX!='X') || (trBorder.headerT!='T') )
      {
        fprintf(stderr,RED "Failed reading boundary after Image @ recvImageSocket call" NORMAL);
        free(imgPtr);
        return 0;
      }


  return imgPtr;
}
