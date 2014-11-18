#include "ns.h"

#define QSIZE 64
#define QSTART 0x00900000

static union Nsipc *queue[QSIZE] = {0};
static int head = 0, tail = 0;

extern union Nsipc nsipcbuf;

void init_queue() {
   int i, error;

   // Allocate pages for Nsipc queue
   for (i = 0; i < QSIZE; i++) {
      queue[i] = (union Nsipc *)(QSTART + i * 0x1000);
      if ((error = sys_page_alloc(0, queue[i], PTE_U | PTE_W | PTE_P)) < 0)
         panic("init_queue: %e", error);   
   }

   cprintf("Init queue\n");
}

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

   init_queue();

   while (1) {
      // Spin until a packet is received
      while ((r = sys_net_recv_pckt(queue[tail]->pkt.jp_data)) == -E_PCKT_NONE)
         sys_yield();
   
      if (r < 0)
         panic("input: %e", r);

      queue[tail]->pkt.jp_len = r;
      tail = (tail + 1) % QSIZE; 

      ipc_send(ns_envid, NSREQ_INPUT, queue[head], PTE_U | PTE_W | PTE_P); 
      head = (head + 1) % QSIZE;
   }
}

