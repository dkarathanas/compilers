#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef CODE_H
#define CODE_H
#include "i_code.h"
#endif

#define KNRM "\x1B[0m"
#define KYEL "\x1B[33m"
#define KRED "\x1B[31m"
#define KMAG "\x1B[35m"
#define KGRN "\x1B[32m"
#define KBLU "\x1B[34m"
#define KCYN "\x1B[36m"
#define KWHT "\x1B[37m"
/*
------------------------------------
DEFINITIONS
------------------------------------
*/
quad *quads = (quad *)0;
unsigned total = 0;
unsigned int currQuad = 0;
unsigned int currprocquad = 0;

unsigned programVarOffset = 0;
unsigned functionLocalOffset = 0;
unsigned formalArgOffset = 0;
unsigned scopeSpaceCounter = 1;

extern int loopcounter;
extern bc_stack* bc_head;
extern func_stack *func_head;
extern loop_stack *loop_head;
extern unsigned int is_func;

extern void   generate_assign(quad*q);
extern void   generate_add(quad* q);
extern void   generate_sub(quad* q);
extern void   generate_mul(quad* q);
extern void   generate_div(quad* q);
extern void   generate_mod(quad* q);
extern void   generate_and(quad* q);
extern void   generate_or(quad* q);
extern void   generate_not(quad* q);
extern void   generate_eq(quad* q);
extern void   generate_noteq(quad* q);
extern void   generate_lesseq(quad* q);
extern void   generate_greatereq(quad* q);
extern void   generate_less(quad* q);
extern void   generate_greater(quad* q);
extern void   generate_call(quad* q);
extern void   generate_newtable(quad* q);
extern void   generate_tablegetelem(quad* q);
extern void   generate_tablesetelem(quad* q);
extern void   generate_nop(quad* q);
extern void   generate_jump(quad* q);
extern void   generate_param(quad* q);
extern void   generate_getretval(quad* q);
extern void   generate_funcstart(quad* q);
extern void   generate_funcend(quad* q);
extern void   generate_return(quad* q);
// extern void   generate_uminus(quad* q);

    int tempcounter = 0;

typedef void (*generator_func_t)(quad *);

generator_func_t generators[] = {
    generate_assign,
    generate_add,
    generate_sub,
    generate_mul,
    generate_div,
    generate_mod,
    generate_or,
    generate_jump,
    generate_and,
    generate_not,
    generate_eq,
    generate_noteq,
    generate_lesseq,
    generate_greatereq,
    generate_less,
    generate_greater,
    generate_call,
    generate_param,
    generate_getretval,
    generate_funcstart,
    generate_funcend,
    generate_newtable,
    generate_tablegetelem,
    generate_tablesetelem,
    generate_return
    // generate_uminus
};

void generate_instructions()
{
    unsigned i;
    for (i = 0; i < currQuad; i++)
    {
        (generators[quads[i].op](quads + i));
        currprocquad++;
    }
}

void expand()
{
    assert(total == currQuad);
    quad *p = (quad *)malloc(NEW_SIZE);
    if (quads)
    {
        memcpy(p, quads, CURR_SIZE);
        free(quads);
    }
    quads = p;
    total += EXPAND_SIZE;
}

void emit(iopcode op, expr *arg1, expr *arg2, expr *result, unsigned label, unsigned line)
{
    if (currQuad == total)
    {
        expand();
    }
    quad *p = quads + currQuad++;
    p->op = op;
    p->arg1 = arg1;
    p->arg2 = arg2;
    p->result = result;
    p->label = label;
    p->line = line;
}

scopespace_t currscopespace(void)
{
    if (scopeSpaceCounter == 1)
    {
        return programvar;
    }
    else
    {
        if (scopeSpaceCounter % 2 == 0)
        {
            return formalarg;
        }
        else
            return funclocal;
    }
}

unsigned currscopeoffset(void)
{
    switch (currscopespace())
    {
    case programvar:
    printf(KMAG"programvar offset = %d\n", programVarOffset);
        return programVarOffset;
    case funclocal:
        printf(KMAG "funclocal offset = %d\n", functionLocalOffset);
        return functionLocalOffset;
    case formalarg:
        printf(KMAG "formalarg offset = %d\n", formalArgOffset);

        return formalArgOffset;
    default:
        assert(0);
    }
}

void inccurrentscopeoffset(void)
{
    switch (currscopespace())
    {
    case programvar:
        ++programVarOffset;
        break;
    case funclocal:
        ++functionLocalOffset;
        break;
    case formalarg:
        ++formalArgOffset;
        break;
    default:
        assert(0);
    }
}

void enterscopespace(void)
{
    ++scopeSpaceCounter;
}

void exitscopespace()
{
    assert(scopeSpaceCounter > 1);
    --scopeSpaceCounter;
}

void resetformalargsoffet(void)
{
    formalArgOffset = 0;
}

void resetfunclocaloffset(void)
{
    functionLocalOffset = 0;
}

void restorecurrscopeoffset(unsigned n)
{
    switch (currscopespace())
    {
    case programvar:
        programVarOffset = n;
        break;
    case funclocal:
        functionLocalOffset = n;
        break;
    case formalarg:
        formalArgOffset = n;
        break;
    default:
        assert(0);
    }
}

unsigned nextquadlabel(void)
{
    return currQuad;
}

void patchlabel(unsigned quadNo, unsigned label)
{
    assert(quadNo < currQuad);
    printf("PATCH LABEL: quadNo: %d, LABEL: %d\n", quadNo, label);
    quads[quadNo].label = label;
}

expr *lvalue_expr(struct SymbolTableEntry *sym)
{
    assert(sym);
    expr *e = (expr *)malloc(sizeof(expr));
    memset(e, 0, sizeof(expr));
    e->next = (expr *)0;
    e->sym = sym;

    switch (sym->s_type)
    {
    case var_s:
        e->type = var_e;
        break;
    case programfunc_s:
        e->type = programfunc_e;
        break;
    case libfunc_s:
        e->type = libfunc_e;
        break;
    default:
        assert(0);
    }
    return e;
}

expr *newexpr(expr_t t){
    expr* e = (expr*)malloc(sizeof(expr));
    memset(e, 0, sizeof(expr));
    e->type = t;
    return e;
}

expr *newexpr_conststring(char *s){
    expr* e = newexpr(conststring_e);
    e->strConst = strdup(s);
    return e;
}

expr *newexpr_constnum(double d){
    expr* e = newexpr(constnum_e);
    e->numConst = d;
    return e;
}

expr *newexpr_constbool(char c){
    expr* e = newexpr(constbool_e);
    e->boolConst = c;
    return e;
}

//TODO: create functions for all expr types

expr *emit_iftableitem(expr *e, unsigned label, unsigned line, unsigned int scope){
    if(e->type != tableitem_e){
        // e->type = programfunc_e;
        return e;
    }
    else{
        expr* result = newexpr(var_e);
        result->sym = newtemp(scope, line);
        emit(tablegetelem, e, e->index, result, label, line);
        printf("e->type = %d",e->type);
        // resettemp();
        return result;
    }
}

expr* member_item(expr* lvalue, char* name, unsigned label, unsigned line, unsigned int scope){
    lvalue = emit_iftableitem(lvalue, label, line, scope);
    expr* item = newexpr(tableitem_e);
    item->sym = lvalue->sym;
    item->index = newexpr_conststring(name);
    return item;
}

expr* make_call(expr* lvalue, expr* elist, unsigned label, unsigned int line, unsigned int scope){
    expr* func = emit_iftableitem(lvalue, label, line, scope);
    expr* tmp = elist;
    while(tmp!=NULL){
        emit(param, NULL, NULL, tmp, label, line);
        if(tmp->next==NULL) break;
        tmp = tmp->next;
    }
    assert(func);
    emit(call, NULL, NULL, func, label, line);
    expr* result = newexpr(var_e);
    result->sym = newtemp(scope, line);
    emit(getretval, NULL, NULL, result, label, line);
    // resettemp();
    return result;
}

int checkuminus(expr *e)
{
    if (e->type == constbool_e || e->type == conststring_e || e->type == nil_e || e->type == newtable_e || e->type == programfunc_e || e->type == libfunc_e || e->type == boolexpr_e)
    {
        return 1;
    }
    return 0;
}

unsigned int istempname(char *s){
    return *s == '_';
}

unsigned int istempexpr(expr* e){
    return e->sym && e->sym->type == var_s && istempname(e->sym->name);
}

st_entry *newtemp(int scope, int line)
{

    st_entry* sym;
    var* new_var;
    char *prefix = "_t";
    char *tmpname;
    char temp[50];
    sprintf(temp, "%d", tempcounter);
    tmpname = (char *)malloc(sizeof(prefix) + sizeof(temp));
    strcpy(tmpname, prefix);
    tmpname = strcat(tmpname, temp);
    sym = lookup(scope, tmpname);
    if(sym==NULL){
        // printf("Temp is null malaka\n");
        new_var = create_symbol_var(tmpname, scope, line);
        sym = create_entry(new_var, LOCAL);
        sym->offset = currscopeoffset();
        sym->space = currscopespace();
        inccurrentscopeoffset();
        tempcounter++;
        return sym;
    }
    else{
        if(is_func)
        tempcounter++;
        return sym;
    }
}

void resettemp(){
    tempcounter = 0;
}

void print_op(FILE *fp, iopcode op)
{
    char *s;
    switch (op)
    {
    case assign:
        s = strdup("assign");
        fprintf(fp, "%-8s", s);
        break;
    case add:
        s = strdup("add");
        fprintf(fp, "%-8s", s);
        break;
    case sub:
        s = strdup("sub");
        fprintf(fp, "%-8s", s);
        break;
    case mul:
        s = strdup("mul");
        fprintf(fp, "%-8s", s);
        break;
    case diva:
        s = strdup("div");
        fprintf(fp, "%-8s", s);
        break;
    case mod:
        s = strdup("mod");
        fprintf(fp, "%-8s", s);
        break;
    case uminus:
        s = strdup("uminus");
        fprintf(fp, "%-8s", s);
        break;
    case or:
        s = strdup("or");
        fprintf(fp, "%-8s", s);
        break;
    case and:
        s = strdup("and");
        fprintf(fp, "%-8s", s);
        break;
    case jump:
        s = strdup("jump");
        fprintf(fp, "%-8s", s);
        break;
    case not:
        s = strdup("not");
        fprintf(fp, "%-8s", s);
        break;
    case ifeq:
        s = strdup("ifeq");
        fprintf(fp, "%-8s", s);
        break;
    case if_noteq:
        s = strdup("ifnoteq");
        fprintf(fp, "%-8s", s);
        break;
    case if_lesseq:
        s = strdup("if_lesseq");
        fprintf(fp, "%-8s", s);
        break;
    case if_greatereq:
        s = strdup("if_grtereq");
        fprintf(fp, "%-8s", s);
        break;
    case if_less:
        s = strdup("if_less");
        fprintf(fp, "%-8s", s);
        break;
    case if_greater:
        s = strdup("if_greater");
        fprintf(fp, "%-8s", s);
        break;
    case call:
        s = strdup("call");
        fprintf(fp, "%-8s", s);
        break;
    case param:
        s = strdup("param");
        fprintf(fp, "%-8s", s);
        break;
    case ret:
        s = strdup("ret");
        fprintf(fp, "%-8s", s);
        break;
    case getretval:
        s = strdup("getretval");
        fprintf(fp, "%-8s", s);
        break;
    case funcstart:
        s = strdup("funcstart");
        fprintf(fp, "%-8s", s);
        break;
    case funcend:
        s = strdup("funcend");
        fprintf(fp, "%-8s", s);
        break;
    case tablecreate:
        s = strdup("tblcrt");
        fprintf(fp, "%-8s", s);
        break;
    case tablegetelem:
        s = strdup("tblgetelem");
        fprintf(fp, "%-8s", s);
        break;
    case tablesetelem:
        s = strdup("tblsetelem");
        fprintf(fp, "%-8s", s);
        break;
    default:
        fprintf(fp, "wtf");
        break;
    }
    return;
}

expr* insert_list_front(expr* head, expr* new){
    expr* tmp = (expr*)malloc(sizeof(expr));
    tmp = new;
    if(head==NULL){
        head = tmp;
        return head;
    }
    tmp->next = head;
    head = tmp;

    return head;
}

expr* merge(expr* head1, expr* head2){
    expr* tmp = head1;
    while(tmp){
        if(!tmp->next){
            tmp->next = head2;
            break;
        }
        tmp = tmp->next;
    }
    return head1;
}

list* merge_list(list* head1, list *head2)
{
    printf("MERGE LISTS\n");
    list *tmp = head1;
    while (tmp)
    {
        if (!tmp->next)
        {
            if(head2!=NULL){
                tmp->next = head2;
                printf("merged\n");
                break;                
            }
            else return head1;
        }
        tmp = tmp->next;
    }
    return head1;
}


void push_func(func_stack** head, int offset){

    func_stack *new = (func_stack*)malloc(sizeof(func_stack));

    if(*head == NULL){
        new->offset = offset;
        new->next = NULL;
        // *head = new;
        printf("push_func first push: offset = %d\n", new->offset);
    }
    new->next = *head;
    new->offset = offset;
    *head = new;
    printf("new push_func: offset = %d\n", new->offset);
    // return head;

}


int pop_func(func_stack* head){
    if(head==NULL){
        printf("func stack is empty\n");
        return -1;
    }
    func_stack *tmp = head;
    int result = tmp->offset;
    head = tmp->next;
    func_head = head;
    // free(tmp);
    printf("new pop_func: offset = %d\n", result);
    return result;
}


void push_loop_stack(loop_stack** head, int c){
    loop_stack* new = (loop_stack*)malloc(sizeof(loop_stack));
    new->counter = c;
    if(*head == NULL){
        *head = new;
        new->next = NULL;
        printf("first push_loop: counter = %d\n", new->counter);

        return;
    }
    new->next = *head;
    *head = new;
    printf("new push_loop: counter = %d\n", new->counter);
    return;
}

int pop_loop(loop_stack* head){
    if(head==NULL){
        printf("loop stack is empty\n");
        return -1;
    }
    loop_stack* tmp = head;
    int result = tmp->counter;
    head = head->next;
    loop_head = head;
    // free(tmp);
    printf("new pop_loop: counter = %d\n", result);
}

bc_stack* push_bc_stack(bc_stack **head, e_stack* c)
{
    bc_stack *new = (bc_stack *)malloc(sizeof(bc_stack));
    new->bc_list = (e_stack*)malloc(sizeof(e_stack));
    new->bc_list->breaklist = c->breaklist;
    if (*head == NULL)
    {
        *head = new;
        new->next = NULL;
        printf("first bc_push\n");
        return *head;
    }
    new->next = *head;
    *head = new;
    return *head;
}

e_stack* pop_bc(bc_stack *head)
{
    if (head == NULL)
    {
        printf("bc stack is empty\n");
        return NULL;
    }
    bc_stack *tmp = head;
    e_stack* result = (e_stack*)malloc(sizeof(e_stack));
    memcpy(&result, &head->bc_list, sizeof(e_stack));
    head = head->next;
    bc_head = head;
    // free(tmp);
    return result;
}

void print_quads()
{
    FILE *fp = fopen("quads.txt", "w");
    int i;
    for (i = 0; i < currQuad; i++)
    {
        quad *p = &quads[i];
        fprintf(fp, "[%d]\t", i);
        print_op(fp, p->op);
        if (p->arg1 != NULL)
        {
            switch (p->arg1->type)
            {
            case constnum_e:
                fprintf(fp, "\t%-8f", p->arg1->numConst);
                break;
            case constbool_e:
                fprintf(fp, "\t%-8c", p->arg1->boolConst);
                break;
            // case boolexpr_e:
            //     fprintf(fp, "\t%-8c", p->arg1->boolConst);
            //     break;
            case conststring_e:
                fprintf(fp, "\t\'%-8s\'", p->arg1->strConst);
                break;
            case nil_e:
                fprintf(fp, "\t\'%-8s\'", "nil");
                break;
            default:
                fprintf(fp, "\t%-8s", p->arg1->sym->value.varVal->name);
            }
        }
        else
            fprintf(fp, "\t%-8s", "NULL");
        if (p->arg2 != NULL)
        {
            switch (p->arg2->type)
            {
            case constnum_e:
                fprintf(fp, "\t%-8f", p->arg2->numConst);
                break;
            case constbool_e:
                fprintf(fp, "\t%-8c", p->arg2->boolConst);
                break;
            // case boolexpr_e:
            //     fprintf(fp, "\t%-8c", p->arg2->boolConst);
            //     break;
            case conststring_e:
                fprintf(fp, "\t\'%-8s\'", p->arg2->strConst);
                break;
            case nil_e:
                fprintf(fp, "\t\'%-8s\'", "nil");
                break;
            default:
                fprintf(fp, "\t%-8s", p->arg2->sym->value.varVal->name);
            }
        }
        else
            fprintf(fp, "\t%-8s", "NULL");
        if (p->result != NULL)
        {
            switch (p->result->type)
            {
            case constnum_e:
                fprintf(fp, "\t%-8f", p->result->numConst);
                break;
            case constbool_e:
                fprintf(fp, "\t%-8c", p->result->boolConst);
                break;
            // case boolexpr_e:
            //     fprintf(fp, "\t%-8c", p->result->boolConst);
            //     break;
            case conststring_e:
                fprintf(fp, "\t\'%-8s\'", p->result->strConst);
                break;
            case nil_e:
                fprintf(fp, "\t\'%-8s\'", "nil");
                break;
            default:
                switch (p->result->sym->type)
                {
                case GLOBAL:
                case LOCAL:
                case FORMAL:
                case DUMMY:
                    fprintf(fp, "\t%-8s", p->result->sym->value.varVal->name);
                    break;
                case USERFUNC:
                    printf("USERFUNC NAME: \t%-8s\n", p->result->sym->value.funcVal->name);
                    fprintf(fp, "\t%-8s", p->result->sym->value.funcVal->name);
                    break;
                case LIBFUNC:
                    fprintf(fp, "\t%-8s", p->result->sym->value.funcVal->name);
                    break;
                }
            }
        }
        else
            fprintf(fp, "\t%-8s", "NULL");
        if (p->label >= 0)
            fprintf(fp, "\t%-8u", p->label);
        fprintf(fp, "\n");
    }
    fclose(fp);
}