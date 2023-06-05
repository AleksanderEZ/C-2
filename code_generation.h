#ifndef CODE_GENERATION_H
#define CODE_GENERATION_H

#include "symbol_table.h"

enum StackOption {CONTINUE_STACK, ELSE_STACK, BREAK_STACK, SKIP_ELSE_STACK, ADVANCE_STACK, PARAMETER_STACK};

void push(int label, enum StackOption stackOption);
int pop(enum StackOption stackOption);
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
void qMain();
void qFunctionArguments(int reg);
int qCallFunction(char* functionName);
int qCallFunctionNoArgs(char* functionName);
void qStartPrint();
void qFinishPrint();
void qPrint(char* formatString, int reg);
void qPrintExplicit(char* expression);
void qPrintExplicitFormat(char* formatString, int reg);
void qPrintImplicitFormat(char* identifier, int reg);
void qStartWhile();
void qWhileCondition(int reg);
void qInstruction(char* instruction);
void qFinishWhile();
void qLoadVar(int reg, char* identifier);
void qStoreVar(int reg, char* identifier);
void qLoadGlobal(int reg, char* identifier);
void qStoreGlobal(int reg, char* identifier);
void qLoadLocal(int reg, char* identifier);
void qStoreLocal(int reg, char* identifier);
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
void qIf(int reg);
void qElse();
void qSkipElse();
void qSkipElseLabel();
int qFunctionDeclaration(int count, char** types, char** names);
void qFinishFunction();
int qSizeOf(char* typeName);
char qTypeMnemonic(char* typeName);

#endif /* CODE_GENERATION_H */