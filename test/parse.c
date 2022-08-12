#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <stdbool.h>
#define strsame(A,B) (strcmp(A, B)==0)
#define MAX 1000
#define ERROR(PHRASE) { fprintf(stderr, \
          "Fatal Error %s occurred in %s, line %d\n", PHRASE, \
          __FILE__, __LINE__); \
          exit(EXIT_FAILURE); }

struct prog{
        char wds[MAX][MAX];
        int cw;
};
typedef struct prog Program;

bool Prog(Program* p);
bool Instrclist(Program* p);
bool Instrc(Program* p);
bool Print(Program* p);
bool Set(Program* p);
bool Create(Program* p);
bool Loop(Program* p);
bool Varname(Program* p);
bool String(Program* p);
bool Polishlist(Program* p);
bool Polish(Program* p);
bool Pushdown(Program* p);
bool Unaryop(Program* p);
bool Binaryop(Program* p);
bool Integer(Program* p);
bool Rows(Program* p);
bool Cols(Program* p);
bool Filename(Program* p);
void test();


int main(int argc, char* argv[])
{
    test();
    //read in file
    if (argc != 2)
    {
        fprintf(stderr, "Usage : %s <loop.nlb>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    FILE *fp = fopen(argv[1], "r");
    if (!fp){
            fprintf(stderr, "Cannot read the file %s\n", argv[1]);
            exit(EXIT_FAILURE);
    }

    //read in every singal word from the file, and check if the content conforms to the formal grammar or not.
    int i=0;
    Program* prog = calloc(1, sizeof(Program));
    while (fscanf(fp,"%s", prog->wds[i++])==1 && i<MAX);
    if (i>=MAX){
        printf("Overload string\n");
        free(prog);
        fclose(fp);
        return EXIT_SUCCESS;
    }
    Prog(prog);
    printf("Parsed OK\n");
    free(prog);
    fclose(fp);
    return EXIT_SUCCESS;
}


bool Prog(Program* p)
{
    if(p==NULL)
    {
        return false;
    }
    char *s = p->wds[p->cw];
    if (strsame(s,"#"))
    {
        for(int i=0; i<MAX; i++)
        {
            p->cw+=1;
            if (strsame(p->wds[p->cw], "BEGIN"))
            {
                i=MAX;
            }
        }
    }
    else if (!strsame(s, "BEGIN")){
        ERROR("No BEGIN statement ?");
    }
    //if there's no { after BEGIN statement, exit the program.
    p->cw +=1;
    if (!strsame(p->wds[p->cw],"{")){
        ERROR("Missing a { after BEGIN statement ?");
    }
    p->cw +=1;
    Instrclist(p);
    return true;
}


bool Instrclist(Program* p)
{
    if(p==NULL)
    {
        return false;
    }
    if (strsame(p->wds[p->cw], "}")){
        return true;
    }
    Instrc(p);
    p->cw +=1;
    Instrclist(p);
    return true;
}


bool Instrc(Program* p)
{
    if(p==NULL){
        return false;
    }
    char *s = p->wds[p->cw];

    if (strsame(s, "PRINT")){
        p->cw +=1;
        Print(p);
        return true;
    }
    else if(strsame(s, "SET")){
        p->cw +=1;
        Set(p);
        return true;
    }
    else if(strsame(s, "ONES") || strsame(s, "READ")){
        Create(p);
        return true;
    }
    else if(strsame(s, "LOOP")){
        p->cw +=1;
        Loop(p);
        return true;
    }
    fprintf(stderr, "%s is not a valid instruction name\n", p->wds[p->cw]);
    exit(EXIT_FAILURE);
}


bool Print(Program* p)
{
    if(p==NULL)
    {
        return false;
    }
    char c = p->wds[p->cw][0];
    if (c == '$'){
        Varname(p);
        return true;
    }
    else if (c == '"'){
        String(p);
        return true;
    }
    ERROR("No $ or \" in the statement?\n ");
}


//******************************************************SET INSTRUCTION ************************************************************************************************
bool Set(Program* p)
{
    if(p==NULL)
    {
        return false;
    }
    Varname(p);
    p->cw +=1;
    if(!strsame(p->wds[p->cw], ":=")){
        ERROR("No := statement ?");
    }
    p->cw +=1;
    Polishlist(p);
    return true;
}

bool Polishlist(Program* p)
{
    if(p==NULL)
    {
        return false;
    }
    //:= ; not allowed, must contain operands
    if (strsame(p->wds[p->cw], ";") && strsame(p->wds[p->cw-1], ":=")){
        ERROR("Missing operand after :=?");
    }
    else if(strsame(p->wds[p->cw], ";")){
        return true;
    }
    Polish(p);
    p->cw +=1;
    Polishlist(p);
    return true;
}

bool Polish(Program* p)
{
    if(p==NULL)
    {
        return false;
    }
    char c = p->wds[p->cw][0];
    //printf("%d\n", l);
    if (c == '$' || isdigit(c))
    {
        Pushdown(p);
        return true;
    }
    else if (c == 'U')
    {
        Unaryop(p);
        return true;
    }
    else if (c == 'B')
    {
        Binaryop(p);
        return true;
    }
    ERROR("Invalid Polish statement ?");
}


bool Pushdown(Program* p)
{
    if(p==NULL)
    {
        return false;
    }
    char c = p->wds[p->cw][0];
    if (c == '$')
    {
        Varname(p);
        return true;
    }
    else if (isdigit(c))
    {
        Integer(p);
        return true;
    }
    return true;
}

bool Unaryop(Program* p)
{
    if(p==NULL)
    {
        return false;
    }
    if (p->wds[p->cw][1] != '-'){
        ERROR("MISSING an '-' in the U- statement ? \n");
    }
    if (strsame(p->wds[p->cw], "U-NOT")){
        return true;
    }
    else if(strsame(p->wds[p->cw], "U-EIGHTCOUNT")){
        return true;
    }
    ERROR("Invalid U- statement? Valid Unaryop: U-NOT or U-EIGHTCOUNT\n");
}

bool Binaryop(Program* p)
{
    if(p==NULL)
    {
        return false;
    }
    char *s = p->wds[p->cw];
    //printf("%s\n", s);
    if (s[1] != '-'){
        ERROR("MISSING an '-' in the B- statement or a $ before variable name B ? \n");
    }

    if (strsame(s, "B-AND") || strsame(s, "B-OR")){
        return true;
    }
    else if (strsame(s, "B-LESS") || strsame(s, "B-GREATER")){
        return true;
    }
    else if (strsame(s, "B-ADD") || strsame(s, "B-TIMES") || strsame(s, "B-EQUALS")){
        return true;
    }
    ERROR("Invalid B- statement ? \n");
}

//<CREATE> ::= "ONES" <ROWS> <COLS> <VARNAME> | "READ" <FILENAME> <VARNAME>
bool Create(Program* p)
{
    if(p==NULL)
    {
        return false;
    }
    char *s = p->wds[p->cw];
    if (strsame(s, "ONES"))
    {
        p->cw +=1;
        Rows(p);
        p->cw +=1;
        Cols(p);
        p->cw +=1;
        Varname(p);
    }
    else if (strsame(s, "READ"))
    {
        p->cw +=1;
        Filename(p);
        p->cw +=1;
        Varname(p);
    }
    return true;
}


bool Rows(Program* p)
{
    if(p==NULL)
    {
        return false;
    }
    Integer(p);
    return true;
}

bool Cols(Program* p)
{
    if(p==NULL)
    {
        return false;
    }
    Integer(p);
    return true;
}

bool Filename(Program* p)
{
    if(p==NULL)
    {
        return false;
    }
    String(p);
    return true;
}

//<LOOP> ::= "LOOP" <VARNAME> <INTEGER> "{" <INSTRCLIST>
bool Loop(Program* p)
{
    if(p==NULL)
    {
        return false;
    }
    Varname(p);
    p->cw +=1;
    if(isdigit(p->wds[p->cw][0])){
        Integer(p);
    }
    else{
        ERROR("One integer is needed for LOOP");
    }
    p->cw +=1;
    if (!strsame(p->wds[p->cw], "{")){
        ERROR("No { statement ?");
    }
    p->cw +=1;
    Instrclist(p);
    return true;
}




bool Integer(Program* p)
{
    if(p==NULL)
    {
        return false;
    }
    int i, l=strlen(p->wds[p->cw]);
    char *s = p->wds[p->cw];
    for (i=0; i<l; i++)
    {
        if (!isdigit(s[i]))
        {
            fprintf(stderr, "%s is not an integer, pls check the gramma\n", s);
            exit(EXIT_FAILURE);
        }
    }
    return true;
}



bool Varname(Program* p)
{
    if(p==NULL)
    {
        return false;
    }
    int l=strlen(p->wds[p->cw]);
    char c0= p->wds[p->cw][0];
    if (c0 != '$')
    {
        ERROR("Missing a Variable name? $A-Z is wanted here.\n");
    }
    char c1 = p->wds[p->cw][1];
    if (isupper(c1) && (l==2))
    {
        return true;
    }
    ERROR("Invalid Variable name? A single uppercase letter A-Z is expected here\n");
}

bool String(Program* p)
{
    if(p==NULL)
    {
        return false;
    }
    int l=strlen(p->wds[p->cw]);
    if (p->wds[p->cw][l-1] == '"'){
        return true;
    }
    fprintf(stderr, "%s NO Double-quote at the end of the String statement ?\n", p->wds[p->cw]);
    exit(EXIT_FAILURE);
}



void test()
{

    assert(!Prog(NULL));
    assert(!Instrclist(NULL));
    assert(!Instrc(NULL));
    assert(!Print(NULL));
    assert(!Varname(NULL));
    assert(!String(NULL));
    assert(!Set(NULL));
    assert(!Polishlist(NULL));
    assert(!Polish(NULL));
    assert(!Pushdown(NULL));
    assert(!Integer(NULL));
    assert(!Unaryop(NULL));
    assert(!Binaryop(NULL));
    assert(!Create(NULL));
    assert(!Rows(NULL));
    assert(!Cols(NULL));
    assert(!Filename(NULL));
    assert(!Loop(NULL));
}

