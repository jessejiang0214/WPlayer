#include "stdafx.h"
#include "ffmpeg_h264_extera_data_annexb.h"

//将FFMPEG的扩展数据转换为MF可识别的H264的SPS和PPS头。
//微软MF的H264解码器要求提交Annex-B样式的头数据。
bool ffmpeg_h264_sequence_header_parser(int* profile,int* level,int* flags,byte* psrc,int nsrc_size,byte* pdst,int* ndst_size,bool* raw)
{
	if (psrc == NULL || nsrc_size < 8) //确认输入正确
		return false;
	bool bRawAnnexB = false;
	if (*(int32_t*)psrc == 0x01000000)
		bRawAnnexB = true;
	if ((*(short*)psrc == 0) && (*(psrc + 2) == 1))
		bRawAnnexB = true;
	if (bRawAnnexB) //判断如果输入的格式原始就是AnnexB格式的，不修改。
	{
		*ndst_size = nsrc_size;
		if (profile)
			*profile = *level = *flags = 0;
		if (raw)
			*raw = true;
		memcpy(pdst,psrc,nsrc_size); //直接复制
		return true;
	}
	if (*psrc != 1) //普通NALU格式（mkv、flv、mp4等，m2ts、ts的H264不同。）
		return false;
	if (psrc[1] != 0 && profile) //H264配置文件
		*profile = psrc[1];
	if (psrc[3] != 0 && level) //配置文件等级
		*level = psrc[3];
	if (psrc[4] != 0 && flags) //flag
		*flags = (psrc[4] & 3) + 1;
	unsigned short nSPS = 0,nPPS = 0;
	nSPS = *(unsigned short*)(psrc + 6) >> 8;
	if (nSPS == 0) //确认有SPS
		return false;
	if (nsrc_size < (nSPS + 7))
		return false;
	if (pdst)
	{
		*(int*)pdst = 0x01000000; //修正为00 00 00 01
		memcpy(&pdst[4],psrc + 8,nSPS); //写入SPS
	}
	if (psrc[8 + nSPS] != 1)
		return false;
	nPPS = *(unsigned short*)(psrc + 9 + nSPS) >> 8;
	if (nPPS == 0) //确定有PPS
		return false;
	if (pdst)
	{
		*(int*)(&pdst[4 + nSPS]) = 0x01000000; //修正00 00 00 01
		memcpy(&pdst[4 + 4 + nSPS],psrc + 11 + nSPS,nPPS); //写入PPS
	}
	if (ndst_size)
		*ndst_size = 4 + 4 + nSPS + nPPS; //写入头大小
	return true;
}