#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <stdbool.h>
#include "general.c"
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
//save data geberated by the instructions in this Data structure;
struct data{
    int arr[MAX][MAX];
    int row;
    int col;
    int p_index; //parent variable's index
    bool is_set;
};
typedef struct data Data;


void Prog(Program* p, Data* d);
void Instrclist(Program* p, Data* d);
void Instrc(Program* p, Data* d);
void Print(Program* p, Data* d);
void Set(Program* p, Data* d);
void Create(Program* p, Data* d);
void Loop(Program* p, Data* d);
void Polishlist(Program* p, Data* d);
void Polish(Program* p, Data* d);
void Pushdown(Program* p, Data* d);
void Unaryop(Program* p, Data* d);
void Binaryop(Program* p, Data* data);
int Varname(Program* p, Data* d);
int Integer(Program* p);
int Rows(Program* p);
int Cols(Program* p);
char* String(Program* p);

bool has_2operands(Program* p);
bool has_validop(Program* p, Data* d);
bool is_int(Program* p);
bool is_var(Program* p);
bool is_set(Program* p,Data* d, int index);
bool push_var(Program* p, Data* d);

bool READ(Program* p, Data* d);
bool ONES(Program* p, Data* d);
bool BOP(Program* p,Data* d); //Binary operations
bool UOP(Program* p, Data* d);//Unary operations
bool ADD(Data* d, int i, int j, int num, int p_id, int c_row, int c_col);
bool TIMES(Data* d, int i, int j, int num, int p_id, int c_row, int c_col);
bool EQUALS(Data* d, int i, int j, int num, int p_id, int c_row, int c_col);
bool AND(Data* d, int i, int j, int num, int p_id, int c_row, int c_col);
bool OR(Data* d, int i, int j, int num, int p_id, int c_row, int c_col);
bool GREATER(Data* d, int i, int j, int num, int p_id, int c_row, int c_col);
bool LESS(Data* d, int i, int j, int num, int p_id, int c_row, int c_col);
bool NOT(Data* d, int p_id, int p_col, int p_row);
bool EIGHTCOUNT(Data* d, int p_id, int p_col, int p_row);
void test();

int main(int argc, char* argv[])
{
    test();
    //read in file
    if (argc != 2)
    {
        fprintf(stderr, "Usage : %s <loopa.nlb>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    FILE *fp = fopen(argv[1], "r");
    if (!fp){
            fprintf(stderr, "Cannot read the file %s\n", argv[1]);
            exit(EXIT_FAILURE);
    }
    //read in every singal word from the file, and check if the content conforms to the formal grammar or not.
    int i=0;
    Program* prog = (Program*) ncalloc(sizeof(Program),1);
    Data* d = (Data*) ncalloc(sizeof(Data), 27);
    while (fscanf(fp,"%s", prog->wds[i++])==1 && i<MAX);
    if (i>=MAX){
        printf("Overload string\n");
        free(prog);
        free(d);
        fclose(fp);
        return EXIT_SUCCESS;
    }
    Prog(prog,d);
    free(prog);
    free(d);
    fclose(fp);
    return EXIT_SUCCESS;
}


void Prog(Program* p, Data* d)
{
    if(p==NULL)
    {
        ERROR("File is empty?");
    }
    char *s = p->wds[p->cw];
    if (s[0]=='#')
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
        ERROR("Missing  { after BEGIN statement ?");
    }
    p->cw +=1;
    Instrclist(p,d);
}


void Instrclist(Program* p, Data* d)
{
    if (strsame(p->wds[p->cw], "}")){
        return;
    }
    Instrc(p,d);
    p->cw +=1;
    Instrclist(p,d);
}


void Instrc(Program* p, Data* d)
{
    char *s = p->wds[p->cw];

    if (strsame(s, "PRINT")){
        p->cw +=1;
        Print(p,d);
        return;
    }
    else if(strsame(s, "SET")){
        p->cw +=1;
        Set(p,d);
        return;
    }
    else if(strsame(s, "ONES") || strsame(s, "READ")){
        Create(p,d);
        return;
    }
    else if(strsame(s, "LOOP")){
        p->cw +=1;
        Loop(p,d);
        return;
    }
    fprintf(stderr, "%s is not a valid instruction name\n", p->wds[p->cw]);
    exit(EXIT_FAILURE);
}


void Set(Program* p,Data* d)
{
    int index = Varname(p,d);
    (d+26)->p_index = index; //save parent variable's index into 27th tmp structure
    (d+index)->is_set = true; //mark the variable as being set
    p->cw +=1;
    if(!strsame(p->wds[p->cw], ":=")){
        ERROR("No := statement ?");
    }
    p->cw +=1;
    Polishlist(p,d);
}



void Polishlist(Program* p, Data* d)
{
    char* s = p->wds[p->cw];
    //:= ; not allowed, must contain operands
    if (strsame(s, ";") && strsame(p->wds[p->cw-1], ":="))
    {
        ERROR("No operand for SET ?");
    }
    else if(strsame(s, ";")){
        return;
    }
    Polish(p,d);
    p->cw +=1;
    Polishlist(p,d);
}


void Polish(Program* p, Data* d)
{
    char* s = p->wds[p->cw];
    if (s[0] == '$' || isdigit(s[0]))
    {
        Pushdown(p,d);
        return;
    }
    else if (s[0] == 'U')
    {
        Unaryop(p,d);
        return;
    }
    else if (s[0] == 'B')
    {
        Binaryop(p,d);
        return;
    }
    fprintf(stderr, "%s is an invalid polish statement\n Valid statements: $[A-Z]| U- | B-.\n", s);
    exit(EXIT_FAILURE);

}


void Pushdown(Program* p,Data* d)
{
    char* s = p->wds[p->cw];
    int p_index = (d+26)->p_index;
    if (push_var(p,d)){
        return;
    }
    if (isdigit(s[0]) && strsame(p->wds[p->cw-1],":="))
    {
        int num=Integer(p);
        (d+p_index)->row=1;
        (d+p_index)->col=1;
        (d+p_index)->arr[1][1]=num;
        return;
    }
    else if (isdigit(s[0]))
    {
        int num=Integer(p);
        (d+26)->row = 1;  //save child's row and col info into the 27th tmp strcuture;
        (d+26)->col = 1;
        (d+26)->arr[1][1]=num;
        return;
    }
}


void Unaryop(Program* p, Data* d)
{
    if (p->wds[p->cw][1] != '-'){
        ERROR("MISSING an '-' in the U- statement ? \n");
    }
    if (strsame(p->wds[p->cw], "U-NOT") || strsame(p->wds[p->cw], "U-EIGHTCOUNT")){
        if (has_2operands(p))
        {
            assert(UOP(p,d));
        }
        return;
    }
    ERROR("Invalid Unary statement? Valid Unaryop: U-NOT or U-EIGHTCOUNT\n");
}


void Binaryop(Program* p, Data* d)
{
    char *s = p->wds[p->cw];
    if (s[1] != '-'){
        ERROR("MISSING an '-' in the B- statement or a $ before variable name B ? \n");
    }

    if (strsame(s, "B-LESS") || strsame(s, "B-GREATER") || strsame(s, "B-AND") || strsame(s, "B-OR"))
    {
        if (has_2operands(p))
        {
            assert(BOP(p,d));
        }
        return;
    }
    else if (strsame(s, "B-ADD") || strsame(s, "B-TIMES") || strsame(s, "B-EQUALS"))
    {
        if (has_2operands(p))
        {
            assert(BOP(p,d));
        }
        return;
    }
    fprintf(stderr, "%s is not a valid Binaryop statement. \nValid Binaryop: B-ADD, B-TIMES, B-EQUALS, B-LESS, B-GREATER, B-AND, B-OR\n", p->wds[p->cw]);
    exit(EXIT_FAILURE);
}


void Create(Program* p, Data* d)
{
    char *s = p->wds[p->cw];
    if (strsame(s, "ONES")){
        assert(ONES(p,d));
        return;
    }
    else if (strsame(s, "READ"))
    {
        if (READ(p,d))
        {
            return;
        }
    }
    fprintf(stderr, "%s is not a valid Create statement. \nValid statement: ONES, READ.\n", p->wds[p->cw]);
    exit(EXIT_FAILURE);
}


void Loop(Program* p, Data* d)
{
    int index=Varname(p,d);
    (d+index)->is_set = true;
    int cnt;
    p->cw +=1;
    if(isdigit(p->wds[p->cw][0]))
    {
        cnt=Integer(p);
    }
    else{
        ERROR("One integer is required for LOOP \n");
    }
    (d+index)->arr[1][1]=1;
    (d+index)->row=1;
    (d+index)->col=1;
    p->cw +=1;
    if (!strsame(p->wds[p->cw], "{")){
        ERROR("No { statement ?");
    }
    p->cw +=1;
    int tep=p->cw;
    while((d+index)->arr[1][1]<=cnt)
    {
        p->cw=tep;
        Instrclist(p,d);
        (d+index)->arr[1][1]+=1;
    }
}


int Integer(Program* p)
{
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
    int num=atoi(s);
    return num;
}


int Varname(Program* p, Data* d)
{
    int l=strlen(p->wds[p->cw]);
    char* s= p->wds[p->cw];
    if (s[0] != '$')
    {
        ERROR("Missing a Variable name? $A-Z is wanted here.\n");
    }
    else if (isupper(s[0]) && (l==1))
    {
        fprintf(stderr, "%s is an invalid operand.\n", s);
        exit(EXIT_FAILURE);
    }

    if (isupper(s[1]) && (l==2))
    {
        int index=s[1]-65;
        if (is_set(p,d,index)){
            return index;
        }
        return index;
    }
    fprintf(stderr, "%s is an invalid Variable name. A single uppercase letter A-Z is expected here \n", s);
    exit(EXIT_FAILURE);
}


char* String(Program* p)
{
    char* s = p->wds[p->cw];
    int l = strlen(s);
    char* tmp= (char*) ncalloc(sizeof(char), l);
    if (s[l-1] == '"'){
        for (int i=1; i<l-1; i++)
        {
            tmp[i-1]=s[i];
        }
        return tmp;
    }
    fprintf(stderr, "%s NO Double-quote at the end of the String statement ?\n", p->wds[p->cw]);
    exit(EXIT_FAILURE);
}


int Rows(Program* p)
{
    int row=Integer(p);
    return row;
}

int Cols(Program* p)
{
    int cols=Integer(p);
    return cols;
}


void Print(Program* p,Data* d)
{
    char c = p->wds[p->cw][0];
    if (is_var(p)){
        int index = Varname(p,d);
        for (int i=1; i<=(d+index)->row; i++)
        {
            for (int j=1; j<=(d+index)->col; j++)
            {
                printf("%d ", (d+index)->arr[i][j]);
            }
            printf("\n");
        }
        return;
    }
    else if (c == '"'){
        char* s =String(p);
        printf("%s\n",s);
        free(s);
        return;
    }
    ERROR("No $ or \" in the statement?\n ");
}


bool ONES(Program* p, Data* d)
{
    if(d==NULL || p==NULL)
    {
        return false;
    }
    p->cw += 1;
    int row = Rows(p);
    p->cw += 1;
    int col = Cols(p);
    p->cw +=1;
    int index = Varname(p,d);
    (d+index)->is_set = true;
    (d+index)->row = row;
    (d+index)->col = col;
    for (int i=1; i<=row; i++)
    {
        for (int j=1; j<=col; j++)
        {
            (d+index)->arr[i][j] = 1;
        }
    }
    return true;
}


bool READ(Program* p, Data* d)
{
    if(d==NULL || p==NULL)
    {
        return false;
    }
    int row=0,col=0,m=0;
    char str[MAX];
    char wd[MAX][MAX];
    p->cw +=1;
    char* fname = String(p);
    FILE *fp = fopen(fname, "r");
    if (!fp)
    {
        fprintf(stderr, "Read Cannot read file %s\n", fname);
        exit(EXIT_FAILURE);
    }
    p->cw +=1;
    int index=Varname(p,d);
    (d+index)->is_set = true; //mark the variable as being set
    if (fgets(str,10,fp)){
        assert(sscanf(str,"%d %d",&row,&col)==2);
    }
    (d+index)->row = row;
    (d+index)->col = col;
    int n=0;
    while (fscanf(fp,"%s",wd[n++])==1);
    for(int i=1;i<=row;i++)
    {
        for(int j=1;j<=col;j++)
        {
            (d+index)->arr[i][j]=atoi(wd[m]);
            m++;
        }
    }
    free(fname);
    fclose(fp);
    return true;
}



bool BOP(Program* p, Data* d)
{
    if (d==NULL || p==NULL)
    {
        return false;
    }
    int p_id = (d+26)->p_index;
    int p_col = (d+p_id)->col;
    int p_row = (d+p_id)->row;
    int c_col = (d+26)->col;
    int c_row = (d+26)->row;
    int num = (d+26)->arr[1][1];
    char *s = p->wds[p->cw];

    if (has_validop(p,d)){
        for (int i=1; i<=p_row; i++){
            for (int j=1; j<=p_col; j++)
            {
                if (strsame(s,"B-ADD")){
                    assert(ADD(d,i,j,num,p_id,c_row,c_col));
                }
                if (strsame(s,"B-TIMES")){
                   assert(TIMES(d,i,j,num,p_id,c_row,c_col));
                }
                if (strsame(s,"B-EQUALS")){
                    assert(EQUALS(d,i,j,num,p_id,c_row,c_col));
                }
                if (strsame(s,"B-AND")){
                    assert(AND(d,i,j,num,p_id,c_row,c_col));
                }
                if (strsame(s,"B-OR")){
                    assert(OR(d,i,j,num,p_id,c_row,c_col));
                }
                if (strsame(s,"B-GREATER")){
                    assert(GREATER(d,i,j,num,p_id,c_row,c_col));
                }
                if (strsame(s,"LESS")){
                    assert(LESS(d,i,j,num,p_id,c_row,c_col));
                }
            }
        }
    }
    return true;
}

bool UOP(Program* p, Data* d)
{
    if (d==NULL || p==NULL)
    {
        return false;
    }
    int p_id = (d+26)->p_index;
    int p_col = (d+p_id)->col;
    int p_row = (d+p_id)->row;
    char *s = p->wds[p->cw];
    if (strsame(s, "U-NOT"))
    {
        assert(NOT(d,p_id,p_col,p_row));
        return true;
    }
    if (strsame(s, "U-EIGHTCOUNT"))
    {
        assert(EIGHTCOUNT(d,p_id,p_col,p_row));
        return true;
    }
    return true;
}


bool ADD(Data* d, int i, int j, int num, int p_id, int c_row, int c_col)
{
    if (d==NULL)
    {
        return false;
    }
    if (c_col==1 && c_row==1){
        (d+p_id)->arr[i][j]+=num;
    }
    else{
        (d+p_id)->arr[i][j]+=(d+26)->arr[i][j];
    }
    return true;
}


bool TIMES(Data* d, int i, int j, int num, int p_id, int c_row, int c_col)
{
    if (d==NULL)
    {
        return false;
    }
    if (c_col==1 && c_row==1){
        (d+p_id)->arr[i][j]*=num;
    }
    else{
        (d+p_id)->arr[i][j]*=(d+26)->arr[i][j];
    }
    return true;
}


bool EQUALS(Data* d, int i, int j, int num, int p_id, int c_row, int c_col)
{
    if (d==NULL)
    {
        return false;
    }
    if (c_col==1 && c_row==1){
        if ((d+p_id)->arr[i][j]==num){
            (d+p_id)->arr[i][j]=1;
        }
        else{
            (d+p_id)->arr[i][j]=0;
        }
    }
    else{
        if((d+p_id)->arr[i][j]==(d+26)->arr[i][j]){
            (d+p_id)->arr[i][j]=1;
        }
        else{
            (d+p_id)->arr[i][j]=0;
        }
    }
    return true;
}


bool AND(Data* d, int i, int j, int num, int p_id, int c_row, int c_col)
{
    if (d==NULL)
    {
        return false;
    }
    if (c_col==1 && c_row==1){
        if ((d+p_id)->arr[i][j]&&num){
            (d+p_id)->arr[i][j]=1;
        }
        else{
            (d+p_id)->arr[i][j]=0;
        }
    }
    else{
        if((d+p_id)->arr[i][j]&&(d+26)->arr[i][j]){
            (d+p_id)->arr[i][j]=1;
        }
        else{
            (d+p_id)->arr[i][j]=0;
        }
    }
    return true;
}



bool OR(Data* d, int i, int j, int num, int p_id, int c_row, int c_col)
{
    if(d==NULL)
    {
        return false;
    }
    if (c_col==1 && c_row==1){
        if ((d+p_id)->arr[i][j]||num){
            (d+p_id)->arr[i][j]=1;
        }
        else{
            (d+p_id)->arr[i][j]=0;
        }
    }
    else{
        if((d+p_id)->arr[i][j]||(d+26)->arr[i][j]){
            (d+p_id)->arr[i][j]=1;
        }
        else{
            (d+p_id)->arr[i][j]=0;
        }
    }
    return true;
}


bool GREATER(Data* d, int i, int j, int num, int p_id, int c_row, int c_col)
{
    if (d==NULL)
    {
        return false;
    }
    if (c_col==1 && c_row==1){
        if ((d+p_id)->arr[i][j]>num){
            (d+p_id)->arr[i][j]=1;
        }
        else{
            (d+p_id)->arr[i][j]=0;
        }
    }
    else{
        if ((d+p_id)->arr[i][j]>(d+26)->arr[i][j]){
            (d+p_id)->arr[i][j]=1;
        }
        else{
            (d+p_id)->arr[i][j]=0;
        }
    }
    return true;
}


bool LESS(Data* d, int i, int j, int num, int p_id, int c_row, int c_col)
{
    if (d==NULL)
    {
        return false;
    }
    if (c_col==1 && c_row==1){
        if ((d+p_id)->arr[i][j]<num){
            (d+p_id)->arr[i][j]=1;
        }
        else{
            (d+p_id)->arr[i][j]=0;
        }
    }
    else{
        if((d+p_id)->arr[i][j]<(d+26)->arr[i][j]){
            (d+p_id)->arr[i][j]=1;
        }
        else{
            (d+p_id)->arr[i][j]=0;
        }
    }
    return true;
}


bool NOT(Data* d, int p_id, int p_col, int p_row)
{
    if (d==NULL)
    {
        return false;
    }
    for (int i=1; i<=p_row; i++)
    {
        for (int j=1; j<=p_col; j++)
        {
            if ((d+p_id)->arr[i][j] == 0)
            {
                (d+p_id)->arr[i][j] = 1;
            }
            else
            {
                    (d+p_id)->arr[i][j] = 0;
            }
        }
    }

    return true;
}

bool EIGHTCOUNT(Data* d, int p_id, int p_col, int p_row)
{
    if (d==NULL)
    {
        return false;
    }
    int tmparr[MAX][MAX]={0};
    for (int i=1; i<=p_row; i++)
    {
        for (int j=1; j<=p_col; j++)
        {
            tmparr[i][j]=(d+p_id)->arr[i][j];
        }
    }
    //Moore 8-neighbourhood (north, south, west, east, NE, NW, SW, SE).
    for (int i=1; i<=p_row; i++)
    {
        for (int j=1; j<=p_col; j++)
        {
            //count the total amount of true values for 8-neighbourhood around each cell
            int cnt=0;
            for (int z=-1; z<2; z++)
            {
                if (tmparr[i+z][j-1]!=0){
                    cnt++;
                }
                if (tmparr[i+z][j]!=0){
                    if (z!=0){
                        cnt++;
                    }
                }
                if (tmparr[i+z][j+1]!=0){
                    cnt++;
                }
            }
            (d+p_id)->arr[i][j]=cnt;
        }
    }
    return true;
}


bool push_var(Program* p, Data* d){
    if(d==NULL || p==NULL)
    {
        return false;
    }
    char* s = p->wds[p->cw];
    int p_index = (d+26)->p_index;

    if ((s[0] == '$') && strsame(p->wds[p->cw-1],":="))
    {
        int c_index =Varname(p,d); //child index

        (d+p_index)->row = (d+c_index)->row;
        (d+p_index)->col = (d+c_index)->col;

        for (int i=1; i<=(d+c_index)->row; i++)   //save child's arr[][] info into parent's structure;
        {
            for (int j=1; j<=(d+c_index)->col; j++)
            {
                (d+p_index)->arr[i][j]=(d+c_index)->arr[i][j];
            }
        }
        return true;
    }
    else if (s[0] == '$')
    {
        int c_index = Varname(p,d); //child index
        (d+26)->row = (d+c_index)->row;  //save child's row and col info into the 27th tmp strcuture;
        (d+26)->col = (d+c_index)->col;
        for (int i=1; i<=(d+c_index)->row; i++)   //save child's arr[][] info into 27th tmp structure;
        {
            for (int j=1; j<=(d+c_index)->col; j++)
            {
                (d+26)->arr[i][j]=(d+c_index)->arr[i][j];
            }
        }
        return true;
    }
    return false;
}


bool has_validop(Program* p,Data* d) //compare the size of 2 arrays
{
    //If both arrays are bigger than 1x1, they must be the same size
    // If one array is a 1x1 scalar, apply this value to each cell of the other array in turn
    if(d==NULL || p==NULL)
    {
        return false;
    }
    int p_index = (d+26)->p_index;
    int p_col = (d+p_index)->col;
    int p_row = (d+p_index)->row;
    int c_col = (d+26)->col;
    int c_row = (d+26)->row;
    char* s = p->wds[p->cw];
    //If both arrays are bigger than 1x1, they must be the same size
    if ((p_col==1) && (p_row==1))
    {
        return true;
    }
    else if ((p_col>1) || (p_row>1))
    {
        if ((c_col==1) || (c_row==1))
        {
            return true;
        }
        else if ((p_col==c_col) && (p_row==c_row))
        {
            return true;
        }
        fprintf(stderr, "Both arrays are bigger than 1x1 for %s operation, they must be the same size.\n", s);
        exit(EXIT_FAILURE);
    }
    return false;
}


bool has_2operands(Program* p) //check if user typein 2 operands for first Binaryop, and 1 for the following op;
{
    if (p==NULL)
    {
        return false;
    }
    char* s = p->wds[p->cw-2];
    char* s1 = p->wds[p->cw];
    if (strsame(s1,"U-NOT") || strsame(s1,"U-EIGHTCOUNT"))
    {
        p->cw -=1;
        if (!is_var(p) && !is_int(p))
        {
            fprintf(stderr, "1 valid operand is missing before %s.\n", p->wds[p->cw+1]);
            exit(EXIT_FAILURE);
        }
        p->cw -=1;
        if (is_var(p) || is_int(p))
        {
            fprintf(stderr, "ONLY 1 operand is required for %s.\n", p->wds[p->cw+2]);
            exit(EXIT_FAILURE);
        }
        p->cw +=2;
    }
    else if (strsame(s,":="))
    {
        fprintf(stderr, "2 valid operands are required for %s, 1 operand is missing.\n", p->wds[p->cw]);
        exit(EXIT_FAILURE);
    }
    else{
        p->cw -=1;
        if (!is_var(p) && !is_int(p))
        {
            fprintf(stderr, "1 valid operand is missing before %s.\n", p->wds[p->cw+1]);
            exit(EXIT_FAILURE);
        }
        p->cw +=1;
    }
    return true;
}

bool is_var(Program* p)
{
    if (p==NULL)
    {
        return false;
    }
    int l=strlen(p->wds[p->cw]);
    char* s= p->wds[p->cw];
    if (s[0] != '$')
    {
        return false;
    }

    if (isupper(s[1]) && (l==2))
    {
        return true;
    }
    return false;
}

bool is_int(Program* p)
{
    if (p==NULL)
    {
        return false;
    }
    int i, l=strlen(p->wds[p->cw]);
    char *s = p->wds[p->cw];
    for (i=0; i<l; i++)
    {
        if (!isdigit(s[i]))
        {
            return false;
        }
    }
    return true;
}


bool is_set(Program* p,Data* d, int index)
{
    if (p==NULL || d==NULL)
    {
        return false;
    }
    char* s= p->wds[p->cw];     //current word
    char* s0= p->wds[p->cw-1];  //one place before current word
    char* s1 = p->wds[p->cw-2]; //two places before current word
    char* s2 = p->wds[p->cw-3]; //three places before current word
    if ((!strsame(s0,"SET")) && (!strsame(s0,"LOOP")) && (!strsame(s1,"READ")) && (!strsame(s2,"ONES")))
    {
        if ((d+index)->is_set != true)
        {
            fprintf(stderr, "%s is an undefined variable, pls set the variable before using it.\n", s);
            exit(EXIT_FAILURE);
        }
    }
    return true;
}



void test()
{
    assert(!has_2operands(NULL));
    assert(!has_validop(NULL, NULL)); //ADD = arr + int or int + int
    assert(!is_int(NULL));
    assert(!is_var(NULL));
    assert(!is_set(NULL,NULL,1));
    assert(!BOP(NULL,NULL));         //Binary operations
    assert(!UOP(NULL, NULL));        //Unary operations
    assert(!ADD(NULL, 1, 2, 3, 4, 5, 6));
    assert(!TIMES(NULL, 1, 2, 3, 4, 5, 6));
    assert(!EQUALS(NULL, 1, 2, 3, 4, 5, 6));
    assert(!AND(NULL, 1, 2, 3, 4, 5, 6));
    assert(!OR(NULL, 1, 2, 3, 4, 5, 6));
    assert(!GREATER(NULL, 1, 2, 3, 4, 5, 6));
    assert(!LESS(NULL, 1, 2, 3, 4, 5, 6));
    assert(!NOT(NULL, 1, 2, 3));
    assert(!EIGHTCOUNT(NULL, 1, 2, 3));
    assert(!push_var(NULL,NULL));
    assert(!READ(NULL, NULL));
    assert(!ONES(NULL,NULL));
}


