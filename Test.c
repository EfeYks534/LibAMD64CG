#include "AMD64CG.h"
#include <stdio.h>
#include <stdlib.h>


int main()
{
	struct BinIArg arg = (struct BinIArg) { 0 };

	uint8_t *code = calloc(256, 1);
	uint8_t *bin = code;

	arg.r   = REG_R13;
	arg.mod = MOD_RBP;
	arg.im  = 0x696969;
	BinGenInstr(&bin, &arg, "MOV mq rq | W 89 /r");

	arg.r   = REG_R13;
	arg.rm  = REG_RCX;
	arg.mod = MOD_DIR;
	arg.im  = 0x69;
	BinGenInstr(&bin, &arg, "MOV rq mq | W 8B /r");

	while(code != bin) {
		printf("%02x ", *code);
		code++;
	}
	putchar('\n');

	return 0;
}
