/*
	Project:FFmpeg Media Source for Windows Media Foundation
	Part:FFmpeg Wrapper
	File:ffmpeg_common_utils.h

	Creator:ShanYe (2014-06-13)
	Last Change:ShanYe (2014-06-13)
	Rev:1.0.0
*/

#pragma once

#include "stdafx.h"
#include "ffmpeg.h"

#define VID_WH_E(w,h,w2,h2) (w <= w2) && (h <= h2)

void make_ffmpeg_codec_audio_info_to_WAVEFORMATEX(AVCodecContext* codec,WAVEFORMATEX* pwfx);
void make_ffmpeg_codec_audio_16bPCM_info_to_WAVEFORMATEX(AVCodecContext* codec,WAVEFORMATEX* pwfx);
int get_ffmpeg_mpeg4_h263_profile_level(AVCodecContext* codec);
bool query_ffmpeg_metadata_desc_string(AVDictionary* metadata,char* name,char* pstr,int cch);
long long convert_ffmpeg_pts(AVPacket* packet,AVRational time_base,long long start_time,long long new_tb);