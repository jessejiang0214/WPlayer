#pragma once

#include "stdafx.h"
#include "ffmpeg.h"

bool ffmpeg_h264_sequence_header_parser(int* profile,int* level,int* flags,byte* psrc,int nsrc_size,byte* pdst,int* ndst_size,bool* raw);