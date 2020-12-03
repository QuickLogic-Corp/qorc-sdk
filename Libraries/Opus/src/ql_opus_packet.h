
#ifndef QL_OPUS_PACKET_H_
#define QL_OPUS_PACKET_H_
#define QL_OPUS_PADDING_LENGTH (0)

typedef struct
{
	unsigned short config : 5;
	unsigned short stereo : 1;
	unsigned short count : 2;

	unsigned short v : 1;		/* 0 - cbr */
	unsigned short p : 1;
	unsigned short nFrames : 6; /* m - depends on the user argument */
#if (QL_OPUS_PADDING_LENGTH > 0)
	unsigned short nPaddingLen;
#endif 
} ql_opus_packet_hdr;

#define QL_OPUS_PACKET_HDR_SIZE (sizeof(ql_opus_packet_hdr))
#define QL_OPUS_MAX_FRAMES_IN_PACKET (12)   /* max is 120mSec, for 10mSec frame, the count max is 12 */
#define QL_OPUS_MAX_FRAMESIZE (48)
#define QL_PAYLOAD_MAX_SIZE (QL_OPUS_PACKET_HDR_SIZE + QL_OPUS_MAX_FRAMES_IN_PACKET*QL_OPUS_MAX_FRAMESIZE)
typedef struct
{
	struct {
		ql_opus_packet_hdr hdr;
		unsigned char payload[QL_PAYLOAD_MAX_SIZE];
	}packet;
	short frame_size;
	int status;
	int packet_size;
	int n_frames_per_packet;
	unsigned char *p_frame_beg[QL_OPUS_MAX_FRAMES_IN_PACKET];
}ql_opus_packet_info;

ql_opus_packet_info *ql_opus_packetize_init(unsigned char *pMem, int nFrames, int framesize);

#endif /* QL_OPUS_PACKET_H_ */

