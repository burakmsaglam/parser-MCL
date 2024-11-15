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

int nest_level =0;

map<string, bool> trackingVar;

// Given code start
namespace Parser{
    bool pushed_back = false;
    LexItem pushed_token;
    // Getting next token
    static LexItem GetNextToken(istream & in, int& line)
    {
        if(pushed_back)
        {
            pushed_back = false;
            return pushed_token;
        }
        return getNextToken(in, line);
    }
    // Trying to push token back
    static void PushBackToken(LexItem & t)
    {
        // Pushing more than once gives error.
        if ( pushed_back)
        {
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
int ErrCount(){
    return error_count;
}

//Increasing the error
void ParseErMessage(int line, string message)
{
    ++error_count;
    cout << line << ": " << message << endl;
}

// Declaring variable function
bool declareVar(const string& varName, int line)
{
    // if found, means declared, find method returns map.end if not found
    if(trackingVar.find(varName) != trackingVar.end())
    {
        ParseErMessage(line, "Variable named : " + varName +" already declared");
        return false;
    }
    // if not found then declare it.
    trackingVar[varName] = false; // not initialized
    return true;
}

// Initializing variable in the table
void initializeVar(string & varName)
{
    trackingVar[varName] = true;
}
// available var
bool varAvailable(string & varName, int line)
{
    // If variable name not found in trackingVar container map where all declared variable are.
    if(trackingVar.find(varName) == trackingVar.end())
    {
        ParseErMessage(line, "Undeclared variable!");
        return false;
    }
    //Is variable initialized?
    if(!trackingVar.find(varName)->second)
    {
        ParseErMessage(line, "Uninitialized variable: " +varName);
        return false;
    }
    return  true;
}

// Controlling Indentation
bool Prog(istream& in, int& line)
{
    LexItem tok = Parser::GetNextToken(in, line);
    if(tok != PROGRAM)
    {
        ParseErMessage(line, "Missing PROGRAM keyword");
        return false;
    }
    
    if(tok != IDENT)
    {
        ParseErMessage(line, "Missing program name.");
        return false;
    }
    // Checking structure
    if(!CompStmt(in, line) )
    {
        ParseErMessage(line, "Invalid Program Structure");
        return false;
    }
    cout << "DONE" << endl;
    return true;
}
// istream passed as reference each time to continue on file. Same as line
bool CompStmt(istream& in, int& line)
{
    LexItem tok = Parser::GetNextToken(in, line);
    if(tok != LBRACE)
    {
        ParseErMessage(line, "Missing left brace.");
        return false;
    }
    
    // Checking statement list
    if(!StmtList(in, line))
    {
        ParseErMessage(line, "Incorrect statement list.");
        return false;
    }
    
    // If all good get the next one.
    tok = Parser::GetNextToken(in, line);
    if(tok != RBRACE)
    {
        ParseErMessage(line, "Missing right brace.");
        return false;
    }
    // If everything passes return true
    return true;
}

bool StmtList(istream& in, int& line)
{
    // Break only in the condition that is not statement or next token is not statement
    while(true)
    {
        if(!Stmt(in,line)) return false; // Error message given in the CompStmt function.
        // If Stmt get next token
        LexItem tok = Parser::GetNextToken(in, line);
        // If next item is not statement when first item being statement break out of the loop
        if(tok != SEMICOL && tok != LBRACE && tok != IF && tok != PRINT && tok != IDENT)
        {
            // Do not forget to put the token back
            Parser::PushBackToken(tok);
            break;
        }
        // In Stmt new token will be called, we need to put token back since we checked the first statement with Stmt function, and after it is correct we called the second one to check if we can continue in the if statement. If we can continue we need to go back 1 step to recheck if the second argument is actually statement.
        Parser::PushBackToken(tok);
    }
    // If succesfully parsed return true.
    return true;
}

bool Stmt(istream& in, int& line)
{
    // Get the next token
    LexItem tok = Parser::GetNextToken(in, line);
    // Checking statement type and if really is statement.
    switch(tok.GetToken())
    {
        //Decl
        case INT: case FLOAT: case BOOL: case CHAR: case STRING: return DeclStmt(in, line);
        
        //Control
            // if
        case IF: return IfStmt(in, line);
            // print
        case PRINT: return PrintStmt(in, line);
            // assignment
        case IDENT:
            //Pushback token to check in the function
            Parser::PushBackToken(tok);
            return AssignStmt(in, line);
            
        //Comp
        case LBRACE:
            
        //Not any one of them
        default:
            ParseErMessage(line, "Invalid Statement.");
            return false;
    }
}

// int x;
bool DeclStmt(istream& in, int& line)
{
    // Checking varlist Note: Will get the next token in varlist no need to get it in this function!
    if (!VarList(in, line))
    {
        ParseErMessage(line, "Incorrect variable list.");
        return false;
    }
    
    // Get next token if in varlist
    LexItem tok = Parser::GetNextToken(in, line);
    if(tok != SEMICOL)
    {
        ParseErMessage(line, "Missing semicolon");
        return false;
    }
    cout << "Initialization of the variable" << tok.GetLexeme() << "in the decleration statement at line " << line << endl;
    return true;
}


bool VarList(istream& in, int& line)
{
    LexItem tok = Parser::GetNextToken(in, line);
    if(tok != IDENT)
    {
        ParseErMessage(line, "Identifier is missing.");
        return false;
    }
    
    // declaring var
    string varName = tok.GetLexeme();
    if (!declareVar(varName, line)) return false; // error message is inside the declarevar func.
    
    // checking if there is assignment operator
    tok = Parser::GetNextToken(in, line);
    if(tok == ASSOP)
    {
        if(!Expr(in, line))
        {
            ParseErMessage(line, "Missing expression after assignment operator!");
            return false;
        }
        // if there is expression, initialize variable.
        initializeVar(varName);
    }
    
    // Checking if there is comma
    if(tok == COMMA)
    {
        return VarList(in, line); // recursively check for comma
    }
    else if(tok == SEMICOL) return true; // end of decleration
    else Parser::PushBackToken(tok); // end of variables
    return true; // successfull parse
}

bool AssignStmt(istream& in, int& line)
{
    LexItem tok = Parser::GetNextToken(in, line);
    if(tok != IDENT)
    {
        ParseErMessage(line, "Missin identifier after assignment operator..");
        return false;
    }
    
    string varName = tok.GetLexeme();
    // when variable initialized and declared, since this is not a decleration!
    if (!varAvailable(varName, line)) return false; // error message inside the function.
    
    // if it is okay to use variable get next token which needs to be assignment operator
    tok = Parser::GetNextToken(in, line);
    if(tok == ASSOP)
    {
        if(!Expr(in,line))
        {
            ParseErMessage(line, "Missing expression assignment operator!");
            return false;
        }
        // if it is indeed expression, make it initialized in the trackingVar using the initializer function.
        initializeVar(varName);
        return true; // parsed ok
    }
    else if(tok == ADDASSOP || tok == SUBASSOP || tok == MULASSOP || tok == DIVASSOP || tok == REMASSOP)
    {
        // check if variable is initialized-- these operators need variable to be initialized
        if(!trackingVar[varName])
        {
            ParseErMessage(line, "Variable needs to be initialized when used with add, sub, mul, div, rem assops. Var name is :" + varName);
            return false;
        }
        if(!Expr(in,line))
        {
            ParseErMessage(line, "Missing expression assignment operator asops!");
            return false;
        }
        // if no problem.
        return true;
    }
    // if token is not assignment operator
    ParseErMessage(line, "Invalid assignment operator");
    return false;
}
