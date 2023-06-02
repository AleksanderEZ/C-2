#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

enum RegType { type, globalVariable, localVariable, function };

struct Reg {
    char* regName;
    enum RegType type;
    struct Reg* typeReg;
    int value;
    int lineOfDeclaration;
    struct Reg* next;
};

struct Reg* getTop();
struct Reg* search(char* regName);
struct Reg* searchRegType(char* id, enum RegType regType);
void newReg(char* regName, enum RegType type, struct Reg* typeReg, int line);
void closeBlock();
void clear();
void dump(const char* message);

#endif /* SYMBOL_TABLE_H */