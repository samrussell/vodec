/*
 * IO.C
 * This will interface with files and streams, and pass frames to/from other modules
 */

#include "io.h"
#include <libavformat/avformat.h>


/*
 * OpenInput(filename, format)
 * Opens a media file for input
 * Should be updated to give a better interface to streams
 */

AVFormatContext* OpenInput(const char* filename, AVInputFormat* format){
  AVFormatContext* format_context = NULL;

  if(avformat_open_input(&format_context, filename, format, NULL)!=0)
    return NULL;
  return format_context;
}

/*
 * OpenOutput(filename, format)
 * Opens a media file for output
 */

AVFormatContext* OpenOutput(const char* filename, AVOutputFormat* output_format){
  AVFormatContext* format_context;

  format_context = avformat_alloc_context();
  if (!format_context){
    printf("avformat_alloc_context() failed\n");
    return NULL;
  }
  if(!output_format)
    output_format = av_guess_format(NULL, filename, NULL);
  format_context->oformat = output_format;

  if (!(output_format->flags & AVFMT_NOFILE)) {
    if (avio_open(&format_context->pb, filename, AVIO_FLAG_WRITE) < 0) {
      printf("avio_open() failed\n");
      return NULL;
    }
  }
  else{
    printf("AVFMT_NOFILE not in flags\n");
    return NULL;
  }

  /*if (avformat_write_header(format_context, NULL) < 0) {
    printf("avformat_write_header() failed\n");
    return NULL;
  }*/
  return format_context;
}


