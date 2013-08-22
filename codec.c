/*
 * CODEC.C
 * This will handle codec opening and closing
 */

#include "codec.h"
#include <libavcodec/avcodec.h>

AVCodec* SetupCodec(enum CodecID codecid){
  AVCodec* codec = NULL;
  codec = avcodec_find_encoder(codecid); //CODEC_ID_H264

  if (!codec) {
    printf("Codec not found\n");
    return NULL;
  }
  return codec;
}

AVCodecContext* SetupCodecContext(int width, int height, int fps, int bitrate, enum CodecID codecid){
  AVCodecContext* codec_context;

  codec_context = avcodec_alloc_context();
  if (!codec_context){
    printf("avcodec_alloc_context() failed\n");
    return NULL;
  }

  // set up context stuff
  codec_context->bit_rate = bitrate;
  codec_context->bit_rate_tolerance = 0;
  codec_context->rc_min_rate = 0;
  codec_context->rc_buffer_size = 10000000;
  codec_context->rc_max_rate = bitrate;
  codec_context->time_base = (AVRational){1,fps};
  codec_context->width = width;
  codec_context->height = height;


  if(codecid == CODEC_ID_H264){
    // set codeccontext presets 
    codec_context->profile = FF_PROFILE_H264_HIGH;
    codec_context->level = 51;
    codec_context->me_range = 16; 
    codec_context->max_qdiff = 4; 
    codec_context->qmin = 10; 
    codec_context->qmax = 51; 
    codec_context->qcompress = 0.6; 
  }
  return codec_context;
}


/*
 * OpenEncoder(type, bitrate)
 * Opens an encoder and does some of the heavy lifting
 */

AVCodecContext* OpenEncoder(enum CodecID codecid, int bitrate){
  AVCodecContext* codec_context;
  return NULL;
}
