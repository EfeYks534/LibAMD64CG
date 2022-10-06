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


/* The `mod` field specifies the addressing mode.
 *  MOD_DIR is register-direct addressing
 *  MOD_IND is register-indirect addressing
 *  MOD_RBP is a 8/32 bit displacement off of RBP
 * The `r` and `rm` fields specify the reg and reg/mem fields of Mod/RM.
 * The `im` field is an immediate, the size is deduced from the instruction identifier.
 */
struct BinIArg
{
	uint8_t   mod;
	uint8_t     r;
	uint8_t    rm;
	uint8_t  rsvd;
	uint64_t   im;
};


int BinGenInstr(uint8_t **bin, struct BinIArg *arg, const char *ident);
/* `*bin` is the code buffer. It is incremented by the size of the instruction written.
 * `*bin` must be able to hold at least 15 bytes.  `*arg` is the instruction arguments.
 * `arg->r` is ignored if '/digit' is in the instruction identifier. `im` is ignored if
 * there is no ib/iw/id/iq in the instruction identifier and the addressing mode is not
 * MOD_RBP.

 * `ident` is the instruction identifier string.
 * It is in the form of "MNEMONIC | OPCODE"
 * The instruction mnemonic is in the form of "INSTR_NAME arg1 arg2"
 * arg1 and arg2 are optional 2 character strings. The first character is the argument
 * kind, the second character is the type. Arguments are separated with space.
 * The argument kind is one of r/m/i (reg / reg/mem / immediate).
 * The argument type is one of b/w/d/q (BYTE/WORD/DWORD/QWORD).
 * Below is the identifier for the ADD reg/mem64, imm8 instruction.
 *   "ADD rq ib | 83 /0 ib"
 * The opcode is how the instruction is represented in memory. It is separated into
 * multiple opcode-bytes. Each one is represented by a word separated with space.
 * Below are the possible opcode-byte strings.
 *  "XX"     - A single byte hexadecimal integer that is placed on memory.
 *  "/r"     - Represents the Mod/RM byte.
 *  "/digit" - Same as /r, but the reg field is replaced by the digit (0 <= digit <= 7)
 *  "it"     - Represents a (multi)byte immediate. 't' is the type (b/w/d/q).
 *  "+r"     - Adds `arg->r` to the last byte on memory
 *  "L"      - Puts the legacy prefix (0x66) on memory
 *  "R"      - Enables the REX prefix. Mod/RM can change it based on reg and reg/mem
 *  "W"      - Enables the REX prefix and sets the W bit, making it 64 bit.
 * Below is the identifier for the POP reg64 instruction.
 *   "POP rq | 58 +rq"

 * The return value is 0 for failure and 1 for success. 0 means no bytes were written.
 */
