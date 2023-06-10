#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

enum RegType { type, globalVariable, localVariable, function, parameter };

struct Reg {
    char* regName;
    enum RegType type;
    struct Reg* typeReg;
    int value;
    int nParameters;
    int lineOfDeclaration;
    int arraySize;
    struct Reg* next;
    struct Reg* previous;
};

struct Reg* getTop();
struct Reg* search(char* regName);
struct Reg* searchRegType(char* id, enum RegType regType);
void newReg(char* regName, enum RegType type, struct Reg* typeReg, int line);
void newParameter(char* regName, struct Reg* typeReg, int line);
void newFunction(char* regName, enum RegType type, struct Reg* typeReg, int line, int nParameters);
void removeLast();
void closeBlock();
void startClosingBlock();
void clear();
void dump(const char* message);
int checkValueExists(int value);

#endif /* SYMBOL_TABLE_H */