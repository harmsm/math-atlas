#include "ifko.h"
#include "fko_arch.h"

static short type2len(int type)
{
   short len=4;
   #ifdef ArchPtrIsLong
      if (type == T_INT || type == T_DOUBLE) len = 8;
   #else
      if (type == T_DOUBLE) len = 8;
   #endif
   return(len);
}
static short type2shift(int type)
{
   short len=2;
   #ifdef  ArchPtrIsLong
      if (type == T_DOUBLE || type == T_INT) len = 3;
   #else
      if (type == T_DOUBLE) len = 3;
   #endif
   return(len);
}

static int LocalLoad(short id)
/*
 * Loads the value stored in local id
 * RETURNS: register loaded to
 */
{
   short inst=5555, reg, typ;

   id--;
   if (IS_CONST(STflag[id]))
   {
      typ = FLAG2TYPE(STflag[id]);
      reg = GetReg(typ);
      switch(typ)
      {
      case T_INT:
         inst = MOV;
         break;
      case T_FLOAT:
         inst = FMOV;
         break;
      case T_DOUBLE:
         inst = FMOVD;
         break;
      default:
         fprintf(stderr, "type=%d!!\n\n", typ);
      }
      InsNewInst(NULL, NULL, inst, -reg, id+1, 0);
      return(reg);
   }
   else if (IS_PTR(STflag[id]))
   {
      inst = LD;
      reg = GetReg(T_INT);
   }
   else
   {
      typ = FLAG2TYPE(STflag[id]);
      reg = GetReg(typ);
      switch(typ)
      {
      case T_INT:
         inst = LD;
         break;
      case T_FLOAT:
         inst = FLD;
         break;
      case T_DOUBLE:
         inst = FLDD;
         break;
      default:
         fprintf(stderr, "type=%d!!\n\n", typ);
      }
   }
   InsNewInst(NULL, NULL, inst, -reg, SToff[id].sa[2], 0);
   return(reg);
}

static void LocalStore(short id, short sreg)
{
   short inst;

   switch(FLAG2PTYPE(STflag[id-1]))
   {
   case T_INT:
      inst = ST;
      break;
   case T_FLOAT:
      inst = FST;
      break;
   case T_DOUBLE:
      inst = FSTD;
      break;
   default:
      fko_error(__LINE__, "Unknown type!\n");
   }
   InsNewInst(NULL, NULL, inst, SToff[id-1].sa[2], -sreg, 0);
}

void DoConvert(short dest, short src)
{
   fprintf(stderr, "Conversions not yet supported\n");
   exit(-1);
}

void DoFpConstLoad(short dest, short src)
{
   char *ln;
   int type;
   
fprintf(stderr, "Handling fpconst!\n");
/*
 * Encode fp const load as special case of FMOV.
 */
   type = FLAG2TYPE(STflag[dest-1]);
   InsNewInst(NULL, NULL, type == T_FLOAT ? FMOV : FMOVD, 
              -GetReg(type), src, __LINE__);
}

void DoComment(char *str)
{
   int i;
   for (i=0; str[i]; i++);
   if (str[--i] == '\n') str[i] = '\0';
   InsNewInst(NULL, NULL, COMMENT, STstrconstlookup(str), 0, 0);
}

void DoMove(short dest, short src)
{
   short rsrc;
   int sflag, type;
   enum inst mov;
   sflag = STflag[src-1];

fprintf(stderr, "DoMove %d %d (%s %s)\n", dest, src, STname[dest-1]?STname[dest-1] : "NULL", STname[src-1]?STname[src-1] : "NULL");

   if (IS_CONST(sflag))
   {
      type = FLAG2PTYPE(sflag);
      rsrc = GetReg(type);
      if (IS_CONST(sflag) && (type == T_INT) && SToff[src-1].i == 0)
         InsNewInst(NULL, NULL, XOR, -rsrc, -rsrc, -rsrc);
      else
      {
         if (type == T_INT) mov = MOV;
         else if (type == T_FLOAT) mov = FMOV;
         else 
         {
            assert(type == T_DOUBLE);
            mov = FMOVD;
         }
         InsNewInst(NULL, NULL, mov, -rsrc, src, __LINE__);
      }
      LocalStore(dest, rsrc);
   }
   else if (FLAG2TYPE(STflag[dest-1]) == FLAG2TYPE(sflag))
   {
      rsrc = LocalLoad(src);
      LocalStore(dest, rsrc);
   }
   else DoConvert(dest, src);
   GetReg(-1);
}

short AddArrayDeref(short array, short index, int offset)
/*
 * offset is integer constant.  array,index are entries in the ST.
 *    array[index+offset]
 */
{
   int mul=4, flag;
   flag = STflag[array-1];
   if (IS_DOUBLE(flag)) mul = 8;
   else if (IS_CHAR(flag)) mul = 1;
   assert(!IS_VEC(flag));
   return(AddDerefEntry(array, index, mul, offset));
}
static void FixDeref(short ptr)
/*
 * This routine takes a deref entry of type:
 *    <STloc> <STloc> <mul> <STconst>
 * And translates into a fully-specified legal index for the machine:
 *    <reg0> <reg1> <mul> <STconst>
 * where the address is then provided by <reg0> + <reg1>*mul + <STconst>.
 * On some machines, mul may need to be 1, resulting in an extra shift
 * instruction, and on others you may be able to use one of <reg1> or <STconst>
 * at a time, resulting in an additional add.
 */
{
   short k, type;

   k = (ptr-1)<<2;
   type = FLAG2TYPE(STflag[DT[k]-1]);
fprintf(stderr, "FixDeref: [%d, %d, %d, %d]\n", DT[k], DT[k+1], DT[k+2], DT[k+3]);
/*
 * Load beginning of array
 */
   DT[k] = -LocalLoad(DT[k]);
/*
 * Multiply constant by mul
 */
   if (DT[k+2]) DT[k+3] *= DT[k+2];
/*
 * Load index register if needed
 */
   if (DT[k+1])
   {
      DT[k+1] = -LocalLoad(DT[k+1]);
/*
 *    Some architectures cannot multiply the index register by some (or any)
 *    constants, and in this case generate an extra shift instruction
 */
      if (!ArchHasLoadMul(DT[k+2]))
      {
         InsNewInst(NULL, NULL, SHL, DT[k+1], DT[k+1], 
                    STiconstlookup(type2shift(type)));
         DT[k+2] = 1;
      }
/*
 *    On machines with fixed-size instructions, you usually need to choose
 *    _either_ an index register, _or_ a constant addition.  If we have both
 *    on such a machine, add the constant to the index register
 */
      #ifndef ArchConstAndIndex
         if (DT[k+3])
         {
            InsNewInst(NULL, NULL, ADD, DT[k+1], DT[k+1], 
                       STiconstlookup(DT[k+3]));
            DT[k+3] = 0;
         }
      #endif
   }
fprintf(stderr, "%s(%d)\n", __FILE__,__LINE__);
}

void DoArrayStore(short ptr, short id)
{
   INSTQ *ip;
   short lreg, ireg=0, preg, k, type;

   k = (ptr-1)<<2;
   type = FLAG2TYPE(STflag[id-1]);
   fprintf(stderr, "pnam=%s, pflag=%d, idname='%s', idflag=%d\n", 
           STname[DT[k]-1], STflag[DT[k]-1], STname[id-1], STflag[id-1]);

   lreg = LocalLoad(id);
   assert(FLAG2TYPE(STflag[DT[k]-1]) == type);
   FixDeref(ptr);
   switch(type)
   {
   case T_INT:
      #ifdef X86_64
         assert(lreg < 8);
         InsNewInst(NULL, NULL, STS, ptr, -lreg, 0);
      #else
         InsNewInst(NULL, NULL, ST, ptr, -lreg, 0);
      #endif
      break;
   case T_FLOAT:
      InsNewInst(NULL, NULL, FST, ptr, -lreg, 0);
      break;
   case T_DOUBLE:
      InsNewInst(NULL, NULL, FSTD, ptr, -lreg, 0);
      break;
   default:
      fko_error(__LINE__, "Unknown type %d\n", type);
   }
   GetReg(-1);
}

void DoArrayLoad(short id, short ptr)
/*
 * id is ST index, ptr is DT index
 */
{
   short k, ireg=0, areg, type, ld;

   k = (ptr-1)<<2;
   type = FLAG2TYPE(STflag[id-1]);
   FixDeref(ptr);
fprintf(stderr, "\n\nANAME=%s, TYPE=%d\n\n", STname[ptr-1] ? STname[ptr-1] : "NULL", type);
   switch(type)
   {
   case T_INT:
/*
 *    Special case for integer arrays, which are still 32-bits on x86-64
 */
      #ifdef X86_64
         areg = GetReg(T_SHORT);
         InsNewInst(NULL, NULL, LDS, -areg, ptr, 0);
         if (!IS_UNSIGNED(STflag[id-1]))
         {
            k = STiconstlookup(31);
            InsNewInst(NULL, NULL, SHL, -areg, -areg, k);
            InsNewInst(NULL, NULL, SAR, -areg, -areg, k);
            LocalStore(id, areg);
            GetReg(-1);
            return;
         }
      #endif
      ld = LD;
      break;
   case T_FLOAT:
      ld = FLD;
      break;
   case T_DOUBLE:
      ld = FLDD;
      break;
   default:
      fko_error(__LINE__, "Unknown type %d\n", type);
   }
   areg = GetReg(type);
   InsNewInst(NULL, NULL, ld, -areg, ptr, 0);
   LocalStore(id, areg);
   GetReg(-1);
}

void HandlePtrArith(short dest, short src0, char op, short src1)
/*
 * Ptr arithmetic must be of form <ptr> = <ptr> [+,-] <int/const>
 */
{
   short rd, rs0, rs1, flag, type, dflag, k;

   if (op != '+' && op != '-')
      yyerror("pointers may take only + and - operators");
   if (!IS_PTR(STflag[src0-1]))
      yyerror("Expecting <ptr> = <ptr> + <int>");

   dflag = STflag[dest-1];
   type = FLAG2TYPE(dflag);
   rs0 = LocalLoad(src0);
   flag = STflag[src1-1];


   if (IS_CONST(flag))
   {
      if (IS_INT(flag))
      #ifdef X86_64
      {
         if (IS_INT(dflag)) rs1 = -STiconstlookup(SToff[src1-1].i*4);
         else rs1 = -STiconstlookup(SToff[src1-1].i*type2len(type));
      }
      #else
         rs1 = -STiconstlookup(SToff[src1-1].i*type2len(type));
      #endif
      else
         yyerror("Pointers may only be incremented by integers");
   }
   else
   {
      rs1 = LocalLoad(src1);
      #ifdef X86_64
         if (IS_INT(dflag)) k = STiconstlookup(2);
         else k = STiconstlookup(type2shift(type));
         InsNewInst(NULL, NULL, SHL, -rs1, -rs1, k);
      #else
         InsNewInst(NULL, NULL, SHL, -rs1, -rs1, 
                    STiconstlookup(type2shift(type)));
      #endif
   }
   InsNewInst(NULL, NULL, op == '+' ? ADD : SUB, -rs0, -rs0, -rs1);
   LocalStore(dest, rs0);
   GetReg(-1);
}

void DoArith(short dest, short src0, char op, short src1)
{
   short rd, rs0, rs1, type;
   enum inst inst;
   extern int DTnzerod, DTnzero, DTabs, DTabsd;

   if (IS_PTR(STflag[dest-1]))
   {
      HandlePtrArith(dest, src0, op, src1);
      return;
   }
   type = FLAG2TYPE(STflag[dest-1]);
/*
 * x86 insts must handle integer division specially
 */
   #ifdef X86
      if (op == '/' && IS_INT(type))
      {
         #ifdef X86_64
            rd = iName2Reg("@rax");
            rs0 = iName2Reg("@rdx");
            rs1 = iName2Reg("@rcx");
         #else
            rd = iName2Reg("@eax");
            rs0 = iName2Reg("@edx");
            rs1 = iName2Reg("@ecx");
         #endif
         InsNewInst(NULL, NULL, LD, -rd, SToff[src0-1].sa[2], 0);
         InsNewInst(NULL, NULL, MOV, -rs0, -rd, 0);
         InsNewInst(NULL, NULL, SAR, -rs0, -rs0, STiconstlookup(31));
         if (IS_CONST(STflag[src1-1]))
            InsNewInst(NULL, NULL, MOV, -rs1, 
                       STiconstlookup(SToff[src1-1].i), 0);
         else
            InsNewInst(NULL, NULL, LD, -rs1, SToff[src1-1].sa[2], 0);
         InsNewInst(NULL, NULL, DIV, -rd, -rs0, -rs1);
         fprintf(stderr, "DIV %d, %d, %d\n", -rd, -rs0, -rs1);
         LocalStore(dest, rd);
         GetReg(-1);
         return;
      }
   #endif
   rd = rs0 = LocalLoad(src0);
   if (op != 'n' && op != 'a')
   {
      if (src0 != src1)
      {
         if (IS_CONST(STflag[src1-1])) rs1 = -src1;
         else rs1 = LocalLoad(src1);
      }
      else rs1 = rs0;
   }
   else rs1 = 0;
   if ( (op == '%' || op == '>' || op == '<') && 
        (type != T_INT && type != T_SHORT) )
      yyerror("modulo and shift not defined for non-integer types");
   switch(op)
   {
   case '+': /* dest = src0 + src1 */
      switch(type)
      {
      case T_INT:
         inst = ADD;
         break;
      case T_FLOAT:
         inst = FADD;
         break;
      case T_DOUBLE:
         inst = FADDD;
         break;
      }
      break;
   case '-': /* dest = src0 - src1 */
      switch(type)
      {
      case T_INT:
         inst = SUB;
         break;
      case T_FLOAT:
         inst = FSUB;
         break;
      case T_DOUBLE:
         inst = FSUBD;
         break;
      }
      break;
   case '*': /* dest = src0 * src1 */
      switch(type)
      {
      case T_INT:
         inst = MUL;
         break;
      case T_FLOAT:
         inst = FMUL;
         break;
      case T_DOUBLE:
         inst = FMULD;
         break;
      }
      break;
   case '/': /* dest = src0 / src1 */
      switch(type)
      {
      case T_INT:
         inst = DIV;
         break;
      case T_FLOAT:
         inst = FDIV;
         break;
      case T_DOUBLE:
         inst = FDIVD;
         break;
      }
      break;
   case '%': /* dest = src0 % src1 */
      yyerror("% not yet supported");
      break;
   case '>': /* dest = src0 >> src1 */
      if (type == T_INT) inst = SHR;
      break;
   case '<': /* dest = src0 << src1 */
      if (type == T_INT) inst = SHL;
      break;
   case 'm': /* dest += src0 * src1 */
      if (type == T_FLOAT || type == T_DOUBLE)
      {
         rd = LocalLoad(dest);
         #ifdef ArchHasMAC
            if (type == T_FLOAT) inst = FMAC;
            else isnt = FMACD;
         #else
            if (type == T_FLOAT)
            {
               InsNewInst(NULL, NULL, FMUL, -rs0, -rs0, -rs1);
               inst = FADD;
            }
            else
            {
               InsNewInst(NULL, NULL, FMULD, -rs0, -rs0, -rs1);
               inst = FADDD;
            }
         #endif
      }
      else yyerror("MAC available for floating point operands only!");
      break;
   case 'n': /* dest = -src0 */
      switch(type)
      {
      case T_INT:
         inst = NEG;
         break;
      case T_FLOAT:
         inst = FNEG;
	 DTnzero = -1;
         break;
      case T_DOUBLE:
         inst = FNEGD;
	 DTnzerod = -1;
         break;
      }
      break;
   case 'a': /* dest = abs(src0) */
      switch(type)
      {
/*
      case T_INT:
         inst = ABS;
         break;
*/
      case T_FLOAT:
         inst = FABS;
	 DTabs = -1;
         break;
      case T_DOUBLE:
         inst = FABSD;
	 DTabsd = -1;
         break;
      default:
         fko_error(__LINE__, "ABS available for floating point only!\n");
      }
      break;
   }
   InsNewInst(NULL, NULL, inst, -rd, -rs0, -rs1);
   LocalStore(dest, rs0);
   GetReg(-1);
}

void DoReturn(short rret)
{
   int retreg, srcreg;
   int mov, ld, type;
   if (rret)
   {
      type = FLAG2PTYPE(STflag[rret-1]);
      switch(type)
      {
      case T_INT:
         mov = MOV;
	 ld = LD;
         rout_flag |= IRET_BIT;
         retreg = IRETREG;
         break;
      case T_FLOAT:
         mov = FMOV;
         ld = FLD;
         rout_flag |= FRET_BIT;
         retreg = FRETREG;
         break;
      case T_DOUBLE:
         mov = FMOVD;
         ld = FLDD;
         rout_flag |= DRET_BIT;
         retreg = DRETREG;
         break;
      default:
         fprintf(stderr, "UNKNOWN TYPE %d on line %d of %s\n", 
                 type, __LINE__, __FILE__);
         exit(-1);
      }
#if 0
      if (IS_CONST(STflag[rret-1])) srcreg = -rret;
      else srcreg = LocalLoad(rret);
      InsNewInst(NULL, NULL, mov, -retreg, -srcreg, 0);
#else
      if (IS_CONST(STflag[rret-1]))
         InsNewInst(NULL, NULL, mov, -retreg, rret, 0);
      else 
         InsNewInst(NULL, NULL, ld, -retreg, SToff[rret-1].sa[2], 0);
#endif
   }
   InsNewInst(NULL, NULL, JMP, 0, STstrconstlookup("IFKO_EPILOGUE"), 0);
   GetReg(-1);
}

static short GetSignInfo(short k)
/*
 * Calculates sign info for ST entry k
 */
{
   int flag, i;
   short kret=0;
   flag = STflag[k-1];
   if (IS_CONST(flag))
   {
      assert(IS_INT(flag));
      i = SToff[k-1].i;
      if (i > 0) kret = 2;
      else if (i == 0) kret = 1;
      else if (i < 0) kret = -2;
      else kret = -1;
   }
   else if (IS_UNSIGNED(flag)) kret = 1;
   return(kret);
}

struct loopq *DoLoop(short I, short start, short end, short inc,
                     short sst, short send, short sinc)
/*
 * sst, send, sinc indicate sign of each parameter:
 *    0 : unknown
 *    1 : >= 0
 *    2 : >  0
 *   -1 : <= 0
 *   -2 : <  0
 */
{
   int flag=0;
   struct loopq *lp;
   short ireg;
   char lnam[128];
fprintf(stderr, "%s(%d) I=%d, start=%d, end=%d\n", __FILE__, __LINE__,I,start,end);
/*
 * If signs of loop parts are unknown, see if we can deduce them
 */
   if (!sst) sst = GetSignInfo(start);
   if (!send) send = GetSignInfo(end);
   if (!sinc) sinc = GetSignInfo(inc);
   if (sst)
   {
      if (sst == 2) flag |= L_PSTART_BIT;
      else if (sst == 1) flag |= L_PSTART_BIT | L_ZSTART_BIT;
      else if (sst == -1) flag |= L_NSTART_BIT | L_ZSTART_BIT;
      else /* if (sst == -2) */ flag |= L_NSTART_BIT;
   }
   if (send)
   {
      if (send == 2) flag |= L_PEND_BIT;
      else if (send == 1) flag |= L_PEND_BIT | L_ZEND_BIT;
      else if (send == -1) flag |= L_NEND_BIT | L_ZEND_BIT;
      else /* if (send == -2) */ flag |= L_NEND_BIT;
   }
   if (sinc)
   {
      if (sinc == 2) flag |= L_PINC_BIT;
      else /* if (sinc == -2) */ flag |= L_NINC_BIT;
   }
fprintf(stderr, "%s(%d)\n", __FILE__, __LINE__);
   lp = NewLoop(flag);
fprintf(stderr, "%s(%d)\n", __FILE__, __LINE__);
   lp->I = I;
   lp->beg = start;
   lp->end = end;
   lp->inc = inc;

   lp->ibeg = InsNewInst(NULL, NULL, CMPFLAG, CF_LOOP_INIT, lp->loopnum, 0);
   flag = STflag[start-1];
   assert(!IS_PTR(flag));
   if (IS_CONST(flag)) DoMove(I, start);
   else
   {
      ireg = LocalLoad(start);
      LocalStore(I, ireg);
      GetReg(-1);
   }
   InsNewInst(NULL, NULL, CMPFLAG, CF_LOOP_BODY, lp->loopnum, 0);
   sprintf(lnam, "_LOOP_%d", lp->loopnum);
   lp->body_label = STstrconstlookup(lnam);
   InsNewInst(NULL, NULL, LABEL, lp->body_label, lp->loopnum, 0);
   return(lp);
}

void FinishLoop(struct loopq *lp)
/*
 * After loop_begin and loop_body written, writes loop_update and test
 */
{
   short ireg, iend;
   int flag;
/*
 * Update loop counter
 */
   InsNewInst(NULL, NULL, CMPFLAG, CF_LOOP_UPDATE, lp->loopnum, 0);
   DoArith(lp->I, lp->I, '+', lp->inc);
   InsNewInst(NULL, NULL, CMPFLAG, CF_LOOP_TEST, lp->loopnum, 0);
   ireg = LocalLoad(lp->I);
   flag = STflag[lp->end-1];
   if (IS_CONST(flag)) iend = lp->end;
   else iend = -LocalLoad(lp->end);
   InsNewInst(NULL, NULL, CMP, 0, -ireg, iend);
   InsNewInst(NULL, NULL, JLT, ICC0, 0, lp->body_label);
   lp->iend = InsNewInst(NULL, NULL, CMPFLAG, CF_LOOP_END, lp->loopnum, 0);
   GetReg(-1);
}

void DoLabel(char *name)
{
   InsNewInst(NULL, NULL, LABEL, STlabellookup(name), 0, 0);
}

void DoIf(char op, short id, short avar, char *labnam)
{
   int flag, type;
   short k, cmp, ireg, ireg1, freg0, freg1, br, label;
   assert(id > 0 && label > 0 && avar > 0);
   label = STlabellookup(labnam);
   flag = STflag[avar-1];
   type = FLAG2PTYPE(STflag[id-1]);
fprintf(stderr, "%s(%d): label=%s\n", __FILE__,__LINE__, STname[label-1]);
   if (type == T_INT)
   {
fprintf(stderr, "%s(%d)\n", __FILE__,__LINE__);
      ireg = LocalLoad(id);
      if (IS_CONST(flag)) ireg1 = -avar;
      else ireg1 = LocalLoad(avar);
      if (op != '&') InsNewInst(NULL, NULL, CMP, ICC0, -ireg, -ireg1);
      else
         #ifdef PPC
            InsNewInst(NULL, NULL, ANDCC, -ireg, -ireg, -ireg1);
         #else
            InsNewInst(NULL, NULL, CMPAND, -ireg, -ireg, -ireg1);
         #endif
      switch(op)
      {
      case '>':
         InsNewInst(NULL, NULL, JGT, 0, ICC0, label);
         break;
      case 'g':  /* >= */
         InsNewInst(NULL, NULL, JGE, 0, ICC0, label);
         break;
      case '<':
         InsNewInst(NULL, NULL, JLT, 0, ICC0, label);
         break;
      case 'l':  /* <= */
         InsNewInst(NULL, NULL, JLE, 0, ICC0, label);
         br = JLE;
         break;
      case '=':
         InsNewInst(NULL, NULL, JEQ, 0, ICC0, label);
         break;
      case '!':
         InsNewInst(NULL, NULL, JNE, 0, ICC0, label);
         break;
      case '&':
         InsNewInst(NULL, NULL, JEQ, 0, ICC0, label);
         break;
      }
   }
#ifndef X86
   else
   {
      freg0 = LocalLoad(id);
      freg1 = LocalLoad(avar);
      cmp = (type == T_FLOAT) ? FCMP : FCMPD;
      switch(op)
      {
      case '>':
         br = JGT;
         break;
      case 'g':  /* >= */
         br = JGE;
         break;
      case '<':
         br = JLT;
         break;
      case 'l':  /* <= */
         br = JLE;
         break;
      case '=':
         br = JEQ;
         break;
      case '!':
         br = JNE;
         break;
      default:
         fko_error(__LINE__, "Illegal fp comparitor '%c'\n", op);
      }
      InsNewInst(NULL, NULL, cmp, FCC0, -freg0, -freg1);
      InsNewInst(NULL, NULL, br, 0, FCC0, label);
   }
#else
      else
      {
         assert(type == T_FLOAT || type == T_DOUBLE);
         ireg = GetReg(T_INT);
         freg0 = LocalLoad(id);
         freg1 = LocalLoad(avar);
         switch(op)
         {
         case '>':
            k = STiconstlookup(2);
            br = JEQ;
            break;
         case 'l':  /* <= */
            k = STiconstlookup(2);
            br = JNE;
            break;
         case 'g':  /* >= */
            k = STiconstlookup(1);
            br = JEQ;
            break;
         case '<':
            k = STiconstlookup(1);
            br = JNE;
            break;
         case '=':
            k = STiconstlookup(0);
            br = JNE;
            break;
         case '!':
            k = STiconstlookup(0);
            br = JEQ;
            break;
         }
         InsNewInst(NULL, NULL, (type == T_FLOAT) ? FCMPW:FCMPWD, 
                    -freg0, -freg1, k);
         InsNewInst(NULL, NULL, (type == T_FLOAT) ? CVTBFI:CVTBDI,
                    -ireg, -freg0, 0);
         InsNewInst(NULL, NULL, CMP, ICC0, -ireg, STiconstlookup(0));
         InsNewInst(NULL, NULL, br, 0, ICC0, label);
         return;
      }
#endif
}
