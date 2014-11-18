#include "ns.h"

extern union Nsipc nsipcbuf;

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

   int r;
   static union Nsipc queue[5];
   static int head = 0, tail = 0;

   // Spin until a packet is received
   while ((r = sys_net_recv_pckt(queue[tail].pkt.jp_data)) == -E_PCKT_NONE)
      sys_yield();
   
   cprintf("Here\n");

   if (r < 0)
      panic("input: %e", r);


   queue[tail].pkt.jp_len = r;

   //memcpy(&queue[tail], &nsipcbuf, sizeof(union Nsipc));
   tail = (tail + 1) % 5; 

   ipc_send(ns_envid, NSREQ_INPUT, &queue[head] , PTE_U | PTE_W); 
   head = (head + 1) % 5;
}
