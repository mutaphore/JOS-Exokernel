#include "ns.h"

extern union Nsipc nsipcbuf;

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";

	// LAB 6: Your code here:
	// 	- read a packet from the network server
	//	- send the packet to the device driver
   
   int perm, val;
   envid_t output_envid = -1;
   
   while (1) {   
      val = ipc_recv(&output_envid, &nsipcbuf, &perm);
      while (sys_net_send_pckt(nsipcbuf.pkt.jp_data, nsipcbuf.pkt.jp_len) != 0)
         sys_yield();   // Keep yielding until packet is sent
   }
}
