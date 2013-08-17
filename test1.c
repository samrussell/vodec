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

void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame) {
  FILE *pFile;
  char szFilename[32];
  int  y;

  // Open file
  sprintf(szFilename, "frame%d.ppm", iFrame);
  pFile=fopen(szFilename, "wb");
  if(pFile==NULL)
    return;

  // Write header
  fprintf(pFile, "P6\n%d %d\n255\n", width, height);

  // Write pixel data
  for(y=0; y<height; y++)
    fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile);

  // Close file
  fclose(pFile);
}

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
  if(avformat_open_input(&pFormatCtx, argv[1], NULL, NULL)!=0)
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

  // put sample parameters
  /*
  c->bit_rate = 400000;
  // resolution must be a multiple of two
  c->width = 352;
  c->height = 288;
  // frames per second
  c->time_base= (AVRational){1,25};
  c->gop_size = 10; // emit one intra frame every ten frames
  c->max_b_frames=1;
  c->pix_fmt = AV_PIX_FMT_YUV420P;
  */

  // steal parameters from the other context
  //pCodecCtxOut->bit_rate = pCodecCtx->bit_rate;
  pCodecCtxOut->bit_rate = 4000000;

  pCodecCtxOut->width = pCodecCtx->width;
  pCodecCtxOut->height = pCodecCtx->height;
  //pCodecCtxOut->time_base = pCodecCtx->time_base;
  pCodecCtxOut->time_base = (AVRational){1,10};
  pCodecCtxOut->gop_size = pCodecCtx->gop_size;
  pCodecCtxOut->max_b_frames = pCodecCtx->max_b_frames;
  pCodecCtxOut->pix_fmt = pCodecCtx->pix_fmt;

  // stealing a profile to make x264 work
  /*
  pCodecCtxOut->bit_rate_tolerance = 0;
  pCodecCtxOut->rc_max_rate = 0;
  pCodecCtxOut->rc_buffer_size = 0;
  pCodecCtxOut->b_frame_strategy = 1;
  pCodecCtxOut->coder_type = 1;
  pCodecCtxOut->me_cmp = 1;
  pCodecCtxOut->me_range = 16;
  pCodecCtxOut->qmin = 10;
  pCodecCtxOut->qmax = 51;
  pCodecCtxOut->scenechange_threshold = 40;
  pCodecCtxOut->flags |= CODEC_FLAG_LOOP_FILTER;
  pCodecCtxOut->me_method = ME_HEX;
  pCodecCtxOut->me_subpel_quality = 5;
  pCodecCtxOut->i_quant_factor = 0.71;
  pCodecCtxOut->qcompress = 0.6;
  pCodecCtxOut->max_qdiff = 4;
  pCodecCtxOut->directpred = 1;
  pCodecCtxOut->flags2 |= CODEC_FLAG2_FASTPSKIP;
  */

  if (avcodec_open(pCodecCtxOut, pCodecOut) < 0) {
    fprintf(stderr, "Could not open codec\n");
    exit(1);
  }



  //f = fopen(argv[2], "wb");
  //if (!f) {
  //  fprintf(stderr, "Could not open %s\n", argv[2]);
  //  exit(1);
  //}

  // use libavformat for output instead
  outfmtcontext = avformat_alloc_context();
  if (!outfmtcontext){
    fprintf(stderr, "Couldn't create output format context\n");
    exit(1);
  }

  outfmt = av_guess_format(NULL,argv[2],NULL);

  outfmtcontext->oformat = outfmt;

  outvideostream = avformat_new_stream(outfmtcontext, NULL);
  avcodec_copy_context( outvideostream->codec, pCodecCtxOut );

  av_dump_format(outfmtcontext, 0, argv[2], 1);

  if (!(outfmt->flags & AVFMT_NOFILE)) {
    if (avio_open(&outfmtcontext->pb, argv[2], AVIO_FLAG_WRITE) < 0) {
      fprintf(stderr, "Could not open '%s'\n", argv[2]);
      exit(1);
    }
  }

  if (avformat_write_header(outfmtcontext, NULL) < 0) {
    fprintf(stderr, "Error occurred when opening output file\n");
    exit(1);
  }

  //pFrameOut = avcodec_alloc_frame();
  //if (!pFrameOut) {
  //  fprintf(stderr, "Could not allocate video frame\n");
  //  exit(1);
  //}
  //pFrameOut->format = pCodecCtxOut->pix_fmt;
  //pFrameOut->width  = pCodecCtxOut->width;
  //pFrameOut->height = pCodecCtxOut->height;

  /* the image can be allocated by any means and av_image_alloc() is
   * just the most convenient way if av_malloc() is to be used */
  //ret = av_image_alloc(frame->data, frame->linesize, c->width, c->height,
  //                     c->pix_fmt, 32);
  //if (ret < 0) {
  //  fprintf(stderr, "Could not allocate raw picture buffer\n");
  //  exit(1);
  //}
  /* encode 1 second of video */
  //for(i=0;i<25;i++) {
  /*
    av_init_packet(&packetOut);
    packetOut.data = NULL;    // packet data will be allocated by the encoder
    packetOut.size = 0;

    // encode the image
    ret = avcodec_encode_video2(c, &packetOut, frame, &got_output);
    if (ret < 0) {
      fprintf(stderr, "Error encoding frame\n");
      exit(1);
    }

    if (got_output) {
      printf("Write frame %3d (size=%5d)\n", i, packetOut.size);
      fwrite(packetOut.data, 1, packetOut.size, f);
      av_free_packet(&packetOut);
    }
  */
  //}

  /* get the delayed frames */
  /*
  for (got_output = 1; got_output; i++) {
    fflush(stdout);

    ret = avcodec_encode_video2(c, &packetOut, NULL, &got_output);
    if (ret < 0) {
      fprintf(stderr, "Error encoding frame\n");
      exit(1);
    }

    if (got_output) {
      printf("Write frame %3d (size=%5d)\n", i, packetOut.size);
      fwrite(packetOut.data, 1, packetOut.size, f);
      av_free_packet(&packetOut);
    }
  }
  */

  /*
  fwrite(endcode, 1, sizeof(endcode), f);
  fclose(f);

  avcodec_close(pCodecCtxOut);
  //av_free(pCodecCtxOut);
  //av_freep(&frame->data[0]);
  //avcodec_free_frame(&frame);
  printf("\n");
  */

  // Do the transcode part

  // Prepare buffer for encoded data



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
	  // Convert the image from its native format to RGB
        sws_scale
        (
            sws_ctx,
            (uint8_t const * const *)pFrame->data,
            pFrame->linesize,
            0,
            pCodecCtx->height,
            pFrameRGB->data,
            pFrameRGB->linesize
        );


        // also encode
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

    //ret = avcodec_encode_video2(pCodecCtxOut, &packetOut, NULL, &got_output);
    //if (ret < 0) {
    //  fprintf(stderr, "Error encoding frame\n");
    //  exit(1);
    //}

    //if (got_output) {
    //  printf("Write frame %3d (size=%5d)\n", i, packetOut.size);
    //  fwrite(packetOut.data, 1, packetOut.size, f);
    //  av_free_packet(&packetOut);
    //}
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
    //fwrite(outbuf, 1, out_size, f);

  }

  /* add sequence end code to have a real mpeg file */
  //fwrite(endcode, 1, sizeof(endcode), f);
  //fclose(f);
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
