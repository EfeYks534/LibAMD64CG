#include <stddef.h>
#include <stdint.h>

#define REG_RAX 0
#define REG_RCX 1
#define REG_RDX 2
#define REG_RBX 3
#define REG_RSP 4
#define REG_RBP 5
#define REG_RSI 6
#define REG_RDI 7
#define REG_R8  8
#define REG_R9  9
#define REG_R10 10
#define REG_R11 11
#define REG_R12 12
#define REG_R13 13
#define REG_R14 14
#define REG_R15 15

#define LOCT_LABEL 0

#define TYPE_U8  0
#define TYPE_U16 1
#define TYPE_U32 2
#define TYPE_U64 3

#define MOD_DIR 0
#define MOD_IND 1
#define MOD_RBP 2


struct BinLocator
{
	uint32_t type;
	uint32_t addr;
};

struct BinIArg
{
	uint8_t   mod;
	uint8_t     r;
	uint8_t    rm;
	uint8_t  rsvd;
	uint64_t   im;
};


int BinGenInstr(uint8_t **bin, struct BinIArg *arg, const char *ident);
