
#include "ql_opus_packet.h"

#define FRAMES_PER_PACKET (3)

ql_opus_packet_info *ql_opus_packetize_init(unsigned char *pMem, int nFrames, int framesize)
{
	ql_opus_packet_info *p_packet_info = (ql_opus_packet_info*)pMem;

	if (nFrames > QL_OPUS_MAX_FRAMES_IN_PACKET)
	{
		p_packet_info->status = -1;
		return p_packet_info;
	}
	p_packet_info->packet.hdr.config = 22; /* 22 - for celt only 10mS WB */
	p_packet_info->packet.hdr.stereo = 0;  /*  0 - mono */
	p_packet_info->packet.hdr.count = 3;   /*  3 - for M frames */
	p_packet_info->packet.hdr.v = 0;		/* 0 - cbr */
	p_packet_info->packet.hdr.p = 0;		/* 0 - no padding */

	p_packet_info->packet.hdr.nFrames = nFrames;
	p_packet_info->frame_size = framesize;

	p_packet_info->packet_size = sizeof(ql_opus_packet_hdr);

	p_packet_info->p_frame_beg[0] = &p_packet_info->packet.payload[0];
	p_packet_info->packet_size += framesize;
	for (int i = 1; i < nFrames; i++)
	{
		p_packet_info->p_frame_beg[i] = p_packet_info->p_frame_beg[i-1] + framesize;
		p_packet_info->packet_size += framesize;
	}

	p_packet_info->n_frames_per_packet = nFrames;
	
	return p_packet_info;
}

