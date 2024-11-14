/*
 * lex.cpp
 * Lexical Analyzer for the Mini C-Like Language
 * CS280 - Fall 2024
 */

#include <cctype>
#include <map>

using std::map;
using namespace std;

#include "lex.h"
//Keywords or reserved words mapping
LexItem id_or_kw(const string& lexeme , int linenum)
{
	string strlexeme = lexeme;
	map<string,Token> kwmap = {
		{ "print", PRINT},
		{ "if", IF },
		{ "else", ELSE },
		{ "string", STRING },
		{ "int", INT },
		{ "float", FLOAT },
		{ "char", CHAR },
		{ "bool", BOOL },
		{ "program", PROGRAM },
		{ "true", TRUE },
		{ "false", FALSE },
		
	};
	Token tt ;
	//convert the lexeme to all lower case letters
	
	//cout << "inside id_or_kw " << lexeme << endl;
	for(int i = 0; i < lexeme.length(); i++)
	{
		strlexeme[i] = tolower(strlexeme[i]);
	}
	
	tt = IDENT;
	auto kIt = kwmap.find(strlexeme);
	if( kIt != kwmap.end() )
	{
		tt = kIt->second;
	}
	
	if(tt == TRUE || tt == FALSE)	
		tt = BCONST;
	return LexItem(tt, lexeme, linenum);
}

map<Token,string> tokenPrint = {
		{PROGRAM, "PROGRAM" },
		{PRINT, "PRINT"},
		{ IF, "IF" },
		{ ELSE, "ELSE" },	
		{ IDENT, "IDENT" },
		{ BOOL, "BOOL" },
		{ STRING, "STRING" },
		{ INT, "INT" },
		{ FLOAT, "FLOAT" },
		{ CHAR, "CHAR" },
		{ ICONST, "ICONST" },
		{ RCONST, "RCONST" },
		{ SCONST, "SCONST" },
		{ BCONST, "BCONST"},
		{ CCONST, "CCONST" },
		
			
		{ PLUS, "PLUS" },
		{ MINUS, "MINUS" },
		{ MULT, "MULT" },
		{ DIV, "DIV" },
		{ ASSOP, "ASSOP" },
		{ EQ, "EQ" },
		{ NEQ, "NEQ" },
		{ GTHAN, "GTHAN" },
		{ LTHAN, "LTHAN" },	
		{ AND, "AND" },
		{ OR, "OR" },
		{ NOT, "NOT" },
		{ COMMA, "COMMA" },
		{ SEMICOL, "SEMICOL" },
		{ LPAREN, "LPAREN" },
		{ RPAREN, "RPAREN" },
		{ LBRACE, "LBRACE" },
		{ RBRACE, "RBRACE" },
		{ REM, "REM" },
		{ ADDASSOP, "ADDASSOP" },
		{ SUBASSOP, "SUBASSOP" },
		{ MULASSOP, "MULASSOP" },
		{ DIVASSOP, "DIVASSOP" },
		{ REMASSOP, "REMASSOP" },
		{ TRUE, "TRUE" },
		{ FALSE, "FALSE" },
		{ DOT, "DOT" },
		{ ERR, "ERR" },

		{ DONE, "DONE" },
};

ostream& operator<<(ostream& out, const LexItem& tok) {
	
	Token tt = tok.GetToken();
	out << tokenPrint[ tt ];
	if( tt == ICONST || tt == RCONST || tt == BCONST) {
		out << ": (" << tok.GetLexeme() << ")";
	}
	else if(tt == IDENT ){
		out << ": <" << tok.GetLexeme() << ">";
	}
	else if(tt == SCONST){
		out << ": \"" << tok.GetLexeme() << "\"";
	}
	else if(tt == CCONST){
		out << ": \'" << tok.GetLexeme() << "\'";
	}
	else if( tok == ERR ) {
		cout << ": In line " << tok.GetLinenum()+1 << ", Error Message {" << tok.GetLexeme() << "}" << endl;
	}
	cout << endl;
	return out;
}

LexItem getNextToken(istream& in, int& linenum)
{
	enum TokState { START, INID, INSTR, ININT, INREAL, INCHAR, INCOMMENT1, INCOMMENT2 } lexstate = START;
	string lexeme, ErrMsg;
	char ch, nextchar, nextch;
	Token tt;
	bool dec = false;
	       
	//cout << "in getNestToken" << endl;
    while(in.get(ch)) {
    	//cout << "in while " << ch << endl;
		switch( lexstate ) {
		case START:
			if( ch == '\n' )
			{
				linenum++;
				//cout << linenum << endl;
			}	
                
			if( isspace(ch) )
				continue;

			//lexeme = ch;

			if( isalpha(ch) || ch =='_') {
				lexeme = ch;
				lexstate = INID;
				//cout << "in ID " << endl;
			}
			else if( ch == '\'' ) {
				lexeme = "";
				//cout << "INCHAR" << endl;
				lexstate = INCHAR;
				
			}
			else if(ch == '\"'){
				lexeme = "";
				lexstate = INSTR;
			}
			
			else if( isdigit(ch) ) {
				lexeme = ch;
				lexstate = ININT;
			}
			
			else if( ch == '/' )
			{
				nextchar = in.peek();
				//cout << "Is this a comment?" << endl;
				if(nextchar == '/')
				{
					in.get(ch);
					//lexeme += ch;
					lexstate = INCOMMENT1;
					//cout << "In comment 1?" << endl;
					continue;
					
				}
				else if(nextchar == '*')
				{
					in.get(ch);
					//lexeme += ch;
					lexstate = INCOMMENT2;
					//cout << "In comment 2?" << endl;
					continue;
					
				}
				else if(nextchar == '=')
				{
					lexeme = ch;
					in.get(ch);
					lexeme += ch;
					tt = DIVASSOP;
					
					return LexItem(tt, lexeme, linenum);
				}	
				else
				{
					tt = DIV;
					lexeme = ch;
					return LexItem(tt, lexeme, linenum);
					
				}	
			}
			else if ( ch == '&')
			{
				lexeme = ch;
				nextchar = in.peek();
				if(nextchar == '&')
				{
					in.get(ch);
					lexeme += ch;
					tt = AND;
					return LexItem(tt, lexeme, linenum);
				}
				else
				{
					return LexItem(ERR, lexeme, linenum);
				}
				
			}
			else if ( ch == '|')
			{
				lexeme = ch;
				nextchar = in.peek();
				if(nextchar == '|')
				{
					in.get(ch);
					lexeme += ch;
					tt = OR;
					return LexItem(tt, lexeme, linenum);
				}
				else
				{
					return LexItem(ERR, lexeme, linenum);
				}
			}
						
			else
			{
				tt = ERR;
				lexeme = ch;
				switch( ch ) {
				case '+':
					tt = PLUS;
					nextchar = in.peek();
					if(nextchar == '=' ){
						in.get(ch);
						lexeme += ch;
						tt = ADDASSOP;
						return LexItem(tt, lexeme, linenum);
					}
					else if(isdigit(nextchar))
					{
						//lexeme = ch;
						in.get(ch);
						lexeme += ch;
						lexstate = ININT;	
						continue;
					}
					else if(nextchar == '.')
					{
						lexstate = INREAL;
						continue;
					}
					return LexItem(tt, lexeme, linenum);
                    break;  
					
				case '-':
					tt = MINUS;
					//lexeme = ch;
					nextchar = in.peek();
					if(nextchar == '=' ){
						in.get(ch);
						lexeme += ch;
						tt = SUBASSOP;
						return LexItem(tt, lexeme, linenum);
					}
					else if(isdigit(nextchar))
					{
						in.get(ch);
						lexeme += ch;
						lexstate = ININT;	
						continue;
					}
					else if(nextchar == '.')
					{
						lexstate = INREAL;
						continue;
					}
					return LexItem(tt, lexeme, linenum);
                    break; 
					
				case '*':
					tt = MULT;
					nextchar = in.peek();
					if(nextchar == '=' ){
						in.get(ch);
						lexeme += ch;
						tt = MULASSOP;
						
					}
					return LexItem(tt, lexeme, linenum);
					break;
				
				case '%':
					tt = REM;
					nextchar = in.peek();
					if(nextchar == '=' ){
						in.get(ch);
						lexeme += ch;
						tt = REMASSOP;
					}
					
					return LexItem(tt, lexeme, linenum);
					break;								
				case '=':
					tt = ASSOP;
					nextchar = in.peek();
					if(nextchar == '='){
						in.get(ch);
						lexeme += ch;
						tt = EQ;
						
					}
					return LexItem(tt, lexeme, linenum);
					break;
					
				case '!':
					tt = NOT;
					nextchar = in.peek();
					if(nextchar == '='){
						in.get(ch);
						lexeme += ch;
						tt = NEQ;
						
					}
					return LexItem(tt, lexeme, linenum);
					break;
				case '(':
					tt = LPAREN;
					break;			
				case ')':
					tt = RPAREN;
					break;
				case '{':
					tt = LBRACE;
					break;			
				case '}':
					tt = RBRACE;
					break;				
				case ',':
					tt = COMMA;
					break;
				case ';':
					tt = SEMICOL;
					break;	
				case '>':
					tt = GTHAN;
					break;
				
				case '<':
					tt = LTHAN;
					break;
					
				case '.':
					tt = DOT;
					nextchar = in.peek();
					if(isdigit(nextchar) )
					{
						in.putback(ch);
						lexeme = "";
						lexstate = INREAL;
						continue;
					}
					
					break;
				
				}//end of inner switch
				return LexItem(tt, lexeme, linenum);
			}//end of else
			
			break;	//break out of START case

		case INID:
			if( isalpha(ch) || isdigit(ch) || ch == '_' ) {
							
				//cout << "in id continued" << ch << endl;
				lexeme += ch;
			}
			else {
				in.putback(ch);
				//cout << lexeme << endl;
				return id_or_kw(lexeme, linenum);
				
			}
			break;
					
		case INCHAR:
            // cout<< "INCHAR 2 " << lexeme<< lexeme.length() << endl;
			if( ch == '\n' ) {
				ErrMsg = "New line is an invalid character constant.";
				return LexItem(ERR, ErrMsg, linenum);
			}
			else if( ch == '\'' && lexeme.length() == 1) {
				//lexeme = lexeme.substr(1, lexeme.length()-2);
				//character constant
				return LexItem(CCONST, lexeme, linenum);
			}
			else if(lexeme.length() >= 1)
			{
				//Invalid character constant length
				lexeme += ch;
				return LexItem(ERR, " Invalid character constant \'" + lexeme + "\'", linenum);
			}
			else if(ch == '\'' && lexeme.length() == 0)
			{
				return LexItem(ERR, " Invalid character constant \'\'", linenum);
			}
			lexeme += ch;
			break;
			
		case INSTR:
                          
			if( ch == '\n' ) {
				return LexItem(ERR, " Invalid string constant \"" + lexeme, linenum);
			}
			if( ch == '\"' ) {
				return LexItem(SCONST, lexeme, linenum);
			}
			lexeme += ch;
			break;
			
		case ININT:
			if( isdigit(ch) ) {
				lexeme += ch;
			}
			else if(ch == '.') {
				nextch = in.peek();
				if(isdigit(nextch))
				{
					in.putback(ch);//put back decimal point
					lexstate = INREAL;
				}
				else
				{
					in.putback(ch);
					return LexItem(ICONST, lexeme, linenum);
				}
				//in.putback(ch);
			}
			else {
				in.putback(ch);
				return LexItem(ICONST, lexeme, linenum);
			}
			break;
		
		case INREAL:
				
			if( ch ==  '.' && isdigit(in.peek()) && !dec) {
				lexeme += ch; 
				dec = true;
			}
			else if(isdigit(ch) && dec ){
				lexeme += ch;
			}
			
			else if((ch == '.') && dec && isdigit(in.peek())){
				lexeme += ch;
				//cout << "error " << lexeme << endl;
				return LexItem(ERR, lexeme, linenum);
			}
			else if((ch == '.' ) && dec && isalpha(in.peek())){
				in.putback(ch);
				return LexItem(RCONST, lexeme, linenum);
			}
			else {
				in.putback(ch);
				//cout << "not error "  << lexeme << endl;
				//decimal = false;
				return LexItem(RCONST, lexeme, linenum);
			}
			
			break;
		
							
		case INCOMMENT1:
			if( ch == '\n' ) {
               	lexstate = START;
               	linenum++;
				//in.putback(ch);
			}
			
			break;
		case INCOMMENT2:
			if(ch == '\n')
				linenum++;
			else if( ch == '*' ) {
               nextch = in.peek();
               if (nextch == '/')
				{
					in.get(ch);
					lexstate = START;
				}
					
			}
			break;
		}//end of switch
	}//end of while loop
	
	if( in.eof() )
		return LexItem(DONE, "", linenum);
		
	return LexItem(ERR, "Error: Some strange symbol", linenum);
}





