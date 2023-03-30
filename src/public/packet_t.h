#ifndef __PACKET_T__
#define __PACKET_T__

#define	MAX_PACKET_SIZE (8192 - HEADER_SIZE)
#define HEADER_SIZE		5 //(sizeof(uint) + sizeof(byte))

typedef struct packet_s
{
    unsigned char   __buf[MAX_PACKET_SIZE + HEADER_SIZE];
	//  byte    *buf;
	int     curpos;
	int     length;

	bool	overflowed;
}
packet_t;

#endif // __PACKET_T__
