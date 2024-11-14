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

// Given code start
namespace Parser{
bool pushed_back = false;
LexItem pushed_token;
// Getting next token
static LexItem GetNextToken (istream & in, int& line)
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
void ParseError(int line, string message)
{
    ++error_count;
    cout << line << ": " << message << endl;
}

// Controlling Indentation
bool Prog(istream& in, int& line)
{
    LexItem tok = Parser::GetNextToken(in, line);
    if(tok != PROGRAM)
    {
        ParseError(line, "Missing PROGRAM keyword");
        return false;
    }
    
    if(tok != IDENT)
    {
        ParseError(line, "Missing program name.");
        return false;
    }
    // Checking structure
    if(!CompStmt(in, line) )
    {
        ParseError(line, "Invalid Program Structure");
        return false;
    }
    cout << "DONE" << endl;
    return true;
}


