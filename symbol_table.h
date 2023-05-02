enum RegType { type, globalVariable, localVariable, function };

struct Reg {
    char* regName;
    enum RegType type;
    struct Reg* typeReg;
    int lineOfDeclaration;
    struct Reg* next;
};

struct entry* search(char* id);
struct entry* searchRegType(char* id, enum RegType regType);
int newReg(char* regName, enum RegType type, struct Reg* typeReg, int line);
int closeBlock();
int free();
void dump(const char* message);