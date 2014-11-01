#include <inc/stdio.h>
#include <inc/error.h>
#include <inc/string.h>

#define BUFLEN 1024
static char buf[BUFLEN];

int tab_complt(int pos);

char *
readline(const char *prompt)
{
	int i, c, echoing;

#if JOS_KERNEL
	if (prompt != NULL)
		cprintf("%s", prompt);
#else
	if (prompt != NULL)
		fprintf(1, "%s", prompt);
#endif

	i = 0;
	echoing = iscons(0);
	while (1) {
		c = getchar();
		if (c < 0) {
			if (c != -E_EOF)
				cprintf("read error: %e\n", c);
			return NULL;
		} else if ((c == '\b' || c == '\x7f') && i > 0) {
			if (echoing)
				cputchar('\b');
			i--;
		} else if (c >= ' ' && i < BUFLEN-1) {
			if (echoing)
				cputchar(c);
			buf[i++] = c;
		} else if (c == '\n' || c == '\r') {
			if (echoing)
				cputchar('\n');
			buf[i] = 0;
			return buf;
		} else if (c == '\t') {
         // Tab completion
         i += tab_complt(i);
      }
	}
}

// Lab 5 Challenge: Tab completion

#define MAX_CMDS 50

static char *commands[23] = {
   "newmotd",
   "motd",
   "lorem",
   "script",
   "testshell.key",
   "testshell.sh",
   "init",
   "cat",
   "echo",
   "init",
   "ls",
   "lsfd",
   "num",
   "forktree",
   "primes",
   "primespipe",
   "sh",
   "testfdsharing",
   "testkbd",
   "testpipe",
   "testpteshare",
   "testshell",
   "hello"
};

int tab_complt(int pos) {

   char *start = buf + pos, *cmd;
   int i, n, d = 0;

   // Walk to the space before this command
   while (start > buf && *start != ' ')
      start--; 

   n = buf + pos - start - 1;

   // Look for a match
   for (i = 0; n > 0 && i < 23; i++) {
      if (strncmp(start, commands[i], n) == 0) {
         cmd = commands[i];
         d = strlen(cmd) - n;
         break;
      }
   }
   // Output the rest of the command
   for (i = n + 1; d > 0 && i < strlen(cmd); i++) {
      buf[pos++] = cmd[i];
      cputchar(cmd[i]);
   }

   return d;
}
