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

int nest_level = 0;

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

// Counting errors
static int error_count = 0;

// When called return the error count
int ErrCount() {
    return error_count;
}

// Increasing the error
void ParseErMessage(int line, string message) {
    ++error_count;
    cout << line << ": " << message << endl;
}

// Declaring variable function
bool declareVar(const string& varName, int line) {
    // if found, means declared, find method returns map.end if not found
    if(trackingVar.find(varName) != trackingVar.end()) {
        ParseErMessage(line, "Variable named : " + varName + " already declared");
        return false;
    }
    // if not found then declare it.
    trackingVar[varName] = false; // not initialized
    return true;
}

// Initializing variable in the table
void initializeVar(string & varName) {
    trackingVar[varName] = true;
}

// Checking if a variable is available and initialized
bool varAvailable(string & varName, int line) {
    // If variable name not found in trackingVar container map where all declared variable are.
    if(trackingVar.find(varName) == trackingVar.end()) {
        ParseErMessage(line, "Undeclared Variable");
        return false;
    }
    // Check if the variable is initialized
    if(!trackingVar.find(varName)->second) {
        ParseErMessage(line, "Uninitialized variable: " + varName);
        return false;
    }
    return true;
}

// Controlling Indentation
bool Prog(istream& in, int& line) {
    LexItem tok = Parser::GetNextToken(in, line);
    if(tok != PROGRAM) {
        ParseErMessage(line, "Missing PROGRAM keyword");
        return false;
    }
    
    tok = Parser::GetNextToken(in, line);
    if(tok != IDENT) {
        ParseErMessage(line, "Missing program name.");
        return false;
    }
    // Checking structure
    if(!CompStmt(in, line)) {
        ParseErMessage(line, "Invalid Program");
        return false;
    }
    cout << "(DONE)" << endl;
    return true;
}

// Compound statement
bool CompStmt(istream& in, int& line) {
    LexItem tok = Parser::GetNextToken(in, line);
    if(tok != LBRACE) {
        ParseErMessage(line, "Missing Left Brace.");
        return false;
    }
    
    if(!StmtList(in, line)) {
        ParseErMessage(line, "Incorrect statement list.");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    if(tok != RBRACE) {
        ParseErMessage(line, "Missing Right Brace.");
        return false;
    }
    return true;
}

bool StmtList(istream& in, int& line) {
    while(true) {
        if(!Stmt(in, line)) {
            ParseErMessage(line, "Syntactic error in statement list.");
            return false;
        }
        
        LexItem tok = Parser::GetNextToken(in, line);
        if(tok != SEMICOL && tok != LBRACE && tok != IF && tok != PRINT && tok != IDENT) {
            Parser::PushBackToken(tok);
            break;
        }
        Parser::PushBackToken(tok);
    }
    return true;
}

bool Stmt(istream& in, int& line) {
    // Get the next token
    LexItem tok = Parser::GetNextToken(in, line);
    // Checking statement type and if really is statement.
    switch(tok.GetToken()) {
        // Decl
        case INT: case FLOAT: case BOOL: case CHAR: case STRING:
            return DeclStmt(in, line);
        
        // Control - if or print
        case IF:
            return IfStmt(in, line);
        case PRINT:
            return PrintStmt(in, line);

        // Assignment
        case IDENT:
            // Push back token to check in the function
            Parser::PushBackToken(tok);
            return AssignStmt(in, line);
            
        // Not any one of them
        default:
            ParseErMessage(line, "Invalid Statement.");
            return false;
    }
}

// Declaration Statement
bool DeclStmt(istream& in, int& line) {
    // Checking VarList
    if (!VarList(in, line)) {
        ParseErMessage(line, "Incorrect variable list.");
        return false;
    }
    
    LexItem tok = Parser::GetNextToken(in, line);
    if(tok != SEMICOL) {
        ParseErMessage(line, "Missing semicolon");
        return false;
    }
    cout << "Initialization of the variable " << tok.GetLexeme() << " in the declaration statement at line " << line << endl;
    return true;
}

bool VarList(istream& in, int& line) {
    LexItem tok = Parser::GetNextToken(in, line);
    if(tok != IDENT) {
        ParseErMessage(line, "Identifier is missing.");
        return false;
    }
    
    // declaring var
    string varName = tok.GetLexeme();
    if (!declareVar(varName, line)) return false;

    // checking if there is assignment operator
    tok = Parser::GetNextToken(in, line);
    if(tok == ASSOP) {
        if(!Expr(in, line)) {
            ParseErMessage(line, "Missing expression after assignment operator!");
            return false;
        }
        initializeVar(varName);
    }

    // Check if there is a comma
    if(tok == COMMA) {
        return VarList(in, line); // recursively check for additional variables
    } else if(tok != SEMICOL) {
        Parser::PushBackToken(tok);
    }
    return true;
}

// Assignment Statement
bool AssignStmt(istream& in, int& line) {
    LexItem tok = Parser::GetNextToken(in, line);
    if(tok != IDENT) {
        ParseErMessage(line, "Missing identifier in assignment statement.");
        return false;
    }
    
    string varName = tok.GetLexeme();
    if (!varAvailable(varName, line)) return false;

    tok = Parser::GetNextToken(in, line);
    if(tok == ASSOP) {
        if(!Expr(in, line)) {
            ParseErMessage(line, "Missing Expression in Assignment Statement");
            ParseErMessage(line, "Incorrect Assignment Statement");
            return false;
        }
        initializeVar(varName);
        return true;
    } else if(tok == ADDASSOP || tok == SUBASSOP || tok == MULASSOP || tok == DIVASSOP || tok == REMASSOP) {
        if(!trackingVar[varName]) {
            ParseErMessage(line, "Variable must be initialized before using in compound assignment.");
            return false;
        }
        if(!Expr(in, line)) {
            ParseErMessage(line, "Missing expression in compound assignment.");
            return false;
        }
        return true;
    }
    ParseErMessage(line, "Invalid assignment operator.");
    return false;
}
bool IfStmt(istream& in, int& line) {
    nest_level++; // increment nesting level

    LexItem tok = Parser::GetNextToken(in, line);
    // Left Parenthesis
    if(tok != LPAREN) {
        ParseErMessage(line, "Missing left parenthesis in If statement.");
        return false;
    }
    
    // Following Expression
    if(!Expr(in, line)) {
        ParseErMessage(line, "Missing expression after if.");
        return false;
    }
    
    tok = Parser::GetNextToken(in, line);
    // Right Parenthesis
    if(tok != RPAREN) {
        ParseErMessage(line, "Missing right parenthesis in If statement.");
        return false;
    }
    cout << "In If statement then-clause at nesting level: " << nest_level << endl;
    
    // Statement in If body
    if(!Stmt(in, line)) {
        ParseErMessage(line, "Missing statement for IF.");
        return false;
    }

    tok = Parser::GetNextToken(in, line);
    // Check for optional ELSE clause
    if(tok == ELSE) {
        cout << "In If statement else-clause at nesting level: " << nest_level << endl;
        if(!Stmt(in, line)) {
            ParseErMessage(line, "Missing statement for ELSE.");
            return false;
        }
    } else {
        Parser::PushBackToken(tok);
    }

    // Parsing completed; reduce nesting level
    nest_level--;
    return true;
}

bool PrintStmt(istream& in, int& line) {
    LexItem tok = Parser::GetNextToken(in, line);
    // Left Parenthesis
    if(tok != LPAREN) {
        ParseErMessage(line, "Missing Left Parenthesis in Print statement.");
        return false;
    }
    
    bool ex = ExprList(in, line);
    if(!ex) {
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
bool ExprList(istream& in, int& line) {
    bool status = Expr(in, line);
    if(!status) {
        ParseErMessage(line, "Missing Expression in expression list.");
        return false;
    }
    
    LexItem tok = Parser::GetNextToken(in, line);
    if(tok == COMMA) {
        return ExprList(in, line);
    } else if(tok.GetToken() == ERR) {
        ParseErMessage(line, "Unrecognized Input Pattern in Expression List.");
        cout << "(" << tok.GetLexeme() << ")" << endl;
        return false;
    } else {
        Parser::PushBackToken(tok);
        return true;
    }
}

// Logical OR Expression
bool Expr(istream& in, int& line) {
    if(!LogANDExpr(in, line)) return false;
    
    LexItem tok = Parser::GetNextToken(in, line);
    // Handle OR operators
    while (tok == OR) {
        if (!LogANDExpr(in, line)) {
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
bool LogANDExpr(istream& in, int& line) {
    if (!EqualExpr(in, line)) {
        return false;
    }

    LexItem tok = Parser::GetNextToken(in, line);
    // Handle AND operators
    while (tok == AND) {
        if (!EqualExpr(in, line)) {
            ParseErMessage(line, "Missing operand after && operator.");
            return false;
        }
        tok = Parser::GetNextToken(in, line);
    }

    Parser::PushBackToken(tok); // Push back the last token
    return true;
}

// Equality Expression
bool EqualExpr(istream& in, int& line) {
    if(!RelExpr(in, line)) return false;
    
    LexItem tok = Parser::GetNextToken(in, line);
    if(tok == EQ || tok == NEQ) {
        if(!RelExpr(in, line)) {
            ParseErMessage(line, "Missing operand after equality operator.");
            return false;
        }
    } else {
        Parser::PushBackToken(tok);
    }
    return true;
}

// Relational Expression
bool RelExpr(istream& in, int& line) {
    if(!AddExpr(in, line)) return false;
    
    LexItem tok = Parser::GetNextToken(in, line);
    if(tok == LTHAN || tok == GTHAN) {
        if(!AddExpr(in, line)) {
            ParseErMessage(line, "Missing operand after relational operator.");
            return false;
        }
    } else {
        Parser::PushBackToken(tok);
    }
    return true;
}

// Addition and Subtraction Expression
bool AddExpr(istream& in, int& line) {
    if(!MultExpr(in, line)) return false;
    
    LexItem tok = Parser::GetNextToken(in, line);
    if(tok == PLUS || tok == MINUS) {
        if(!MultExpr(in, line)) {
            ParseErMessage(line, "Missing operand after addition operator.");
            return false;
        }
    } else {
        Parser::PushBackToken(tok);
    }
    return true;
}

// Multiplication, Division, and Modulus Expression
bool MultExpr(istream& in, int& line) {
    if(!UnaryExpr(in, line)) return false;
    
    LexItem tok = Parser::GetNextToken(in, line);
    while (tok == MULT || tok == DIV || tok == REM) {
        if(!UnaryExpr(in, line)) {
            ParseErMessage(line, "Missing operand after operator.");
            return false;
        }
        tok = Parser::GetNextToken(in, line);
    }
    Parser::PushBackToken(tok);
    return true;
}
// Unary Expression
bool UnaryExpr(istream& in, int& line) {
    int unaryOp = 0;
    LexItem tok = Parser::GetNextToken(in, line);
    if (tok == MINUS || tok == PLUS || tok == NOT) {
        unaryOp = tok.GetToken();
    } else {
        Parser::PushBackToken(tok);
    }
    
    if(!PrimaryExpr(in, line, unaryOp)) {
        ParseErMessage(line, "Invalid primary expression after unary operator.");
        return false;
    }
    return true;
}

// Primary Expression
bool PrimaryExpr(istream& in, int& line, int unaryOperator) {
    LexItem tok = Parser::GetNextToken(in, line);
    bool success = false;

    if (tok == IDENT) {
        string name = tok.GetLexeme();
        success = varAvailable(name, line);
    } else if (tok == ICONST || tok == RCONST || tok == BCONST || tok == CCONST || tok == SCONST) {
        success = true;
    } else if (tok == LPAREN) {
        if (!Expr(in, line)) {
            ParseErMessage(line, "Missing expression after Left Parenthesis.");
            return false;
        }
        tok = Parser::GetNextToken(in, line);
        if (tok != RPAREN) {
            ParseErMessage(line, "Missing Right Parenthesis.");
            return false;
        }
        success = true;
    } else {
        ParseErMessage(line, "Unrecognized Primary Expression.");
        return false;
    }

    // Apply unary operator effects
    if (success && unaryOperator != 0) {
        switch (unaryOperator) {
            case MINUS:
            case PLUS:
                if (tok != ICONST && tok != RCONST) {
                    ParseErMessage(line, "Unary + or - applied to non-numeric operand.");
                    return false;
                }
                break;
            case NOT:
                if (tok != BCONST) {
                    ParseErMessage(line, "Unary ! applied to non-boolean operand.");
                    return false;
                }
                break;
        }
    }

    return success;
}

