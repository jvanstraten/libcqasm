%define api.pure full
%locations

%code requires {
    #include <memory>
    #include <cstdio>
    #include "cqasm-ast.hpp"
    #include "cqasm-analyzer.hpp"
    using namespace cqasm::ast;
    typedef void* yyscan_t;
}

%code {
    int yylex(YYSTYPE* yylvalp, YYLTYPE* yyllocp, yyscan_t scanner);
    void yyerror(YYLTYPE* yyllocp, yyscan_t scanner, cqasm::AnalyzerInternals &analyzer, const char* msg);
}

%param { yyscan_t scanner }
%parse-param { cqasm::AnalyzerInternals &analyzer }

/* YYSTYPE union */
%union {
    char           	*str;
    IntegerLiteral  *ilit;
    FloatLiteral    *flit;
    Identifier      *idnt;
    Index           *indx;
    UnaryOp         *unop;
    BinaryOp        *biop;
    Expression      *expr;
    ExpressionList  *expl;
    IndexItem       *idxi;
    IndexRange      *idxr;
    IndexEntry      *idxe;
    IndexList       *idxl;
    MatrixLiteral1  *mat1;
    MatrixLiteral2  *mat2;
    MatrixLiteral   *mat;
    StringBuilder   *strb;
    StringLiteral   *slit;
    JsonLiteral     *jlit;
    AnnotationData  *adat;
    Instruction     *inst;
    Bundle          *bun;
    Mapping         *map;
    Subcircuit      *sub;
    Statement       *stmt;
    StatementList   *stms;
    Version         *vers;
    Program         *prog;
};

/* Typenames for nonterminals */
%type <ilit> IntegerLiteral
%type <flit> FloatLiteral
%type <idnt> Identifier
%type <indx> Index
%type <unop> UnaryOp
%type <biop> BinaryOp
%type <expr> Expression
%type <expl> ExpressionList
%type <idxi> IndexItem
%type <idxr> IndexRange
%type <idxe> IndexEntry
%type <idxl> IndexList
%type <mat1> MatrixLiteral1
%type <mat2> MatrixLiteral2 MatrixLiteral2c
%type <mat>  MatrixLiteral
%type <strb> StringBuilder
%type <slit> StringLiteral
%type <jlit> JsonLiteral
%type <adat> AnnotationName AnnotationData
%type <inst> Instruction AnnotInstr
%type <bun>  SLParInstrList CBParInstrList
%type <map>  Mapping
%type <sub>  Subcircuit
%type <stmt> Statement AnnotStatement
%type <stms> StatementList
%type <vers> Version
%type <prog> Program

/* Whitespace management */
%token NEWLINE

/* Version statement */
%token VERSION

/* Keywords */
%token QUBITS
%token MAP
%token CDASH

/* Numeric literals */
%token <str> INT_LITERAL
%token <str> FLOAT_LITERAL
%token BAD_NUMBER

/* String and JSON literals */
%token STRING_OPEN STRING_CLOSE
%token JSON_OPEN JSON_CLOSE
%token <str> STRBUILD_APPEND STRBUILD_ESCAPE

/* Matrix literals */
%token MAT_OPEN MAT_CLOSE

/* Identifiers */
%token <str> IDENTIFIER

/* Illegal character */
%token BAD_CHARACTER

/* Associativity rules for static expressions. The lowest precedence level
comes first. NOTE: expression precedence must match the values in
operators.[ch]pp for correct pretty-printing! */
%left ',' ':'                                /* SIMD/SGMQ indexation */
%left '+' '-'                                /* Addition/subtraction */
%left '*' '/'                                /* Multiplication/division */
%right UPLUS UMINUS                          /* Unaries */

/* In a single-line parallel statement, possibly containing only a single gate,
annotations apply to the gate, not the bundle. Therefore '@' has greater
priority than '|' */
%left '|'
%left '@'
%nonassoc BUNDLE

/* Misc. Yacc directives */
%error-verbose
%start Root

%%

/* One or more newlines. */
Newline         : Newline NEWLINE
                | NEWLINE
                ;

/* Zero or more newlines. */
OptNewline      : Newline
                |
                ;

/* Integer literals. */
IntegerLiteral  : INT_LITERAL                                                   { $$ = new IntegerLiteral(); $$->value = std::strtol($1, nullptr, 0); std::free($1); }
                ;

/* Floating point literals. */
FloatLiteral    : FLOAT_LITERAL                                                 { $$ = new FloatLiteral(); $$->value = std::strtod($1, nullptr); std::free($1); }
                ;

/* Identifiers. */
Identifier      : IDENTIFIER                                                    { $$ = new Identifier(); $$->name = std::string($1); std::free($1); }
                ;

/* Old matrix syntax, specified as a row-major flattened list of the
real/imaginary value pairs.*/
MatrixLiteral1  : '[' ExpressionList ']'                                        { $$ = new MatrixLiteral1(); $$->pairs.set_raw($2); }
                ;

/* Mew matrix syntax with explitic structure and using expressions for
representing the complex numbers. */
MatrixLiteral2c : MatrixLiteral2c Newline ExpressionList                        { $$ = $1; $$->rows.add_raw($3); }
                | ExpressionList                                                { $$ = new MatrixLiteral2(); $$->rows.add_raw($1); }
                ;

MatrixLiteral2  : MAT_OPEN OptNewline MatrixLiteral2c OptNewline MAT_CLOSE      { $$ = $3; }
                ;

/* Either matrix syntax. */
MatrixLiteral   : MatrixLiteral1                                                { $$ = $1; }
                | MatrixLiteral2                                                { $$ = $1; }
                ;

/* String builder. This accumulates JSON/String data, mostly
character-by-character. */
StringBuilder   : StringBuilder STRBUILD_APPEND                                 { $$ = $1; $$->push_string(std::string($2)); std::free($2); }
                | StringBuilder STRBUILD_ESCAPE                                 { $$ = $1; $$->push_escape(std::string($2)); std::free($2); }
                |                                                               { $$ = new StringBuilder(); }
                ;

/* String literal. */
StringLiteral   : STRING_OPEN StringBuilder STRING_CLOSE                        { $$ = new StringLiteral(); $$->value = $2->stream.str(); delete $2; }
                ;

/* JSON literal. */
JsonLiteral     : JSON_OPEN StringBuilder JSON_CLOSE                            { $$ = new JsonLiteral(); $$->value = $2->stream.str(); delete $2; }
                ;

/* Array/register indexation. */
Index           : Expression '[' IndexList ']'                                  { $$ = new Index(); $$->expr.set_raw($1); $$->indices.set_raw($3); }
                ;

/* Math operations, evaluated by the parser. */
UnaryOp         : '-' Expression %prec UMINUS                                   { $$ = new Negate(); $$->expr.set_raw($2); }
                ;

BinaryOp        : Expression '*' Expression                                     { $$ = new Multiply(); $$->lhs.set_raw($1); $$->rhs.set_raw($3); }
                | Expression '+' Expression                                     { $$ = new Add();      $$->lhs.set_raw($1); $$->rhs.set_raw($3); }
                | Expression '-' Expression                                     { $$ = new Subtract(); $$->lhs.set_raw($1); $$->rhs.set_raw($3); }
                ;

/* Supported types of expressions. */
Expression      : IntegerLiteral                                                { $$ = $1; }
                | FloatLiteral                                                  { $$ = $1; }
                | Identifier                                                    { $$ = $1; }
                | MatrixLiteral                                                 { $$ = $1; }
                | StringLiteral                                                 { $$ = $1; }
                | JsonLiteral                                                   { $$ = $1; }
                | Index                                                         { $$ = $1; }
                | '(' Expression ')'                                            { $$ = $2; }
                | '+' Expression %prec UPLUS                                    { $$ = $2; }
                | UnaryOp                                                       { $$ = $1; }
                | BinaryOp                                                      { $$ = $1; }
                | error                                                         { $$ = new ErroneousExpression(); }
                ;

/* List of one or more expressions. */
ExpressionList  : ExpressionList ',' Expression                                 { $$ = $1; $$->items.add_raw($3); }
                | Expression %prec ','                                          { $$ = new ExpressionList(); $$->items.add_raw($1); }
                ;

/* Indexation modes. */
IndexItem       : Expression                                                    { $$ = new IndexItem(); $$->index.set_raw($1); }
                ;

IndexRange      : Expression ':' Expression                                     { $$ = new IndexRange(); $$->first.set_raw($1); $$->last.set_raw($3); }
                ;

IndexEntry      : IndexItem                                                     { $$ = $1; }
                | IndexRange                                                    { $$ = $1; }
                ;

IndexList       : IndexList ',' IndexEntry                                      { $$ = $1; $$->items.add_raw($3); }
                | IndexEntry                                                    { $$ = new IndexList(); $$->items.add_raw($1); }
                ;

/* The information caried by an annotation or pragma statement. */
AnnotationName  : Identifier '.' Identifier                                     { $$ = new AnnotationData(); $$->interface.set_raw($1); $$->operation.set_raw($1); }
                ;

AnnotationData  : AnnotationName                                                { $$ = $1; }
                | AnnotationName '(' ')'                                        { $$ = $1; }
                | AnnotationName '(' ExpressionList ')'                         { $$ = $1; $$->operands.set_raw($3); }
                ;

/* Instructions. Note that this is NOT directly a statement grammatically;
they are always part of a bundle. */
Instruction     : Identifier                                                    { $$ = new Instruction(); $$->name.set_raw($1); $$->operands.set_raw(new ExpressionList()); }
                | Identifier ExpressionList                                     { $$ = new Instruction(); $$->name.set_raw($1); $$->operands.set_raw($2); }
                | CDASH Identifier Expression                                   { $$ = new Instruction(); $$->name.set_raw($2); $$->condition.set_raw($3); $$->operands.set_raw(new ExpressionList()); }
                | CDASH Identifier Expression ',' ExpressionList                { $$ = new Instruction(); $$->name.set_raw($2); $$->condition.set_raw($3); $$->operands.set_raw($5); }
                ;

/* Instructions are not statements (because there can be multiple bundled
instructions per statement) but can be annotated, so they need their own
annotation rule. */
AnnotInstr      : AnnotInstr '@' AnnotationData                                 { $$ = $1; $$->annotations.add_raw($3); }
                | Instruction                                                   { $$ = $1; }
                ;

/* Single-line bundling syntax. */
SLParInstrList  : SLParInstrList '|' AnnotInstr                                 { $$ = $1; $$->items.add_raw($3); }
                | AnnotInstr %prec '|'                                          { $$ = new Bundle(); $$->items.add_raw($1); }
                ;

/* Multi-line bundling syntax. */
CBParInstrList  : CBParInstrList Newline SLParInstrList                         { $$ = $1; $$->items.extend($3->items); delete $3; }
                | SLParInstrList                                                { $$ = $1; }
                ;

/* Map statement, aliasing some expression with an identifier. */
Mapping         : MAP Expression ',' Identifier                                 { $$ = new Mapping(); $$->expr.set_raw($2); $$->alias.set_raw($4); }
                | MAP Identifier '=' Expression                                 { $$ = new Mapping(); $$->alias.set_raw($2); $$->expr.set_raw($4); }
                ;

/* Subcircuit header statement. */
Subcircuit      : '.' Identifier                                                { $$ = new Subcircuit(); $$->name.set_raw($2); }
                | '.' Identifier '(' Expression ')'                             { $$ = new Subcircuit(); $$->name.set_raw($2); $$->iterations.set_raw($4); }
                ;

/* Any of the supported statements. */
Statement       : Mapping                                                       { $$ = $1; }
                | Subcircuit                                                    { $$ = $1; }
                | SLParInstrList                                                { $$ = $1; }
                | '{' OptNewline CBParInstrList OptNewline '}'                  { $$ = $3; }
                | error                                                         { $$ = new ErroneousStatement(); }
                ;

/* Statement with annotations attached to it. */
AnnotStatement  : AnnotStatement '@' AnnotationData                             { $$ = $1; $$->annotations.add_raw($3); }
                | Statement                                                     { $$ = $1; }
                ;

/* List of one or more statements. */
StatementList   : StatementList Newline AnnotStatement                          { $$ = $1; $$->items.add_raw($3); }
                | AnnotStatement                                                { $$ = new StatementList(); $$->items.add_raw($1); }
                ;

/* Version. */
Version         : Version '.' IntegerLiteral                                    { $$ = $1; $$->items.push_back($3->value); delete $3; }
                | IntegerLiteral                                                { $$ = new Version(); $$->items.push_back($1->value); delete $1; }
                ;

/* Program. */
Program         : OptNewline VERSION Version Newline
                    QUBITS Expression Newline
                    StatementList OptNewline                                    { $$ = new Program(); $$->version.set_raw($3); $$->num_qubits.set_raw($6); $$->statements.set_raw($8); }
                | OptNewline VERSION Version Newline
                    QUBITS Expression OptNewline                                { $$ = new Program(); $$->version.set_raw($3); $$->num_qubits.set_raw($6); }
                ;

/* Toplevel. */
Root            : Program                                                       { analyzer.result.ast_root.set_raw($1); }
                | error                                                         { analyzer.result.ast_root.set_raw(new ErroneousProgram()); }
                ;

%%

void yyerror(YYLTYPE* yyllocp, yyscan_t unused, cqasm::AnalyzerInternals &analyzer, const char* msg) {
    std::ostringstream sb;
    sb << analyzer.result.filename
       << ":"  << yyllocp->first_line
       << ":"  << yyllocp->first_column
       << ": " << msg;
    analyzer.push_error(sb.str());
}
