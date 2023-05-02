#include "symbol_table.h";
#include <stdlib.h>
#include <stdio.h>

struct Reg* top = NULL;

struct entry* search(char* regName) {
    struct Reg* p = top;
    while(p != NULL && strcmp(p->regName, regName) != 0) p = p->next;
    return p;
}

struct entry* searchRegType(char* regName, enum RegType regType) {
    struct Reg *p = search(regName); 
    if(p != NULL && p->type==regType) return p; else return NULL;
}

int newReg(char* regName, enum RegType type, struct Reg* typeReg, int line) {
    if(search(regName) != NULL) yyerror("Error: Name already defined");
    struct Reg* new = (struct Reg*) malloc(sizeof(struct Reg));
    new->regName = regName;
    new->type = type;
    new->typeReg = typeReg;
    new->lineOfDeclaration = line;
    new->next = top;
    top = new;
    return top;
}

int closeBlock() {
    while(top != NULL && top->type != function) {
        struct reg *p = top->next;
        free(top->regName); free(top); top=p;
    }
}

int free(const char* message) {
    printf("--INIT FREE -- %s", message);
    struct Reg* p = top;
    while(p != NULL){
        struct Reg* freePointer = p;
        p = p->next;
        free(freePointer->regName);
        free(freePointer);
    }
}

void dump(const char* message) {
    printf("--INIT DUMP -- %s", message);
    struct Reg* p = top;
    while(p != NULL){
        printf("0x%x Line: %d, Name: %s, Type: %s\n", (int)p, p->lineOfDeclaration, p->regName, p->typeReg==NULL?"·":p->typeReg->regName);
        p = p->next;
    }
}