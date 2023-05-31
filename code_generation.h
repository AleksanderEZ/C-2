#ifndef CODE_GENERATION_H
#define CODE_GENERATION_H

#include "symbol_table.h"

void advanceLabel();
void newLabel();
void setObjFile(char* objPath);
int qAssignRegister();
void qFreeRegister(int reg);
void qFreeRegisters();
void qInit();
void qEnd();
void qLine();
void qStat();
void qCode();
void qCallFunction(char* function, char* arguments);
void qCallFunctionNoArgs(char* function);
void qMalloc(int reg);
void qSizeOf(char* expression);
void qPrintReg(int reg);
void qPrintExplicit(char* expression);
void qPrintExplicitFormat(char* formatString, char* arguments);
void qPrintImplicitFormat(char* identifier, char* arguments);
void qStartWhile();
void qWhileCondition();
void qInstruction(char* instruction);
void qFinishWhile();
void qLoadVar(int reg, char* identifier, enum RegType regType);
void qStoreVar(int reg, char* identifier, enum RegType regType);
void qLoadGlobal(int reg, char* identifier);
void qStoreGlobal(int reg, char* identifier);
void qLoadIntValue(int reg, int value);
void qLoadFloatValue(int reg, float value);
void qLoadCharValue(int reg, char value);
void qLoadStringValue(int reg, char* value);
void qAdd(int reg1, int reg2);
void qSubtract(int reg1, int reg2);
void qMultiply(int reg1, int reg2);
void qDivide(int reg1, int reg2);
void qModulus(int reg1, int reg2);
void qAnd(int reg1, int reg2);
void qOr(int reg1, int reg2);
void qNegate(int reg1);
void qEquals(int reg1, int reg2);
void qNotEquals(int reg1, int reg2);
void qGreater(int reg1, int reg);
void qGreaterEquals(int reg1, int reg2);
void qLesser(int reg1, int reg2);
void qLesserEquals(int reg1, int reg2);
void qReturn(int reg);


#endif /* CODE_GENERATION_H */