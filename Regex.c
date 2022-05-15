#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define MAXREGEXSTR 100
#define MAXSTR 1000

enum RegType {SINGLE, LIST, ANY, ATMOST1, STAR, PLUS, FIRST, LAST, NOTLIST, BLANK, OR, EXPR,
                RECURSE};

typedef struct regex* reg;
typedef struct ParsedRegex* preg;

struct regex{
    enum RegType type;
    char *str;
    reg next;
    reg subExpr;
};

struct ParsedRegex{
    enum RegType type1;
    enum RegType type2;
    enum RegType type3;
    enum RegType type4;
    int numTypes;
    char *str;
    preg next;
};

struct ORList{
    reg t1;
    reg next;
};

/*----------------------------------------------------------------------------
Stack Implementation
----------------------------------------------------------------------------*/

#define PROGSTACK 1000
typedef struct Stack* st;
struct Stack{
    int st[PROGSTACK]; // Stack of size 10^3
    int sp; //stack ptr
};

// Push v onto stack s
void push(st s, int v){
    if(s->sp >= PROGSTACK){
        printf("Error in program! Stack overflow");
    }
    s->st[(s->sp)++] = v;
}
// remove and return the top value of s
int pop(st s){
    if(s->sp == 0){
        printf("Error in program! No elements on stack");
    }
    return s->st[--(s->sp)];
}

int top(st s){
    if(s->sp <= 0){
        printf("Error CTOP");
        return -1;
    }
    return s->st[s->sp - 1];
}

/*----------------------------------------------------------------------------
RegEx Implementation
----------------------------------------------------------------------------*/

reg emptyReg(){
    reg r = malloc(sizeof(struct regex));
    return r;
}

reg makeReg(enum RegType type, char *str, reg next){
    reg r = emptyReg();
    r->next = next;
    r->type = type;
    r->str = str;
    r->subExpr = 0;
    return r;
}

reg makeSubReg(enum RegType type, char *str, reg next, reg sub){
    reg r = makeReg(type, str, next);
    r->subExpr = sub;
    return r;
}

reg revReg(reg r){
    reg rev = NULL;
    reg t = NULL;
    reg t1 = NULL;
    reg t2 = r;
    while(r){
        t2 = r->next;
        r->next = rev;
        rev = r;
        r = t2;
    }
    return rev;
}
// given a regex with a (), return whatever is in the () by reading it into a different string
int getSubExpr(char *s, char *dest, int pos){
    int j = 0;
    while(s[pos] && s[pos] != ')'){
        dest[j++] = s[pos++];
    }
    dest[j] = 0;
    return pos;
}

char *preProcess(char *exp){
    int i = 0;
    int hasOr = 0;
    int hasLast = 1;
    while(exp[i]){
        if(exp[i] == '|'){
            hasOr = 1;
        }
        i++;
        if(!exp[i + 1] && exp[i] == '$'){
            hasLast = 1;
        }
    }
    int hasFirst = (exp[0] == '^');
    i = 0;
    if(!hasOr){
        return exp;
    }
    char *exp1 = malloc(sizeof(char) * MAXSTR);
    int j = 0;
    exp1[j++] = '(';
    while(exp[i]){
        if(exp[i] == '|'){
            if(hasLast)
                exp1[j++] = '$';
            exp1[j++] = ')';
            exp1[j++] = exp[i++];
            exp1[j++] = '(';
            if(hasFirst)
                exp1[j++] = '^';
        }
        else{
            exp1[j++] = exp[i++];
        }
    }
    if(hasLast){
        exp1[j++] = '$';
    }
    exp1[j++] = ')';
    return exp1;
}

reg splitRegEx(char *e){
    reg r = NULL;
    int i = 0;
    char *exp = preProcess(e);
    printf("%s\n", exp);
    if(exp[i] == '^'){
        r = makeReg(FIRST, NULL, r);
        i++;
    }
    char *prevStr = NULL;
    while(exp[i]){
        if(exp[i] == '['){
            i++;
            char *currStr = malloc(sizeof(char) * MAXREGEXSTR);
            int j = 0;
            while(exp[i] != ']'){
                currStr[j++] = exp[i++];
            }
            currStr[j] = 0;
            i++;
            r = makeReg(LIST, currStr, r);
            prevStr = currStr;
        }
        else if(exp[i] == '*'){
            i++;
            r = makeReg(STAR, NULL, r);
        }
        else if(exp[i] == '+'){
            i++;
            r = makeReg(PLUS, NULL, r);
        }
        else if(exp[i] == '.'){
            i++;
            r = makeReg(ANY, NULL, r);
        }
        else if(exp[i] == '?'){
            i++;
            r = makeReg(ATMOST1, NULL, r);
        }
        else if(exp[i] == '$'){
            r = makeReg(LAST, NULL, r);
            return r;
        }
        else if(exp[i] == '|'){
            r = makeReg(OR, NULL, r);
            i++;
        }
        else if(exp[i] == '('){
            char *subExpr = malloc(sizeof(char) * 200);
            i = getSubExpr(exp, subExpr, i + 1);
            reg t = splitRegEx(subExpr);
            reg t1 = revReg(t);
            r = makeSubReg(EXPR, NULL, r, t1);
            i++;
        }
        else{
            if(exp[i] == '\\'){
                char *c = malloc(sizeof(char) * 2);
                *c = exp[++i];
                c[1] = 0;
                r = makeReg(SINGLE, c, r);
                ++i;
            }
            else{
                char *c = malloc(sizeof(char) * 2);
                *c = exp[i++];
                c[1] = 0;
                r = makeReg(SINGLE, c, r);
            }
        }
    }
    //r = revReg(r);
    return r;
}

void printReg(reg);

void printRegType(enum RegType t){
        switch (t)
        {
        case SINGLE:
            printf("SINGLE ");
            break;
        case LIST:
            printf("LIST ");
            break;
        case ANY:
            printf("ANY ");
            break;
        case ATMOST1:
            printf("ATMOST1 ");
            break;
        case STAR:
            printf("STAR ");
            break;
        case PLUS:
            printf("PLUS ");
            break;
        case FIRST:
            printf("FIRST ");
            break;
        case LAST:
            printf("LAST ");
            break;
        case NOTLIST:
            printf("NOTLIST ");
            break;
        case BLANK:
            printf("BLANK ");
            break;
        case EXPR:
            printf("EXPR ");
            break;
        case RECURSE:
            printf("RECURSE ");
            break;
        default:
            break;
        }
}

void printReg(reg r){
    while(r){
        printRegType(r->type);
        if(r->str){
            printf("%s ", r->str);
        }
        else    printf("NoStr");
        if(r->subExpr){
            printf("SubExpression: ");
            printReg(r->subExpr);
        }
        r = r->next;
    }
}

void printPreg(preg r){
    while(r){
        printRegType(r->type1);
        printRegType(r->type2);
        printRegType(r->type3);
        printRegType(r->type4);

        if(r->str){
            printf("%s ", r->str);
        }
        else{printf("NoStr");}
        r = r->next;
    }
}


preg emptyPreg(){
    preg p = malloc(sizeof(struct ParsedRegex));
    return p;
}

preg makePreg(enum RegType t1, enum RegType t2, enum RegType t3, enum RegType t4, int regTypes, char *s, preg prev){
    preg p = emptyPreg();
    p->type1 = t1;
    p->type2 = t2;
    p->type3 = t3;
    p->type4 = t4;
    p->numTypes = regTypes;
    p->str = s;
    p->next = p;
}

preg parseReg(reg r){
    if(r == NULL)
        return NULL;
    
    reg prev = NULL;
    reg r1 = r;
    reg r2 = r->next;
    preg res = NULL;
    if(!r2){
        return res;
    }
    if(r1->type == FIRST){
        prev = r1;
        r1 = r2;
        r2 = r2->next;
    }
    else if(r2->type == LAST){
        res = makePreg(LAST, BLANK, BLANK, BLANK, 1, r1->str, res);
    }
    
    int move2 = 0;
    while(r1){
        if(prev->type == FIRST){
            if(r2){
                if(r2->type == PLUS || r2->type == STAR){
                    res = makePreg(PLUS, r1->type, BLANK, BLANK, 2, r1->str, res);
                    move2 = 1;
                }
                else if(r2->type == ATMOST1){
                    res = makePreg(FIRST, r1->type, BLANK, BLANK, 2, r1->str, res);
                    move2 = 1;
                }
                else if(r2->type == LAST){
                    res = makePreg(FIRST, LAST, r1->type, BLANK, 3, r1->str, res);
                    return res;
                }
                else{
                    res = makePreg(FIRST, r1->type, BLANK, BLANK, 2, r1->str, res);
                    move2 = 0;
                }
            }
            else{
                res = makePreg(FIRST, r1->type, BLANK, BLANK, 2, r1->str, res);
                return res;
            }
        }
        else if(r2){
            if(r2->type == PLUS || r2->type == STAR || r2->type == ATMOST1){
                res = makePreg(r2->type, r1->type, BLANK, BLANK, 2, r1->str, res);
                move2 = 1;
            }
            else if(r2->type == LAST){
                int num = res->numTypes;
                if(num == 0){
                    res->type1 = LAST;
                }
                else if(num == 1){
                    res->type2 = LAST;
                }
                else if(num == 2){
                    res->type3 = LAST;
                }
                else{
                    res->type4 = LAST;
                }
                res->numTypes = num + 1;
                return res;
            }
            else{
                res = makePreg(r1->type, BLANK, BLANK, BLANK, 1, r1->str, res);
                move2 = 0;
            }
        }
        else{
            res = makePreg(r1->type, BLANK, BLANK, BLANK, 1, r1->str, res);
            break;
            move2 = 0;
        }
        if(move2){
            if(r2->next){
                prev = r1->next;
                r1 = r2->next;
                if(r1->next){
                    r2 = r1->next;
                }
                else{
                    r2 = 0;
                }
            }
            else{
                break;
            }
        }
        else{
            prev = r1;
            r1 = r2;
            r2 = r2->next;
        }
    }
    return res;
}

int charIsInList(char c, char *l){
    //printf("%s ", l);
    int len = strlen(l);
    // printf("%d ", len);
    for(int i = 0; i < len; i++){
        if(i < len - 1 && i > 0){
            if(l[i] == '-'){
                char start = l[i - 1];
                char end = l[i + 1];
                if(start <= c && c <= end){
                    return 1;
                }
            }
            else{
                if(l[i] == c){
                    return 1;
                }
            }
        }
        else{
                if(l[i] == c){
                    return 1;
                }
            }
    }
    return 0;
}

void regexErrorMsg(int x){
    printf("Error in Regex at %d", x);
}

char *remString(char *s, int i){
    char *t = malloc(sizeof(char) * (strlen(s) - i + 1));
    int j = 0;
    while(s[i]){
        t[j++] = s[i++];
    }
    t[j] = 0;
    return t;
}

reg copyReg(reg t){
    reg s = 0;
    while(t){
        if(t->type == EXPR){
            reg t1 = copyReg(t->subExpr);
            char *s1 = malloc(sizeof(char) * MAXREGEXSTR);
            strcpy(s1, t->str);
            s = makeSubReg(EXPR, s1, s, t1);
        }
        else{
            char *s1 = 0;
            if(t->str){
                s1 = malloc(sizeof(char) * MAXREGEXSTR);
                //printf("%s", t->str);
                strcpy(s1, t->str);
            }
            s = makeReg(t->type, s1, s);
        }
        t = t->next;
    }
    reg s1 = revReg(s);
    return s1;
}

reg appendReg(reg t1, reg t2){
    reg t1Iterator = t1;
    if(!t1){
        return t2;
    }
    while(t1Iterator->next){
        t1Iterator = t1Iterator->next;
    }
    while(t2){
        t1Iterator->next = t2;
        t2 = t2->next;
        t1Iterator = t1Iterator->next;
    }
    return t1;
}

void freeReg(reg r){
    reg temp = r;
    //printf("FREEING\n");
    while(r){
        temp = r->next;
        //printReg(r);
        //printf("\n");
        free(r->str);
        if(r->type == EXPR){
            freeReg(r->subExpr);
            break;
        }
        free(r);
        r = temp;
    }
}

st depthStack;

int evalRegex(reg R, char *s){
    //printf("%s, ", s);
    //printReg(R);
    //printf("\n");
    if(!R){
        return 1;
    }
    int l = strlen(s);
    int i = 0;
    int inRecurse = 0;
    if(R->type == RECURSE){
        inRecurse = 1;
    }
    if(R->type == FIRST || R->type == RECURSE){
        if(R->next){
            if(R->next->type == SINGLE){
                if(s[0] == R->next->str[0]){
                    i = 1;
                }
                else{
                    return 0;
                }
                if(R->next->next){
                    if(R->next->next->type == STAR || R->next->next->type == PLUS){
                        R = R->next;
                        i = 0;
                    }
                    else{
                        R = R->next->next;
                    }
                }
                else{
                    return 1;
                }
            }
            else if(R->next->type == LIST){
                if(charIsInList(s[0], R->next->str)){
                    i = 1;
                }
                else return 0;
                if(R->next->next){
                    if(R->next->next->type == STAR || R->next->next->type == PLUS){
                        R = R->next;
                        i = 0;
                    }
                    else{
                        R = R->next->next;
                    }
                }
                else{
                    return 1;
                }
            }
            else if(R->next->type == EXPR){
                reg t1 = makeReg(FIRST, NULL, R->next->subExpr);
                int res1 = evalRegex(t1, s);
                free(t1);
                if(!res1)
                    return 0;
                R = R->next;
            }
        }
        else return 1;
    }
    else{
        /*if(R->type == SINGLE){
            
        }
        else if(R->type == LIST){
            while(s[i] && !charIsInList(s[i], R->str)){
                i++;
            }
            if(!s[i]){
                return 0;
            }
        }
        else if(R->type == ANY){
            
        }
        */
        char *t = malloc(sizeof(char) * MAXSTR);
        reg t1 = makeReg(FIRST, NULL, R);
        while(s[i]){
            printf("\nIn First Loop\n\n");
            t = remString(s, i);
            if(evalRegex(t1, t)){
                free(t);
                free(t1);
                return 1;
            }
            free(t);
            i++;
        }
        free(t1);
        if(!s[i]){
            return 0;
        }
    }

    //printReg(R);
    //printf("; %d\n", i);
    while(R && s[i]){
        //printReg(R);
        //printf("; %d\n", i);
        if(R->next){
            if(R->next->type == STAR){
                int isPossible = 0;
                if(R->type == SINGLE){
                    // If we need to check at least zero instances, then we need to loop over all of them and see which one works
                    while(s[i] && s[i] == R->str[0]){
                        char *t = remString(s, i);
                        //printf("In Loop StarSingle %s ", t);
                        reg tmp = makeReg(FIRST, NULL, R->next->next);
                        int isPossible = evalRegex(tmp, t);
                        if(isPossible){
                            // we can return here itself as recursive case is checked.
                            free(t);
                            free(tmp);
                            return 1;
                        }
                        free(t);
                        free(tmp);
                        i++;
                    }
                    char *t = remString(s, i);
                    reg tmp = makeReg(FIRST, NULL, R->next->next);
                    int isPossible = evalRegex(tmp, t);
                    
                    if(isPossible){
                        free(t);
                        free(tmp);
                        return 1;
                    }
                    free(t);
                    free(tmp);
                    return 0;
                }
                else if(R->type == LIST){
                    // If we need to check at least zero instances, then we need to loop over all of them and see which one works
                    printf("Entering Externally\n");
                    while(s[i] && charIsInList(s[i], R->str)){
                        char *t = remString(s, i);
                        printf("In Loop StarSingle %s \n", t);
                        reg tmp = makeReg(FIRST, NULL, R->next->next);
                        int isPossible = evalRegex(tmp, t);
                        if(isPossible){
                            // we can return here itself as recursive case is checked.
                            free(t);
                            free(tmp);
                            /*if(inRecurse){
                                push(depthStack, i);
                            }*/
                            return 1;
                        }
                        free(t);
                        free(tmp);
                        i++;
                    }
                    char *t = remString(s, i);
                    reg tmp = makeReg(FIRST, NULL, R->next->next);
                    int isPossible = evalRegex(tmp, t);
                    
                    if(isPossible){
                        /*if(inRecurse){
                                push(depthStack, i);
                            }*/
                        free(t);
                        free(tmp);
                        return 1;
                    }
                    free(t);
                    free(tmp);
                    return 0;
                }
                else if(R->type == ANY){
                    while(s[i]){
                        char *t = remString(s, i);
                        //printf("In Loop StarSingle %s ", t);
                        int isPossible = evalRegex(R->next->next, t);
                        if(isPossible){
                            // we can return here itself as recursive case is checked.
                            free(t);
                            /*if(inRecurse){
                                push(depthStack, i);
                            }*/
                            return 1;
                        }
                        free(t);
                        i++;
                    }
                    return 0;
                }
                else if(R->type == EXPR){
                    // Of the form (expr)*
                    reg subExpr = R->subExpr;
                    char *t = remString(s, i);
                    reg t0 = copyReg(subExpr);
                    // reg t1 = makeReg(RECURSE, NULL, R->subExpr);
                    reg t1 = appendReg(subExpr, R->next->next);
                    reg t2 = copyReg(t0);
                    //printf("\n");
                    //printReg(subExpr);
                    //printf("\n");
                    reg t3 = makeReg(FIRST, NULL, t0);
                    int res1 = evalRegex(t3, t);
                    free(t3);
                    printf("\nEXPR STAR RES: %d\n", res1);
                    if(res1){
                        while(res1){
                            printf("EXPR STAR RES: %d\n", res1);
                            /*int upd = pop(depthStack);
                            if(upd < 0){
                                if(inRecurse){
                                push(depthStack, -1);
                            }
                                return 0;
                            }
                            i += upd + 1;
                            t = remString(s, i);
                            */
                            //reg t2 = makeReg(FIRST, NULL, R->next->next);
                            printf("Now Eval Regex(t1, t)\n");
                            t3 = makeReg(FIRST, NULL, subExpr);
                            if(evalRegex(t3, t)){
                                freeReg(t2);
                                free(t3);
                                //freeReg(subExpr);
                                return 1;
                            }
                            free(t3);
                            reg temp0 = copyReg(t0);
                            subExpr = appendReg(temp0, R->subExpr);
                            t2 = appendReg(t2, t0);
                            printReg(subExpr);
                            printf("\n");
                            t3 = makeReg(FIRST, NULL, t2);
                            res1 = evalRegex(t3, t);
                            free(t3);
                            //t1 = appendReg(R->subExpr, t1);
                            /*int evalRem = evalRegex(t2, t);
                            if(evalRem){
                                /*if(inRecurse){
                                push(depthStack, i);
                            }
                                free(t1);
                                // free(t);
                                return 1;
                            }
                            else{
                                res1 = evalRegex(t1, t);
                            }*/
                        }
                        /*if(inRecurse){
                                push(depthStack, -1);
                            }*/
                        freeReg(t2);
                        free(t);
                        // free(t1);
                        return 0;
                    }
                    else{
                        reg t3 = makeReg(FIRST, NULL, R->next->next);
                        res1 = evalRegex(t3, t);
                        free(t3);
                        return res1;
                    }
                }
            }
            else if(R->next->type == PLUS){
                int isPossible = 0;
                if(R->type == SINGLE){
                    // If we need to check at least zero instances, then we need to loop over all of them and see which one works
                    while(s[i] && s[i] == R->str[0]){
                        char *t = remString(s, i);
                        //printf("In Loop StarSingle %s ", t);
                        reg tmp = makeReg(FIRST, NULL, R->next->next);
                        int isPossible = evalRegex(tmp, t);
                        if(isPossible){
                            // we can return here itself as recursive case is checked.
                            free(t);
                            free(tmp);
                            return 1;
                        }
                        free(t);
                        free(tmp);
                        i++;
                    }
                    return 0;
                }
                else if(R->type == LIST){
                    if(s[i] && charIsInList(s[i], R->str)){
                        while(s[i] && charIsInList(s[i], R->str)){
                            char *t = remString(s, i);
                            int isPossible = evalRegex(R->next->next, t);
                            if(isPossible){
                                // we can return here itself as recursive case is checked.
                                /*if(inRecurse){
                                    push(depthStack, i);
                                }*/
                                free(t);
                                return 1;
                            }
                            free(t);
                            i++;
                        }
                        char *t = remString(s, i);
                        //printf("Last Case: %s", t);
                        int isPossible = evalRegex(R->next->next, t);
                        if(isPossible){
                            /*if(inRecurse){
                                push(depthStack, i);
                            }*/
                            return 1;
                        }
                        /*if(inRecurse){
                                push(depthStack, -1);
                            }*/
                        free(t);
                        return 0;
                    }
                    else{
                        /*if(inRecurse){
                                push(depthStack, i);
                            }*/
                        return 0;
                    }
                }
                else if(R->type == ANY){
                    if(s[i]){
                        while(s[i]){
                            char *t = remString(s, i);
                            //printf("In Loop StarSingle %s ", t);
                            int isPossible = evalRegex(R->next->next, t);
                            if(isPossible){
                                // we can return here itself as recursive case is checked.
                                free(t);
                                return 1;
                            }
                            free(t);
                            i++;
                        }
                        return 0;
                    }
                    else{
                        return 0;
                    }
                }
                else if(R->type == EXPR){
                    // Of the form (expr)*
                    reg subExpr = R->subExpr;
                    char *t = remString(s, i);
                    reg t0 = copyReg(subExpr);
                    // reg t1 = makeReg(RECURSE, NULL, R->subExpr);
                    reg t1 = appendReg(subExpr, R->next->next);
                    reg t2 = copyReg(t0);
                    //printf("\n");
                    //printReg(subExpr);
                    //printf("\n");
                    reg t3 = makeReg(FIRST, NULL, t0);
                    int res1 = evalRegex(t3, t);
                    free(t3);
                    printf("\nEXPR PLUS RES: %d\n", res1);
                    if(res1){
                        while(res1){
                            printf("EXPR PLUS RES: %d\n", res1);
                            printf("Now Eval Regex(t1, t)\n");
                            t3 = makeReg(FIRST, NULL, subExpr);
                            printReg(t0);
                            printf("\n");
                            printReg(t1);
                            printf("\n");
                            printReg(t2);
                            printf("\n");
                            printReg(t3);
                            printf("\n");
                            if(evalRegex(t3, t)){
                                freeReg(t2);
                                free(t3);
                                //freeReg(subExpr);
                                return 1;
                            }
                            free(t3);
                            reg temp0 = copyReg(t0);
                            subExpr = appendReg(temp0, R->subExpr);
                            t2 = appendReg(t2, t0);
                            printReg(subExpr);
                            printf("\n");
                            t3 = makeReg(FIRST, NULL, t2);
                            res1 = evalRegex(t3, t);
                            printf("Second Round\n");
                            printReg(t0);
                            printf("\n");
                            printReg(t1);
                            printf("\n");
                            printReg(t2);
                            printf("\n");
                            printReg(t3);
                            printf("\n");
                            free(t3);
                            printf("Freed");
                        }
                        freeReg(t2);
                        free(t);
                        // free(t1);
                        return 0;
                    }
                    else{
                        return 0;
                    }
                }
            }
            else if(R->next->type == ATMOST1){
                if(R->type == SINGLE){
                    if(s[i] && s[i] == R->str[0]){
                        char *t = remString(s, i);
                        char *t1 = remString(s, i + 1);
                        //printf("In Loop StarSingle %s ", t);
                        reg tmp = makeReg(FIRST, NULL, R->next->next);
                        int isPossible = evalRegex(tmp, t) || evalRegex(tmp, t1);
                        if(isPossible){
                            // we can return here itself as recursive case is checked.
                            free(t);
                            free(tmp);
                            free(t1);
                            return 1;
                        }
                        free(t);
                        free(t1);
                        free(tmp);
                    }
                    return 0;
                }
                if(R->type == SINGLE){
                    if(s[i] && s[i] == R->str[0]){
                        reg Temp = makeReg(SINGLE, R->str, R->next->next);
                        char *t = remString(s, i);
                        int isPossible = evalRegex(Temp, t);
                        if(isPossible){
                            free(t);
                            free(Temp);
                            /*if(inRecurse){
                                push(depthStack, i);
                            }*/
                            return 1;
                        }
                        free(t);
                        free(Temp);
                        i++;
                        R = R->next->next;
                    }
                    else{
                        R = R->next->next;
                    }
                }
                else if(R->type == LIST){
                    if(s[i] && charIsInList(s[i], R->str)){
                        reg Temp = makeReg(LIST, R->str, R->next->next);
                        char *t = remString(s, i);
                        int isPossible = evalRegex(Temp, t);
                        if(isPossible){
                            free(t);
                            free(Temp);
                            /*if(inRecurse){
                                push(depthStack, i);
                            }*/
                            return 1;
                        }
                        free(t);
                        free(Temp);
                        i++;
                        R = R->next->next;
                    }
                    else{
                        R = R->next->next;
                    }
                }
                else if(R->type == ANY){
                    if(s[i]){
                        char *t = remString(s, i + 1);
                        int isPossible = evalRegex(R->next->next, t);
                        if(isPossible){
                            free(t);
                            /*if(inRecurse){
                                push(depthStack, i);
                            }*/
                            return 1;
                        }
                        
                    }
                    else{
                        R = R->next->next;
                    }
                }
                else if(R->type == EXPR){
                    // Of the form (expr)*
                    reg subExpr = R->subExpr;
                    char *t = remString(s, i);
                    //reg t0 = copyReg(subExpr);
                    reg t1 = appendReg(subExpr, R->next->next);
                    reg t2 = makeReg(FIRST, NULL, R->next->next);
                    reg t3 = makeReg(FIRST, NULL, t1);
                    int res1 = evalRegex(t2, t);
                    int res2 = evalRegex(t3, t);
                    return res1 || res2;
                    
                }
            }
            else{
                if(R->type == SINGLE){
                    if(R->str[0] != s[i]){
                        //printf("4h\n");
                        /*if(inRecurse){
                            push(depthStack, -1);
                        }*/
                        return 0;
                    }
                    else{
                        i++;
                        R = R->next;
                    }
                }
            }
        }
        else if(R->type == SINGLE){
            if(s[i] != R->str[0]){
                //printf("3h\n");
                /*if(inRecurse){
                    push(depthStack, -1);
                }*/
                return 0;
            }
            else{
                R = R->next;
                i++;
            }
        }
        else if(R->type == EXPR){
            char *t = remString(s, i);
            return evalRegex(R->subExpr, t);
        }
        else{
            if(s[i] != R->str[0]){
                //printf("3h");
                /*if(inRecurse){
                    push(depthStack, -1);
                }*/
                return 0;
            }
            else{
                R = R->next;
                i++;
            }
        }
    }
    /*if(inRecurse){
        push(depthStack, i - 1);
    }
    */
    return 1;
}


int main(){

    //printPreg(parseReg(revReg(splitRegEx("^a[1234]int$"))));
    // char s[200] = "^[12 34]*(abc)*+ *intt?\0";
    // char s[200] = "(abc*)*def\0";
    //char s[200] = "([abcd] *)+def\0";
    depthStack = malloc(sizeof(struct Stack));
    depthStack->sp = 0;
    //reg r = revReg(splitRegEx(s));
    //printReg(r);
    //int res = evalRegex(r, "abceccddccabcddef");
    //int res = evalRegex(r, "abccccccccadbcccdef");
    // int res = evalRegex(r, "abccccabcdef");
    //printf("%d\n", res);
    //freeReg(r);
    //int res = evalRegex(r, "13244   1      intt  ");
    // printf("%d %d\n", charIsInList(' ', "12 34"), 1);
    //printf("%d", res);
    //printf("Yes");
    //printReg(revReg(splitRegEx("^a[1234]int$")));
    char s[150] = "red|blue";
    reg r = revReg(splitRegEx(s));
    printReg(r);
    freeReg(r);
}