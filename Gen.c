#include "AMD64CG.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define II_MR 0
#define II_RM 1
#define II_MI 2
#define II_RI 3
#define II_I_ 4

#define OPC_IMM    0
#define OPC_IMMB   1
#define OPC_MODRM  2
#define OPC_REG    3
#define OPC_ADDR   4
#define OPC_LEGACY 5
#define OPC_REX    6
#define OPC_REXW   7


struct OpcByte
{
	uint8_t type;
	uint8_t  imm;
};

struct InstrIdent
{
	char name[8];

	uint8_t oprc;

	uint8_t r_type : 2;
	uint8_t i_type : 2;
	uint8_t  order : 4;

	uint8_t opc;
	struct OpcByte op[6];
};

struct InstrBin
{
	uint8_t mask;
	uint8_t  leg;
	uint8_t  rex;

	uint8_t  opcsz;
	uint8_t opcode[14];
};


static const char *type_names[] = { "8", "16", "32", "64" };
static const char *type_names2 = "bwdq";


static uint8_t Chr2Type(const char ch)
{
	switch(ch)
	{
	case 'b': return TYPE_U8;
	case 'w': return TYPE_U16;
	case 'd': return TYPE_U32;
	case 'q': return TYPE_U64;
	default:  return TYPE_U32;
	}
}


static uint8_t XD2U8(const char ch)
{
	if(ch >= '0' && ch <= '9')
		return ch - '0';

	if(ch >= 'A' && ch <= 'F')
		return ch - 'A' + 0xA;

	if(ch >= 'a' && ch <= 'f')
		return ch - 'a' + 0xA;
	return 0;
}

static int ParseMnemonicTok(struct InstrIdent *id, int i, const char *tok)
{
	if(*tok == '|')
		return 0;

	switch(i)
	{
	case 0:
		strncpy(id->name, tok, 7);
		id->name[7] = 0;
		break;
	case 1:
		if(*tok != 'r' && *tok != 'm' && *tok != 'i')
			return -1;

		id->r_type = Chr2Type(tok[1]);

		if(*tok == 'r')
			id->order = II_RM;
		else if(*tok == 'm')
			id->order = II_MR;
		else
			id->order = II_I_;

		id->oprc = 1;
		break;
	case 2:
		if(*tok != 'r' && *tok != 'm' && *tok != 'i')
			return -1;

		id->i_type = Chr2Type(tok[1]);

		if(id->order == II_I_)
			return -1;

		if((*tok == 'r' && id->order == II_RM) ||
		   (*tok == 'm' && id->order == II_MR))
			return -1;

		if(*tok == 'i')
			id->order = id->order == II_RM ? II_RI : II_MI;

		id->oprc = 2;
		break;
	case 3:
		if(*tok != '|')
			return -1;
		return 0;
	}

	return 1;
}

static int ParseOpcodeTok(struct InstrIdent *id, int i, const char *tok)
{
	if(i == 6)
		return -1;

	id->opc++;

	struct OpcByte *ob = &id->op[i];

	if(*tok == '/') {
		if(tok[1] == 'r') {
			ob->type = OPC_MODRM;
			return 1;
		}

		if(tok[1] < '0' || tok[1] > '9')
			return -1;

		ob->type = OPC_REG;
		ob->imm  = tok[1] - '0';

		return 1;
	} else if(*tok == 'i') {
		ob->type = OPC_IMMB;
		ob->imm  = Chr2Type(tok[1]);
		return 1;
	} else if(*tok == '+') {
		if(tok[1] == 'r') {
			ob->type = OPC_ADDR;
			ob->imm  = Chr2Type(tok[2]);
			return 1;
		}
		return -1;
	} else if(*tok == 'L') {
		ob->type = OPC_LEGACY;
		return 1;
	} else if(*tok == 'R') {
		ob->type = OPC_REX;
		return 1;
	} else if(*tok == 'W') {
		ob->type = OPC_REXW;
		return 1;
	}

	if(!isxdigit(*tok) || !isxdigit(tok[1]))
		return -1;

	ob->type = OPC_IMM;
	ob->imm = (XD2U8(*tok) << 4) | XD2U8(tok[1]);

	return 1;
}

static int ParseInstrIdent(struct InstrIdent *id, char *ident)
{
	char *token = strtok(ident, " ");

	if(token == NULL)
		return -1;

	int r = 1;
	int i = 0;

	while(r == 1) {
		r = ParseMnemonicTok(id, i++, token);
		token = strtok(NULL, " ");

		if(token == NULL)
			return -1;
	}

	if(r == -1)
		return -1;

	r = 1;
	i = 0;

	while(r == 1) {
		r = ParseOpcodeTok(id, i++, token);
		token = strtok(NULL, " ");

		if(token == NULL) {
			r = 0;
			break;
		}
	}

	if(r == -1)
		return -1;

	return 1;
}

/*
static void PrintInstr(struct InstrIdent *id)
{
	printf("%s ", id->name);

	switch(id->order)
	{
	case II_MR: case II_MI: printf("reg/mem%s", type_names[id->r_type]); break;
	case II_RM: case II_RI: printf("reg%s", type_names[id->r_type]); break;
	}

	if(id->oprc == 2) {
		printf(", ");

		switch(id->order)
		{
		case II_MR: printf("reg%s", type_names[id->i_type]); break;
		case II_RM: printf("reg/mem%s", type_names[id->i_type]); break;
		case II_RI: case II_MI: printf("imm%s", type_names[id->i_type]); break;
		}
	}

	printf(" | ");

	for(int i = 0; i < id->opc; i++) {
		struct OpcByte *ob = &id->op[i];
		switch(ob->type)
		{
		case OPC_IMM:
			printf("%02X ", ob->imm);
			break;
		case OPC_IMMB:
			printf("i%c ", type_names2[ob->imm]);
			break;
		case OPC_MODRM:
			printf("/r ");
			break;
		case OPC_REG:
			printf("/%d ", ob->imm);
			break;
		case OPC_ADDR:
			printf("+r%c ", type_names2[ob->imm]);
			break;
		}
	}

	putchar('\n');
}
*/

static int InstrGenBin(uint8_t **bin, struct InstrIdent *id, struct BinIArg *arg)
{
	struct InstrBin ib = (struct InstrBin) { 0 };

	for(int i = 0; i < id->opc; i++) {
		struct OpcByte *ob = &id->op[i];

		switch(ob->type)
		{
		case OPC_IMM:
			ib.opcode[ib.opcsz++] = ob->imm;
			break;
		case OPC_IMMB:
			switch(ob->imm)
			{
			case TYPE_U8:
				ib.opcode[ib.opcsz++] = arg->im;
				break;
			case TYPE_U16:
				*((uint16_t *) &ib.opcode[ib.opcsz]) = arg->im;
				ib.opcsz += 2;
				break;
			case TYPE_U32:
				*((uint32_t *) &ib.opcode[ib.opcsz]) = arg->im;
				ib.opcsz += 4;
				break;
			case TYPE_U64:
				*((uint16_t *) &ib.opcode[ib.opcsz]) = arg->im;
				ib.opcsz += 8;
				break;
			}
			break;
		case OPC_REG:
		case OPC_MODRM: {
			if(arg->mod == MOD_RBP) {
				uint8_t modrm = (arg->im > 127 ? 128 : 64) | 5;
				modrm |= (arg->r & 0x7) << 3;

				if(arg->r > 7)
					ib.rex |= 0x44;
			

				ib.opcode[ib.opcsz++] = modrm;

				if(arg->im > 0x7FFFFFFF)
					return -1;

				if(arg->im > 127) {
					*((uint32_t *) &ib.opcode[ib.opcsz]) = arg->im;
					ib.opcsz += 4;
				} else {
					ib.opcode[ib.opcsz++] = arg->im;
				}

				break;
			}

			uint8_t modrm = arg->mod == MOD_DIR ? 0xC0 : 0x00;

			if(ob->type == OPC_REG) {
				modrm |= ob->imm << 3;
			} else {
				modrm |= (arg->r & 0x7) << 3;
				if(arg->r > 7) {
					ib.rex |= 0x44;
				}
			}
			modrm |= (arg->rm & 0x7);
			if(arg->rm > 7) {
				ib.rex |= 0x41;
			}

			ib.opcode[ib.opcsz++] = modrm;
			break;
		  }
		case OPC_ADDR:
			if(arg->r > 7)
				return -1;
			ib.opcode[ib.opcsz - 1] += arg->r;
			break;
		case OPC_LEGACY:
			ib.mask |= 1;
			ib.leg = 0x66;
			break;
		case OPC_REX:
			ib.mask |= 2;
			break;
		case OPC_REXW:
			ib.mask |= 2;
			ib.rex |= 0x48;
			break;
		}
	}

	if(ib.mask & 1)
		*((*bin)++) = ib.leg;
	if((ib.mask & 2) && ib.rex != 0)
		*((*bin)++) = ib.rex;

	memcpy(*bin, ib.opcode, ib.opcsz);

	*bin += ib.opcsz;

	return 1;
}


int BinGenInstr(uint8_t **bin, struct BinIArg *arg, const char *const_ident)
{
	char *ident = strdup(const_ident);

	struct InstrIdent id = (struct InstrIdent) { 0 };

	if(ParseInstrIdent(&id, ident) == 1) {
		//PrintInstr(&id);

		if(InstrGenBin(bin, &id, arg) == 1) {
			free(ident);
			return 1;
		}
	}

	free(ident);

	return 0;
}
