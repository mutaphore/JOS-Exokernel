#include "ns.h"

void
input(envid_t ns_envid)
{
	binaryname = "ns_input";

	// LAB 6: Your code here:
	// 	- read a packet from the device driver
	//	- send it to the network server
	// Hint: When you IPC a page to the network server, it will be
	// reading from it for a while, so don't immediately receive
	// another packet in to the same physical page.

   int r, tail = 0;
   static union Nsipc *nsipcbuf;

   while (1) {
      // Spin until a packet is received
      while ((r = sys_net_recv_pckt(NULL)) == -E_PCKT_NONE)
         sys_yield();
   
      if (r < 0)
         panic("input: %e", r);

      nsipcbuf = (union Nsipc *)(URBUFMAP + tail * PGSIZE);
      nsipcbuf->pkt.jp_len = r;
      ipc_send(ns_envid, NSREQ_INPUT, nsipcbuf, PTE_U | PTE_W | PTE_P);
      tail = (tail + 1) % NUMRDS;
   }
}

