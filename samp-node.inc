/*
* samp-node plugin (c) Amyr Ahmady (iAmir) 2020
* include file
*/

#if defined _inc_samp_node
	#undef _inc_samp_node
#endif

#if defined _node_included
	#endinput
#endif
#define _node_included

native SAMPNode_CallEvent(const eventName[], {Float,_}:...);
