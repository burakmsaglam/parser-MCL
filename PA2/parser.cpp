//
//  parser.cpp
//  PA2
//
//  Created by Burak Mert Saglam on 11/14/24.
//


#include <iostream>
#include <fstream>
#include "lex.h"
#include "parser.h"

using namespace std;

int nestLevel = 0;

map<string, bool> trackingVar;

// Given code start
namespace Parser {
    bool pushed_back = false;
    LexItem pushed_token;
    
    // Getting next token
    static LexItem GetNextToken(istream & in, int& line) {
        if(pushed_back) {
            pushed_back = false;
            return pushed_token;
        }
        return getNextToken(in, line);
    }
    
    // Trying to push token back
    static void PushBackToken(LexItem & t) {
        // Pushing more than once gives error.
        if (pushed_back) {
            abort();
        }
        pushed_back = true;
        pushed_token = t;
    }
}
// end

// Counting errors, statics cause it needs to be increased overall everywhere.
static int error_c = 0;

// When called return the error count
int ErrCount() 
{
    return error_c;
}

// Increasing the error
void ParseErMessage(int line, string message) 
{
    ++error_c;
    cout << line << ": " << message << endl;
}

// Declaring variable function
bool declareVar(const string& varName, int line) 
{
    // if found, means declared, find method returns map.end if not found
    if(trackingVar.find(varName) != trackingVar.end()) 
    {
        // Necessary couts for case 2
        ParseErMessage(line, "Variable Redefinition");
        ParseErMessage(line, "Incorrect variable in Declaration Statement.");
        ParseErMessage(line, "Invalid declaration statement.");
        return false;
    }
    // if not found then declare it.
    trackingVar[varName] = true;
    return true;
}

// Initializing variable in the table to mark the variable
void initializeVar(string & varName) 
{
    trackingVar[varName] = true;
}

// Checking if a variable is available and initialized
bool varAvailable(string & varName, int line) 
{
    // If variable name not found in trackingVar container map where all declared variable are.
    if(trackingVar.find(varName) == trackingVar.end()) 
    {
        ParseErMessage(line, "Undeclared Variable");
        return false;
    }
    // Check if the variable is initialized
    if(!trackingVar.find(varName)->second) 
    {
        cout << trackingVar.find(varName)->second;
        ParseErMessage(line, "Uninitialized variable: " + varName);
        return false;
    }
    return true;
}

// Controlling Indentation and program structure
bool Prog(istream& in, int& line) 
{
    // Get the next token.
    LexItem tok = Parser::GetNextToken(in, line);
    // Need t
    if(tok != PROGRAM) 
    {
        ParseErMessage(line, "Missing PROGRAM keyword");
        return false;
    }
    
    tok = Parser::GetNextToken(in, line);
    if(tok != IDENT) 
    {
        ParseErMessage(line, "Missing program name.");
        return false;
    }
    
    // checking left brace before entering compstmt
    tok = Parser::GetNextToken(in, line);
    if(tok != LBRACE)
    {
        ParseErMessage(line, "Invalid Program");
        return false;
    }
    // push back if its ok.
    Parser::PushBackToken(tok);
    
    // Checking structure starting the parsing.
    if(!CompStmt(in, line)) 
    {
        ParseErMessage(line, "Invalid Program");
        return false;
    }
    // If not fail in any case.
    cout << "(DONE)" << endl;
    return true;
}
    

// Compound statement
bool CompStmt(istream& in, int& line) 
{
    LexItem tok = Parser::GetNextToken(in, line);
    // If not Lbrace
    if(tok != LBRACE) {
        ParseErMessage(line, "Missing Left Brace.");
        return false;
    }
    // Expecting stmt
    if(!StmtList(in, line)) {
        ParseErMessage(line, "Incorrect statement list");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    // If not closed right
    if(tok != RBRACE)
    {
        cout << "Compstmt Current tok is : " << tok;
        ParseErMessage(line, "Missing Right Brace.");
        return false;
    }
    return true;
}

bool StmtList(istream& in, int& line) {
    while (true) 
    {
        // Parse the current statement
        if (!Stmt(in, line)) 
        {
            ParseErMessage(line, "Syntactic error in statement list.");
            return false;
        }
        // Get the next token to check what comes after the statement
        LexItem tok = Parser::GetNextToken(in, line);

        // If a semicolon is found, continue to the next statement
        if (tok == SEMICOL) continue;

        // Handle type specifiers -- for declarations
        if (tok == INT || tok == FLOAT || tok == BOOL || tok == CHAR || tok == STRING)
        {
            Parser::PushBackToken(tok); // Push back type token for DeclStmt
            continue;
        }

        // Handle control statements or assignments
        if (tok == LBRACE || tok == IF || tok == PRINT || tok == IDENT) 
        {
            Parser::PushBackToken(tok); // Push back token for further processing in Stmt
            continue;
        }

        // If a closing brace is found, end the statement list
        if (tok == RBRACE) {
            Parser::PushBackToken(tok); // Push back the brace for Compound Statement handling
            break;
        }
        // If conditions not met, give error
        ParseErMessage(line, "Unexpected token in statement list.");
        return false;
    }
    return true;
}

bool Stmt(istream& in, int& line) 
{
    LexItem tok = Parser::GetNextToken(in, line);
    // Getting all conditions
    switch(tok.GetToken()) 
    {
        case INT: case FLOAT: case BOOL: case CHAR: case STRING:
            // If decl statement returns false return
            if (!DeclStmt(in, line)) return false;
            return true;
        case IF:
            if (!IfStmt(in, line)) 
            {
                ParseErMessage(line, "Invalid control statement.");
                return false;
            }
            return true;
        case PRINT:
            if (!PrintStmt(in, line)) 
            {
                ParseErMessage(line, "Invalid control statement.");
                return false;
            }
            return true;
        case IDENT:
            Parser::PushBackToken(tok);
            if (!AssignStmt(in, line)) 
            {
                ParseErMessage(line, "Invalid control statement.");
                return false;
            }
            return true;
        case RBRACE:
            Parser::PushBackToken(tok);
            return true;
        case ELSE:
//            ParseErMessage(line, "Invalid control statement."); else is wrong but no error message from here
            return false;
        default:
            ParseErMessage(line, "Invalid control statement.");
            return false;
    }
}

// Declaration Statement
bool DeclStmt(istream& in, int& line) 
{
    // Process the variable list
    if (!VarList(in, line)) return false;
    
    // Expect a semicolon after the variable list
    LexItem tok = Parser::GetNextToken(in, line);
    if (tok != SEMICOL) 
    {
        ParseErMessage(line, "Missing semicolon at the end of the declaration statement.");
        return false;
    }

    return true; // Successfully parsed declaration statement
}

bool VarList(istream& in, int& line) 
{
    LexItem tok = Parser::GetNextToken(in, line);
    // Need IDENT, grammar rule
    if (tok != IDENT)
    {
        ParseErMessage(line, "Identifier is missing.");
        return false;
    }

    // Checking if var declared with special func if not declare it
    string varName = tok.GetLexeme();
    if (!declareVar(varName, line)) return false;

    // Check for assignment operator
    tok = Parser::GetNextToken(in, line);
    if (tok == ASSOP) 
    {
        // Need expr -- grammar rule
        if (!Expr(in, line))
        {
            ParseErMessage(line, "Missing expression after assignment operator!");
            return false;
        }
        // Mark the variable as initialized
        initializeVar(varName);
        cout << "Initialization of the variable " << varName << " in the declaration statement at line " << line << endl;
    }
    // if not assignment op
    else Parser::PushBackToken(tok);
    
    // get next op to check comma & semi col
    tok = Parser::GetNextToken(in, line);
    if (tok == COMMA)
        return VarList(in, line); // Continue parsing additional variables recursively
    // end of statement if semicol
    else if (tok == SEMICOL)
    {
        Parser::PushBackToken(tok); // Let DeclStmt handle the semicolon
        return true;
    }
    // If not end or not semi col then there should be error
    else
    {
        ParseErMessage(line, "Missing comma in variable decleration.");
        return false;
    }
}

// Assignment Statement
bool AssignStmt(istream& in, int& line) {
    LexItem tok = Parser::GetNextToken(in, line);
    if(tok != IDENT)
    {
        ParseErMessage(line, "Missing identifier in assignment statement.");
        return false;
    }

    string varName = tok.GetLexeme();
    if (!varAvailable(varName, line))
    {// add missing messages Undeclared variable comes from function
        ParseErMessage(line, "Missing Left-Hand Side Variable in Assignment Statement.");
        ParseErMessage(line, "Incorrect Assignment Statement.");
        return false;
    }
    tok = Parser::GetNextToken(in, line);
    // Missing assign operator
    if(tok != ASSOP)
    {
        ParseErMessage(line, "Missing Assignment Operator");
        ParseErMessage(line, "Incorrect Assignment Statement.");
        return false;
    }
    else if(tok == ASSOP)
    {
        if (!Expr(in, line)) 
        {
            ParseErMessage(line, "Missing Expression in Assignment Statement");
            ParseErMessage(line, "Incorrect Assignment Statement.");
            return false;
        }
        // Checking semi colon
        tok = Parser::GetNextToken(in, line);
        if (tok != SEMICOL)
        {
            --line;
            ParseErMessage(line, "Missing semicolon at end of Statement.");
            return false;
        }
        // Mark the variable as initialized after successful assignment
        initializeVar(varName);
        return true;
    }

    ParseErMessage(line, "Invalid assignment operator.");
    return false;
}

bool IfStmt(istream& in, int& line) 
{
    nestLevel++; // Increment nesting level for each if stat
    
    // Expect '(' after 'IF'
    LexItem tok = Parser::GetNextToken(in, line);
    if (tok != LPAREN) 
    {
        ParseErMessage(line, "Missing left parenthesis in If statement.");
        return false;
    }

    // Parse the condition inside the parentheses
    if (!Expr(in, line)) {
        ParseErMessage(line, "Missing if statement condition.");
        ParseErMessage(line, "Incorrect IF Statement.");
        return false;
    }

    // Expect ')' to close the condition
    tok = Parser::GetNextToken(in, line);
    if (tok != RPAREN) 
    {
        ParseErMessage(line, "Missing right parenthesis in If statement.");
        return false;
    }

    // Parse the 'THEN' clause (single statement or compound statement)
    cout << "in IfStmt then-clause at nesting level: " << nestLevel << endl;
    
    // Getting next token to check brace
    tok = Parser::GetNextToken(in, line);
    if (tok == LBRACE) 
    {
        // If '{' is found, process a compound statement
        Parser::PushBackToken(tok); // Push back to let CompStmt handle it
        if (!CompStmt(in, line))
        {
            ParseErMessage(line, "Invalid compound statement in If statement.");
            return false;
        }
    }
    else
    {
        // Otherwise, process a single statement
        Parser::PushBackToken(tok); // Push back to let Stmt handle it
        if (!Stmt(in, line))
        {
            ParseErMessage(line, "Incorrect IF Statement.");
            return false;
        }
    }

    // Get new token to checck for optional ELSE clause
    tok = Parser::GetNextToken(in, line);
    if (tok == ELSE) 
    {
        // Give where you are
        cout << "in IfStmt else-clause at nesting level: " << nestLevel << endl;

        tok = Parser::GetNextToken(in, line);
        if(tok == LBRACE)
        {
            // If '{' is found, process a compound statement
            Parser::PushBackToken(tok); // Push back to let CompStmt handle it
            if (!CompStmt(in, line)) 
            {
                ParseErMessage(line, "Invalid compound statement in ELSE clause.");
                return false;
            }
        } 
        else
        {
            // Otherwise, process a single statement
            Parser::PushBackToken(tok); // Push back to let Stmt handle it
            if (!Stmt(in, line)) 
            {
                ParseErMessage(line, "Missing statement for ELSE.");
                return false;
            }
        }
    }
    else Parser::PushBackToken(tok); // If no ELSE clause, push back the token
    nestLevel--; // Decrement nesting level after processing
    return true;
}

bool PrintStmt(istream& in, int& line) 
{
    LexItem tok = Parser::GetNextToken(in, line);
    // Left Parenthesis
    if(tok != LPAREN) {
        ParseErMessage(line, "Missing Left Parenthesis in Print statement.");
        ParseErMessage(line, "Incorrect Print Statement.");
        return false;
    }
    
    bool ex = ExprList(in, line);
    if(!ex) 
    {
        ParseErMessage(line, "Missing expression after Print.");
        return false;
    }
    
    tok = Parser::GetNextToken(in, line);
    // Right Parenthesis
    if(tok != RPAREN) {
        ParseErMessage(line, "Missing Right Parenthesis in Print statement.");
        return false;
    }
    return true;
}

bool ExprList(istream& in, int& line) 
{
    // Parse the first expression in the list
    bool status = Expr(in, line);
    if(!status)
    {
        ParseErMessage(line, "Missing Expression in expression list.");
        return false;
    }

    LexItem tok = Parser::GetNextToken(in, line);

    // Handle cases where there is a missing comma or invalid token
    while(tok == COMMA)
    {
        status = Expr(in, line); // Parse the next expression
        if (!status) 
        {
            ParseErMessage(line, "Missing Expression after comma in expression list.");
            return false;
        }
        tok = Parser::GetNextToken(in, line); // Check for more commas or RPAREN
    }

    // If we encounter an unexpected token (other than RPAREN), it indicates an error
    if (tok != RPAREN) 
    {
        ParseErMessage(line, "Missing comma in expression list or invalid token.");
        return false;
    }
    
    // Push back the RPAREN for the calling function to handle
    Parser::PushBackToken(tok);
    return true;
}

// Logical OR Expression
bool Expr(istream& in, int& line) 
{
    if(!LogANDExpr(in, line)) return false;
    
    LexItem tok = Parser::GetNextToken(in, line);
    // Handle OR operators
    while (tok == OR) 
    {
        if (!LogANDExpr(in, line)) 
        {
            ParseErMessage(line, "Missing operand after || operator.");
            return false;
        }
        tok = Parser::GetNextToken(in, line);
    }
    // Push back token that is not OR
    Parser::PushBackToken(tok);
    return true;
}

// Logical AND Expression
bool LogANDExpr(istream& in, int& line) 
{
    if (!EqualExpr(in, line)) return false;
    // Get the next token
    LexItem tok = Parser::GetNextToken(in, line);
    
    // Handling AND operators
    while(tok == AND)
    {
        if (!EqualExpr(in, line))
        {
            ParseErMessage(line, "Missing operand after && operator.");
            return false;
        }
        tok = Parser::GetNextToken(in, line);
    }
    Parser::PushBackToken(tok); // Push back the last token
    return true;
}

// Equality Expression
bool EqualExpr(istream& in, int& line) 
{
    if(!RelExpr(in, line)) return false;
    LexItem tok = Parser::GetNextToken(in, line);
    // If eq or not equal
    if(tok == EQ || tok == NEQ)
    {
        // Checking relexpr
        if(!RelExpr(in, line))
        {
            ParseErMessage(line, "Missing operand after equality operator.");
            return false;
        }
        
        // Looking head for another eq
        LexItem NextTok = Parser::GetNextToken(in, line);
        if(NextTok == EQ || NextTok == NEQ)
        {
            ParseErMessage(line, "Illegal Equality Expression.");
            Parser::PushBackToken(NextTok);
            return false;
        }
        // if not put it back.
        Parser::PushBackToken(NextTok);
    }
    // Push back for further evaluation
    else Parser::PushBackToken(tok);
    return true;
}


// RelExpr ::= AddExpr {(< | >) AddExpr}
 
bool RelExpr(istream& in, int& line) 
{
    if(!AddExpr(in, line)) return false;
    
    
    LexItem tok = Parser::GetNextToken(in, line);
    if(tok == LTHAN || tok == GTHAN) 
    {
        if(!AddExpr(in, line)) 
        {
            ParseErMessage(line, "Missing operand after relational operator");
            return false;
        }
        
        tok = Parser::GetNextToken(in, line);
        if(tok == LTHAN || tok == GTHAN) 
        {
            ParseErMessage(line, "Illegal Relational Expression.");
            return false;
        }
        Parser::PushBackToken(tok);
    }
    else Parser::PushBackToken(tok);
    return true;
}

// Addition and Subtraction Expression
bool AddExpr(istream& in, int& line) 
{
    if(!MultExpr(in, line)) return false;
    
    LexItem tok = Parser::GetNextToken(in, line);
    if(tok == PLUS || tok == MINUS) 
    {
        if(!MultExpr(in, line)) 
        {
            ParseErMessage(line, "Missing operand after addition operator.");
            return false;
        }
    } else
        Parser::PushBackToken(tok);
    
    return true;
}

// Multiplication, Division, and Modulus Expression
bool MultExpr(istream& in, int& line) 
{
    if(!UnaryExpr(in, line)) return false;
    
    LexItem tok = Parser::GetNextToken(in, line);
    while (tok == MULT || tok == DIV || tok == REM) 
    {
        if(!UnaryExpr(in, line)) 
        {
            ParseErMessage(line, "Missing operand after operator.");
            return false;
        }
        tok = Parser::GetNextToken(in, line);
    }
    Parser::PushBackToken(tok);
    return true;
}

// Unary Expression
bool UnaryExpr(istream& in, int& line) 
{
    LexItem tok = Parser::GetNextToken(in, line);
    // If not minus, plus, or not put the token back
    if (tok != MINUS && tok != PLUS && tok != NOT)
    {
        Parser::PushBackToken(tok);
    }
    //If not primary
    if(!PrimaryExpr(in, line, tok.GetToken())) return false;
    //If any condition not met
    return true;
}

// Primary Expression
bool PrimaryExpr(istream& in, int& line, int unaryOperator) 
{
    LexItem tok = Parser::GetNextToken(in, line);
    
    if (tok == IDENT) 
    {
        string name = tok.GetLexeme();
        return varAvailable(name, line);
    }
    // If any of the following true
    else if (tok == ICONST || tok == RCONST || tok == BCONST || tok == CCONST || tok == SCONST)
        return true;
    
    // Checking paranthesis
    else if (tok == LPAREN)
    {
        if (!Expr(in, line)) 
        {
            ParseErMessage(line, "Missing expression after Left Parenthesis");
            return false;
        }
        tok = Parser::GetNextToken(in, line);
        if (tok != RPAREN) 
        {
            ParseErMessage(line, "Missing Right Parenthesis");
            return false;
        }
        return true;
    }
    // If non of specified conditions met return false
    else return false;
    
}
