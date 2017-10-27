#ifndef _CLIENTINFO_H
#define _CLIENTINFO_H

typedef struct{
	char myfifo[100];
	int leftarg;
	int rightarg;
	char op;
} CLIENTINFO,* CLIENTINFOPTR;
#endif
