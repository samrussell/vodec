/*
 * CODEC.C
 * This will handle codec opening and closing
 */

#include "codec.h"
#include <libavcodec/avcodec.h>

AVCodec* SetupCodec(enum AVCodecID codecid){
  codec = avcodec_find_encoder(codecid); //CODEC_ID_H264

  if (!codec) {
    printf("Codec not found\n");
    return NULL;
  }
  return codec;
}

AVCodecContext* SetupCodecContext(int bitrate, enum AVCodecID codecid){
  AVCodecContext* codec_context;

  codec_context = avcodec_alloc_context();
  if (!codec_context){
    printf("avcodec_alloc_context() failed\n");
    return NULL;
  }

  // set up context stuff
  codec_context->bit_rate = 0;
  codec_context->bit_rate_tolerance = 0;
  codec_context->rc_min_rate = 0;
  codec_context->rc_buffer_size = 10000000;
  codec_context->rc_max_rate = bitrate;

  if(codecid == CODEC_ID_H264){
    // set codeccontext presets 
  }
  return codec_context;
}


/*
 * OpenEncoder(type, bitrate)
 * Opens an encoder and does some of the heavy lifting
 */

AVCodecContext* OpenEncoder(enum AVCodecID codecid, int bitrate){
  AVCodecContext* codec_context;
  return NULL;
}
