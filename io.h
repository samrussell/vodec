/*
 * IO.H
 * This will interface with files and streams, and pass frames to/from other modules
 */

#include <libavformat/avformat.h>

AVFormatContext* OpenInput(const char* filename, AVInputFormat* format);
AVFormatContext* OpenOutput(const char* filename, AVOutputFormat* format);


