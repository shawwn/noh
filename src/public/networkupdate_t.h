// (C)2005 S2 Games
// networkupdate_t.h
//
//=============================================================================
#ifndef __NETWORKUPDATE_T_H__
#define __NETWORKUPDATE_T_H__

//this is the structure used for sending any changes in object positions, angles, etc, over the network
typedef struct networkUpdate_s
{
	uint				framenum;
	int					time;			//server timestamp of this update

	uint				start_object;	//non - modded index into client->net_objects circular buffer
	int					num_objects;	//number of objects in this update

	SPlayerState		playerstate;	//playerstate structure (no need for a circular buffer, as there's only 1 per frame)
}
SNetworkUpdate;

#endif // __NETWORKUPDATE_T_H__
