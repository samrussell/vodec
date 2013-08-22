/*
 * CODEC.H
 * This will handle codec opening and closing
 */

#include <libavcodec/avcodec.h>

AVCodec* SetupCodec(enum CodecID codecid);
AVCodecContext* OpenEncoder(enum CodecID codecid, int bitrate);
AVCodecContext* SetupCodecContext(int width, int height, int fps, int bitrate, enum CodecID codecid);
