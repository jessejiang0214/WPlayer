#ifndef FFMPEG_H_
#define FFMPEG_H_

#pragma once

extern "C" {
#include <libavcodec\avcodec.h>
#include <libavcodec\avfft.h>
#include <libavcodec\vaapi.h>
#include <libavcodec\old_codec_ids.h>
#include <libavformat\avformat.h>
//#include <libavfilter\avfilter.h>
#include <libavutil\avutil.h>
#include <libavutil\common.h>
#include <libavutil\bswap.h>
#include <libavutil\opt.h>
#include <libavutil\md5.h>
#include <libavutil\time.h>
#include <libavutil\audioconvert.h>
#include <libavutil\attributes.h>
#include <libavutil\imgutils.h>
#include <libavutil\intfloat.h>
#include <libavutil\intfloat_readwrite.h>
#include <libavutil\intreadwrite.h>
#include <libavutil\timecode.h>
#include <libavutil\mem.h>
#include <libavutil\pixdesc.h>
#include <libavutil\fifo.h>
#include <libswscale\swscale.h>
#include <libswresample\swresample.h>
}

#define _INIT_FFMPEG_ av_register_all();avcodec_register_all();

#define DS_REF_TIME_BASE 10000000
#define MF_REF_TIME_BASE DS_REF_TIME_BASE

#endif