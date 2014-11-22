#include "ns.h"

extern union Nsipc nsipcbuf;

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";

	// LAB 6: Your code here:
	// 	- read a packet from the network server
	//	- send the packet to the device driver
   
   int perm, val, tail = 0;
   envid_t output_envid = -1;
   void *tbuf;
   
   while (1) {   
      if ((val = ipc_recv(&output_envid, &nsipcbuf, &perm)) == NSREQ_OUTPUT) {
         tbuf = (void *)(UTBUFMAP + tail * PGSIZE);

         // Currently we need to copy because we receive an IPC from server.
         // Alternative implementation to take advantage of zero-copy is
         // to copy data directly into mapped memory buffer at UTBUFMAP
         // in source process.
         memcpy(tbuf, &nsipcbuf.pkt.jp_data, nsipcbuf.pkt.jp_len);

         while (sys_net_send_pckt(NULL, nsipcbuf.pkt.jp_len) != 0)
            sys_yield();   // Keep yielding until packet is sent
         tail = (tail + 1) % NUMTDS;
      }
   }
}
