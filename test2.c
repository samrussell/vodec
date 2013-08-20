// test2.c
//

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include <stdio.h>

#include "io.h"

int main(int argc, char *argv[]) {
  AVFormatContext *pFormatCtx = NULL;
  int             i, videoStream;
  AVCodecContext  *pCodecCtx = NULL;
  AVCodec         *pCodec = NULL;
  AVFrame         *pFrame = NULL;
  AVFrame         *pFrameRGB = NULL;
  AVPacket        packet;
  int             frameFinished;

  AVDictionary    *optionsDict = NULL;

  AVFormatContext *pFormatCtxOut = NULL;
  int             videoStreamOut;
  AVCodecContext  *pCodecCtxOut = NULL;
  AVCodec         *pCodecOut = NULL;
  AVFrame         *pFrameOut = NULL;
  AVPacket        packetOut;
  int             frameFinishedOut;
  FILE*           f=NULL;
  int             ret;
  int             got_output;
  int             out_size;
  int             outbuf_size;
  uint8_t         *outbuf;
  AVOutputFormat  *outfmt;
  AVFormatContext *outfmtcontext;
  AVStream        *outvideostream=NULL;
  AVCodec         *outvideocodec;

  uint8_t endcode[] = { 0, 0, 1, 0xb7 };

  AVDictionary    *optionsDictOut = NULL;

  if(argc < 2) {
    printf("Please provide a movie file\n");
    return -1;
  }
  // Register all formats and codecs
  av_register_all();

  // Open video file
  //if(avformat_open_input(&pFormatCtx, argv[1], NULL, NULL)!=0)
  pFormatCtx = OpenInput(argv[1], NULL);
  if(!pFormatCtx)
    return -1; // Couldn't open file

  // Retrieve stream information
  if(avformat_find_stream_info(pFormatCtx, NULL)<0)
    return -1; // Couldn't find stream information

  // Dump information about file onto standard error
  av_dump_format(pFormatCtx, 0, argv[1], 0);

  // Find the first video stream
  videoStream=-1;
  for(i=0; i<pFormatCtx->nb_streams; i++)
    if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
      videoStream=i;
      break;
    }
  if(videoStream==-1)
    return -1; // Didn't find a video stream

  // Get a pointer to the codec context for the video stream
  pCodecCtx=pFormatCtx->streams[videoStream]->codec;
  
  // be awesome and get codec parameters
  
  printf("Time base: %d/%d\n", pFormatCtx->streams[videoStream]->time_base.num, pFormatCtx->streams[videoStream]->time_base.den);
  printf("Real base frame rate: %d/%d\n", pFormatCtx->streams[videoStream]->r_frame_rate.num, pFormatCtx->streams[videoStream]->r_frame_rate.den);
  printf("Bit rate: %d\n", pFormatCtx->streams[videoStream]->codec->bit_rate); 
  
  // Find the decoder for the video stream
  pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
  if(pCodec==NULL) {
    fprintf(stderr, "Unsupported codec!\n");
    return -1; // Codec not found
  }
  // Open codec
  if(avcodec_open2(pCodecCtx, pCodec, &optionsDict)<0)
    return -1; // Could not open codec

  // Allocate video frame
  pFrame=avcodec_alloc_frame();

  // Set up output encoder too


  // Free the YUV frame
  av_free(pFrame);

  // Close the codec
  avcodec_close(pCodecCtx);

  // Close the video file
  avformat_close_input(&pFormatCtx);

  return 0;
}
