// tutorial01.c
//
// This tutorial was written by Stephen Dranger (dranger@gmail.com).
//
// Code based on a tutorial by Martin Bohme (boehme@inb.uni-luebeckREMOVETHIS.de)
// Tested on Gentoo, CVS version 5/01/07 compiled with GCC 4.1.1

// A small sample program that shows how to use libavformat and libavcodec to
// read video from a file.
//
// Use the Makefile to build all examples.
//
// Run using
//
// tutorial01 myvideofile.mpg
//
// to write the first five frames from "myvideofile.mpg" to disk in PPM
// format.

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
  int             numBytes;
  uint8_t         *buffer = NULL;

  AVDictionary    *optionsDict = NULL;
  struct SwsContext      *sws_ctx = NULL;

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
  struct SwsContext *sws_ctxOut = NULL;

  if(argc < 3) {
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

  // Allocate an AVFrame structure
  pFrameRGB=avcodec_alloc_frame();
  if(pFrameRGB==NULL)
    return -1;

  // Determine required buffer size and allocate buffer
  numBytes=avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width,
			      pCodecCtx->height);
  buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

  sws_ctx =
    sws_getContext
    (
        pCodecCtx->width,
        pCodecCtx->height,
        pCodecCtx->pix_fmt,
        pCodecCtx->width,
        pCodecCtx->height,
        PIX_FMT_RGB24,
        SWS_BILINEAR,
        NULL,
        NULL,
        NULL
    );

  // Assign appropriate parts of buffer to image planes in pFrameRGB
  // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
  // of AVPicture
  avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24,
		 pCodecCtx->width, pCodecCtx->height);

  // Set up output encoder too

  // find the encoder AV_CODEC_ID_H264
  pCodecOut = avcodec_find_encoder(CODEC_ID_VP8); //CODEC_ID_H264);

  if (!pCodecOut) {
    fprintf(stderr, "Codec not found\n");
    exit(1);
  }

  pCodecCtxOut = avcodec_alloc_context();
  if (!pCodecCtxOut) {
    fprintf(stderr, "Could not allocate video codec context\n");
    exit(1);
  }


  pCodecCtxOut->bit_rate = 4000000;

  pCodecCtxOut->width = pCodecCtx->width;
  pCodecCtxOut->height = pCodecCtx->height;
  pCodecCtxOut->time_base = (AVRational){1,10};
  pCodecCtxOut->gop_size = pCodecCtx->gop_size;
  pCodecCtxOut->max_b_frames = pCodecCtx->max_b_frames;
  pCodecCtxOut->pix_fmt = pCodecCtx->pix_fmt;


  if (avcodec_open(pCodecCtxOut, pCodecOut) < 0) {
    fprintf(stderr, "Could not open codec\n");
    exit(1);
  }


  outfmtcontext = OpenOutput(argv[2], NULL);
  if (!outfmtcontext){
    fprintf(stderr, "Couldn't create output format context\n");
    exit(1);
  }
  outfmt = outfmtcontext->oformat;


  outvideostream = avformat_new_stream(outfmtcontext, NULL);
  avcodec_copy_context( outvideostream->codec, pCodecCtxOut );

  if (avformat_write_header(outfmtcontext, NULL) < 0) {
    fprintf(stderr, "Error occurred when opening output file\n");
    exit(1);
  }


outbuf_size=100000;
outbuf=calloc(outbuf_size, 1);
printf("Buffer address: 0x%X\n", outbuf);

printf("Ready to start the process\n");

  // Read frames and save first five frames to disk
  i=0;
  while(av_read_frame(pFormatCtx, &packet)>=0) {
    // Is this a packet from the video stream?
    if(packet.stream_index==videoStream) {
      // Decode video frame
      avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished,
			   &packet);

      // Did we get a video frame?
      if(frameFinished) {
	      // encode
        av_init_packet(&packetOut);
        packetOut.data = NULL;    // packet data will be allocated by the encoder
        packetOut.size = 0;


        // encode the image
        //ret = avcodec_encode_video2(pCodecCtxOut, &packetOut, pFrame, &got_output);
        //if (ret < 0) {
        //  fprintf(stderr, "Error encoding frame\n");
        //  exit(1);
        //}

        //printf("About to start encoding\n");

        /*if(outvideostream == NULL){//create stream in file
          outvideostream = avformat_new_stream(outfmtcontext,pCodec);
          avcodec_copy_context(outvideostream->codec,pCodec);
          outvideostream->sample_aspect_ratio = pCodec->sample_aspect_ratio;
          avformat_write_header(oc,NULL);
        }
        packet.stream_index = outvideostream->id*/

        // use avcodec_encode_video() (not 2)
        out_size = avcodec_encode_video(pCodecCtxOut, outbuf, outbuf_size, pFrame);
        //printf("Called avcodec_encode_video()\n");
        printf("encoding frame %3d (size=%5d)\n", i, out_size);
        packetOut.data=outbuf;
        packetOut.size = out_size;
        //fwrite(outbuf, 1, out_size, f);
        av_write_frame(outfmtcontext, &packetOut);
        av_free_packet(&packetOut);

        //if (got_output) {
        //  printf("Write frame %3d (size=%5d)\n", i, packetOut.size);
        //  fwrite(packetOut.data, 1, packetOut.size, f);
        //  av_free_packet(&packetOut);
        //}
	    // Save the frame to disk
	    i++;
	    //if(++i<=50)
	    //  SaveFrame(pFrameRGB, pCodecCtx->width, pCodecCtx->height, i);
      }
    }

    // Free the packet that was allocated by av_read_frame
    av_free_packet(&packet);
  }

  // finish encode
  for (out_size=1; out_size; i++) {
    fflush(stdout);
    av_init_packet(&packetOut);
    packetOut.data = NULL;    // packet data will be allocated by the encoder
    packetOut.size = 0;
    out_size = avcodec_encode_video(pCodecCtxOut, outbuf, outbuf_size, NULL);
    printf("encoding frame %3d (size=%5d)\n", i, out_size);
    packetOut.data=outbuf;
    packetOut.size = out_size;
    if(out_size){
      av_write_frame(outfmtcontext, &packetOut);
    }
    av_free_packet(&packetOut);

  }

  av_write_trailer( outfmtcontext );
  avio_close( outfmtcontext->pb );
  avformat_free_context( outfmtcontext );
  free(outbuf);
  avcodec_close(pCodecCtxOut);

  // Free the RGB image
  av_free(buffer);
  av_free(pFrameRGB);

  // Free the YUV frame
  av_free(pFrame);

  // Close the codec
  avcodec_close(pCodecCtx);

  // Close the video file
  avformat_close_input(&pFormatCtx);

  return 0;
}
