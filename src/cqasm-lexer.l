%option noinput nounput noyywrap 8bit nodefault
%option yylineno
%option reentrant bison-bridge bison-locations

%{
/*     #include "cqasm2/ast/ast.hpp" */
    #include "cqasm-parser.hpp"
    #define YY_USER_ACTION yylloc->last_column += yyleng;
    #define LOCATION_RESET \
        yylloc->first_column = 1; yylloc->first_line = 1; \
        yylloc->last_column = 1; yylloc->last_line = 1;
    #define LOCATION_LINE \
        yylloc->last_column = 1; yylloc->last_line += 1;
    #define LOCATION_STEP \
        yylloc->first_column = yylloc->last_column; \
        yylloc->first_line = yylloc->last_line;
    #define WITH_STR(TOKNAME) \
        /*printf("Pushing %s token starting at %d:%d\n", #TOKNAME, yylloc->first_line, yylloc->first_column);*/ \
        yylval->str = strdup(yytext); return TOKNAME
    #define WITHOUT_STR(TOKNAME) \
        /*printf("Pushing %s token starting at %d:%d\n", #TOKNAME, yylloc->first_line, yylloc->first_column);*/ \
        return TOKNAME
    #define OPERATOR \
        /*printf("Pushing %s token starting at %d:%d\n", yytext, yylloc->first_line, yylloc->first_column);*/ \
        return yytext[0]
    #define DELETE LOCATION_STEP
%}

%x MULTILINE_COMMENT
%x VERSION
%x STRING
%x JSON
%x JSONSTRING
%%

%{
    LOCATION_STEP;
%}

<INITIAL>{


    /*=======================================================================*/
    /* WHITESPACE MANAGEMENT                                                 */
    /*=======================================================================*/

    /* Newline and single-line comment */
(\n|\r\n?)                                          LOCATION_LINE; WITHOUT_STR(NEWLINE);

    /* Semicolons function exactly the same way as newlines */
;                                                   WITHOUT_STR(NEWLINE);

    /* And then there's the opposite, the escaped newline */
\\(\n|\r\n?)                                        LOCATION_LINE; DELETE;

    /* Gobble up comments */
    /* TODO: error on EOF within comment mode */
#.*                                                 DELETE;
"/*"                                                BEGIN(MULTILINE_COMMENT); DELETE;
}
<MULTILINE_COMMENT>{
"*/"                                                BEGIN(INITIAL); DELETE;
(\n|\r\n?)                                          LOCATION_LINE; DELETE;
.                                                   DELETE;
}
<INITIAL>{


    /* Gobble up whitespace */
[ \t]+                                              DELETE;


    /*=======================================================================*/
    /* VERSION STATEMENT                                                     */
    /*=======================================================================*/

    /* This is treated separately to avoid conflicts with float literals */
(?i:version)                                        BEGIN(VERSION); WITHOUT_STR(VERSION);
}
<VERSION>{
(\n|\r\n?)                                          LOCATION_LINE; BEGIN(INITIAL); WITHOUT_STR(NEWLINE);
[ \t]+                                              DELETE;
[0-9]+                                              WITH_STR(INT_LITERAL);
\.                                                  OPERATOR;
.                                                   WITHOUT_STR(BAD_CHARACTER);
}
<INITIAL>{

    /*=======================================================================*/
    /* KEYWORDS                                                              */
    /*=======================================================================*/

    /* Qubit allocation */
(?i:qubits)                                         WITHOUT_STR(QUBITS);

    /* Qubit register naming */
(?i:map)                                            WITHOUT_STR(MAP);

    /* C-dash prefix for binary-controlled gates */
[cC]-/[a-zA-Z_]                                     WITHOUT_STR(CDASH);


    /*=======================================================================*/
    /* NUMERIC LITERALS                                                      */
    /*=======================================================================*/

    /* Integer literals */
[0-9]+                                              WITH_STR(INT_LITERAL);

    /* Float literals */
[0-9]*\.[0-9]+([eE][-+]?[0-9]+)?                    WITH_STR(FLOAT_LITERAL);

    /* Any digit followed by a letter not detected by the above is an invalid number */
[0-9][a-zA-Z0-9_]                                   WITHOUT_STR(BAD_NUMBER);


    /*=======================================================================*/
    /* STRING LITERALS                                                       */
    /*=======================================================================*/

["]                                                 BEGIN(STRING); WITHOUT_STR(STRING_OPEN);
}
<STRING>{
["]                                                 BEGIN(INITIAL); WITHOUT_STR(STRING_CLOSE);
[\\][tn'"\\]                                        WITH_STR(STRBUILD_ESCAPE);
[\\](\n|\r\n?)                                      LOCATION_LINE; DELETE;
(\n|\r\n?)                                          LOCATION_LINE; WITH_STR(STRBUILD_APPEND);
.                                                   WITH_STR(STRBUILD_APPEND);
}
<INITIAL>{


    /*=======================================================================*/
    /* JSON LITERALS                                                         */
    /*=======================================================================*/

"{|"                                                BEGIN(JSON); WITHOUT_STR(JSON_OPEN);
}
<JSON>{
"|}"                                                BEGIN(INITIAL); WITHOUT_STR(JSON_CLOSE);
["]                                                 BEGIN(JSONSTRING); WITH_STR(STRBUILD_APPEND);
(\n|\r\n?)                                          LOCATION_LINE; WITH_STR(STRBUILD_APPEND);
.                                                   WITH_STR(STRBUILD_APPEND);
}
<JSONSTRING>{
["]                                                 BEGIN(JSON); WITH_STR(STRBUILD_APPEND);
[\\].                                               WITH_STR(STRBUILD_APPEND);
(\n|\r\n?)                                          LOCATION_LINE; WITH_STR(STRBUILD_APPEND);
.                                                   WITH_STR(STRBUILD_APPEND);
}
<INITIAL>{


    /*=======================================================================*/
    /* MATRIX LITERALS                                                       */
    /*=======================================================================*/

"[|"                                                WITHOUT_STR(MATRIX_OPEN);
"|]"                                                WITHOUT_STR(MATRIX_CLOSE);


    /*=======================================================================*/
    /* IDENTIFIERS                                                           */
    /*=======================================================================*/

[a-zA-Z_][a-zA-Z0-9_]*                              WITH_STR(IDENTIFIER);


    /*=======================================================================*/
    /* OPERATORS                                                             */
    /*=======================================================================*/

[()[\]{}+\-*:,.@]                                   OPERATOR;


    /*=======================================================================*/
    /* DEFAULT                                                               */
    /*=======================================================================*/

    /* Catch any other character as a single-character lexeme */
.                                                   WITHOUT_STR(BAD_CHARACTER);


}
