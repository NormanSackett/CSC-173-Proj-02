/* -*- compile-command: "gcc Parser.c -o Parser -Wall -Werror"; -*-
 *
 * Parser.c
 *
 * George Ferguson's adaptation of the code from FOCS Fig. 11.27 for
 * parsing strings of balanced parentheses.
 *
 * Changes from the code in focs_11_27.c:
 * - Define and use functions lookahead and match rather than direct
 *   string manipulation in the parsing function
 *   - These use type bool, so include stdbool.h
 * - Write parsing function B MECHANICALLY based on the productions, as seen
 *   in class.
 * - Define and use function parse to check that the entire input was consumed
 *   after the parsing function returns successfully
 * - Define a stub for printing the parse tree, but you will need to write
 *   this yourself
 *   - This uses printf, so include stdio.h
 */
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#define FAILED NULL
#define STACK_SIZE 256

typedef struct NODE *TREE;
struct NODE {
    char label;
    TREE leftmostChild, rightSibling;
};

typedef struct {
    TREE data[STACK_SIZE];
    int head;
} stack;

typedef struct {
    char *lookahead_chars; //maps characters and productions
    int **lookahead_prods;
    char **productions;
} table;

TREE makeNode0(char x);
TREE makeNode1(char x, TREE t);
TREE makeNode2(char x, TREE t1, TREE t2);
TREE makeNode3(char x, TREE t1, TREE t2, TREE t3);
TREE makeNode4(char x, TREE t1, TREE t2, TREE t3, TREE t4);

TREE parseTree; //holds the result of the parse
char *nextTerminal; //current position in input string

//Returns true if the current input symbol is the given char, otherwise false.
bool lookahead(char c) {
    //printf("%c, %c, ", *nextTerminal, c);
    return *nextTerminal == c;
}

//If the current input symbol is the given char, advance to the next
// character of the input and return true, otherwise leave it and return false.
bool match(char c) {
    if (lookahead(c)) {
	nextTerminal += 1;
	return true;
    } else {
	return false;
    }
}

//Functions representing the productions of the gievn grammar:
// <A> -> <B> <E>
// <B> -> b | <C>
// <C> -> c <D>
// <D> -> <C> | \0
// <E> -> ( <E> ) | e
TREE A();
TREE B();
TREE C();
TREE D();
TREE E();

TREE A() {
    TREE y1 = B();
    if (y1 == NULL) return NULL;
    TREE y2 = E();
    if (y2 == NULL) return NULL;

    return makeNode2('0', y1, y2); //returns a tree with the 'A' syntactic category as the root and the B and E subtrees as children
}

TREE B() {
    if (!lookahead('b')) {
        TREE y1 = C();
        if (y1 == NULL) return NULL;

        return makeNode1('1', y1);
    } else {
        if (!match('b')) return NULL;

        return makeNode1('1', makeNode0('b'));
    }
}

TREE C() {
    if (!match('c')) return NULL;
    TREE y1 = makeNode0('c');
    TREE y2 = D();
    if (y2 == NULL) return NULL;

    return makeNode2('2', y1, y2);
}

TREE D() {
    if (!lookahead('c')) { //the only construction for the 'C' syntactic category requires a 'c' character at the start
        return makeNode1('3', makeNode0('E')); //subtree is the syntactic category 'D' with a null child
    } else {
        TREE y1 = C();
        if (y1 == NULL) return NULL;

        return makeNode1('3', y1);
    }
}

TREE E() {
    TREE y1;
    if (!lookahead('(')) {
        if (!match('e')) return NULL;
        return makeNode1('4', makeNode0('e'));
    } else {
        if (!match('(')) return NULL;
        TREE y2 = makeNode0('(');
        TREE y3 = E();
        if (y3 == NULL) return NULL;
        if (!match(')')) return NULL;
        TREE y4 = makeNode0(')');
        return makeNode3('4', y2, y3, y4);
    }
}

//Attempt to parse the given string as a string of balanced parentheses.
// If the parse succeeds, return the parse tree, else return FAILED (NULL).
TREE parse(char* input) {
    nextTerminal = input;
    TREE parseTree = A();
    if (parseTree != NULL && lookahead('\0')) {
	return parseTree;
    } else {
	return FAILED;
    }
}

//table-driven parsing

TREE pop(stack *s) {
    TREE t = s->data[s->head];
    s->head--;
    
    return t;
}

void push(stack *s, TREE t) {
    if (s->head >= STACK_SIZE - 1) {
        printf("stack overflow");
        return;
    }

    s->head++;
    s->data[s->head] = t;
}

TREE peek(stack *s) {
    return s->data[s->head];
}

void add_node(TREE cur_parent, TREE t) {
    if (cur_parent->leftmostChild == NULL) {
        cur_parent->leftmostChild = t;
        return;
    }

    t->rightSibling = cur_parent->leftmostChild;
    cur_parent->leftmostChild = t;
}

int table_lookahead(table tab, int prod_num, int num_lookahead_chars) {
    for (int i = 0; i < num_lookahead_chars; i++) {
        if (lookahead(tab.lookahead_chars[i])) {
            //if the next character is one of the accepted characters, return the corresponding production
            return tab.lookahead_prods[prod_num][i];
        }
    }
    return -1;
}

TREE table_parse(table tab, int num_lookahead_chars, stack s, TREE cur_parent) {
    TREE t = pop(&s);
    char c = t->label;
    //printf("popping %c from the stack\n", c);
    cur_parent = t;
    if (48 <= c && 57 >= c) { //check if c is a production index
        int prod = table_lookahead(tab, c-48, num_lookahead_chars); //finds the needed production based on the next character
        if (prod == -1) return NULL;
        int i = 0;
        while (tab.productions[prod][i] != '\0') {
            TREE temp_tree = makeNode0(tab.productions[prod][i++]);
            add_node(cur_parent, temp_tree);
            //printf("pushing %c to the stack\n", tab.productions[prod][i]);
            push(&s, temp_tree);
        }
        if (i == 0) add_node(cur_parent, makeNode0('E')); //handle epsilon productions
    } else {
        if (!match(c)) return NULL;
    }
    
    if (peek(&s) != NULL) {
        table_parse(tab, num_lookahead_chars, s, cur_parent);
    }
    return cur_parent;
}

char *create_lookahead_chars(int size, char lookahead_chars[size]) {
    char *new_arr = malloc(size * sizeof(char));
    for (int i = 0; i < size; i++) {
        new_arr[i] = lookahead_chars[i];
    }
    return new_arr;
}

int **create_lookahead_prods(int size1, int size2, int lookahead_prods[size1][size2]) {
    int **new_table = malloc(size1 * sizeof(int *));
    for (int i = 0; i < size1; i++) {
        new_table[i] = malloc(size2 * sizeof(int));
        for (int j = 0; j < size2; j++) {
            new_table[i][j] = lookahead_prods[i][j];
        }
    }
    return new_table;
}

char **create_productions(int size1, int size2, char productions[size1][size2]) {
    char **new_table = malloc(size1 * sizeof(char *));
    for (int i = 0; i < size1; i++) {
        new_table[i] = malloc(size2 * sizeof(char));
        for (int j = 0; j < size2; j++) {
            new_table[i][j] = productions[i][j];
        }
    }
    return new_table;
}

table make_table() {
    char lookahead_chars[4] = {'b', 'c', '(', 'e'};

    //table will assume that the syntactic category at position 0 is the start symbol
    int lookahead_prods[5][4] = { //-1 encodes an invalid lookahead
        {0, 0, -1, -1},
        {1, 2, -1, -1},
        {-1, 3, -1, -1},
        {5, 4, 5, 5},
        {-1, -1, 6, 7}
    };

    //The productions are encoded backward here so they are in the correct order when pushed onto the stack.
    //Numbers correspond to syntactic categories in alignment with the syntacitic category positions in the lookahead productions table
    // and all other characters are terminals.

    //This implementation is fine for the grammar being used here, but would fail if the grammar allowed for numerical inputs
    // or had more than 10 productions. The stack could be made to hold a string rather than a single character
    // and the syntactic categories could be indicated with reserved characters (like <>). Then, when reading from the stack,
    // if the reserved characters are seen, convert the numerical characters to an integer and use that integer at the production index.
    char productions[8][6] = {  
        "41",
        "b",
        "2",
        "3c",
        "2",
        "",
        ")4(",
        "e"
    };
    table parse_table;
    parse_table.lookahead_chars = create_lookahead_chars(4, lookahead_chars);
    parse_table.lookahead_prods = create_lookahead_prods(5, 4, lookahead_prods);
    parse_table.productions = create_productions(8, 6, productions);
    return parse_table;
}

void print_tree(TREE t, char *indent, int indent_length) { //pretty print of lmcrs tree
    printf("%s%c\n", indent, t->label);

    if (t->leftmostChild != NULL) {
        TREE child = t->leftmostChild;
        while (child != NULL) {
            //temporary string is used to prepend another layer of indentation for the children of the current node
            int new_indent_length = indent_length + 3;
            char *temp_str = malloc((new_indent_length + 1) * sizeof(char));
            temp_str[0] = '|';
            temp_str[1] = ' ';
            temp_str[2] = ' ';
            for (int i = 0; i < indent_length; i++) {
                temp_str[i+3] = indent[i];
            }
            temp_str[new_indent_length] = '\0';

            print_tree(child, temp_str, new_indent_length);
            free(temp_str);
            child = child->rightSibling;
        }
    }
}

int main()
{
    char input[64];
    printf("Grammar definition:\n");
    printf("(0) <A> -> <B><E>\n");
    printf("(1) <B> -> b\n");
    printf("(2) <B> -> <C>\n");
    printf("(3) <C> -> c<D>\n");
    printf("(4) <D> -> <C>\n");
    printf("(5) <D> -> NULL\n");
    printf("(6) <E> -> (<E>)\n");
    printf("(7) <E> -> e\n");
    printf("\nEnter string (recursive descent):\n");
    scanf("%s", &input);

    parseTree = parse(input);
    if (parseTree == NULL) {
        printf("\nINVALID INPUT");
        return 0;
    }
    print_tree(parseTree, "|--", 3);
    printf("\n\nEnter string (table-driven):\n");

    scanf("%s", &input);
    nextTerminal = input;
    stack s;
    s.head = 0;
    s.data[0] = makeNode0('0');
    TREE t = makeNode0('0');
    parseTree = table_parse(make_table(), 4, s, t);
    if (parseTree == NULL) {
        printf("INVALID INPUT");
        return 0;
    }
    print_tree(parseTree, "|--", 3);
}

TREE makeNode0(char x)
{
    TREE root;
    root = (TREE) malloc(sizeof(struct NODE));
    root->label = x;
    root->leftmostChild = NULL;
    root->rightSibling = NULL;
    return root;
}

TREE makeNode1(char x, TREE t)
{
    TREE root;
    root = makeNode0(x);
    root->leftmostChild = t;
    return root;
}

TREE makeNode2(char x, TREE t1, TREE t2) {
    TREE root;
    root = makeNode0(x);
    root->leftmostChild = t1;
    t1->rightSibling = t2;
    return root;
}

TREE makeNode3(char x, TREE t1, TREE t2, TREE t3)
{
    TREE root;
    root = makeNode1(x, t1);
    t1->rightSibling = t2;
    t2->rightSibling = t3;
    return root;
}

TREE makeNode4(char x, TREE t1, TREE t2, TREE t3, TREE t4)
{
    TREE root;
    root = makeNode1(x, t1);
    t1->rightSibling = t2;
    t2->rightSibling = t3;
    t3->rightSibling = t4;
    return root;
}

