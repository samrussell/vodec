/*
 * CODEC.H
 * This will handle codec opening and closing
 */

#include <libavcodec/avcodec.h>

AVCodec* SetupCodec(enum AVCodecID codecid);
AVCodecContext* OpenEncoder(enum AVCodecID codecid, int bitrate);
AVCodecContext* SetupCodecContext(int bitrate, enum AVCodecID codecid);
