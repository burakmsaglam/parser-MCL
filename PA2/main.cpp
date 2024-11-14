#include <iostream>
#include <fstream>
#include <stack>
#include <string>


using namespace std;
int BalancedBrackets(string inputString)
{
    stack<char> bracketStorage;
    bool hasBrack = false;
    char ch;
    for (int  i = 0; i < inputString.length() ; i++)
    {
        ch = inputString[i];
        // if open
        if (ch == '{' || ch == '[' || ch == '(')
        {
            bracketStorage.push(ch);
            hasBrack = true;
        }
        // if close
        else if (ch == '{' || ch == ')' || ch == ']')
        {
            hasBrack = true;
            if(bracketStorage.empty() && ch == ')')
            {
                cerr <<"case 4: There are more closing brackets than opening ones for one or more bracket types in the input string." << endl;
                return -1;
            }
            else if(bracketStorage.empty() && ch == '(')
            {
                cerr <<"case 4: There are more opening brackets than closing ones for one or more bracket types in the input string." << endl;
                return -1;
            }
            char top = bracketStorage.top();
            // if matches
            if((ch == '}' && top == '{') || (ch == ')' && top == '(') || (ch == ']' && top == '['))
            {
                bracketStorage.pop();
            }
            else
            {
                cout << "case 3: Mismatching bracket type between an opening and a closing bracket." << endl;
                return -1;
            }
        }
    }
    // no bracket
    if (!hasBrack)
    {
        cout << "case 5: Scanning the input string is completed successfully." << endl;
        return 0;
    }
    // brackets matched
    if (bracketStorage.empty())
    {
        cout << "case 1: Scanning the input string is completed successfully." << endl;
        return 1;
    }
    // no luck
    else
    {
        cout << "case 2: There are more opening brackets than closing ones for one or more bracket types in the input string." << endl;
        return -1;
    }
}

int main(int argc, char *argv [])
{
    string sentence;
    int balanced;
        
    ifstream file;
    if( argc >= 2 ) {
        file.open(argv[1]);
        if( !file)
        {
            /* error */
            cerr << "File cannot be opened: " << argv[1] << endl;
            exit (1);
        }
                
    }
    else
    {
        cerr << "No file is found" << endl;
        exit (1);
    }
         
    getline(file, sentence);
    if(sentence.length() == 0)
    {
        cout << "File is empty." << endl;
        return 0;
    }
    
    balanced = BalancedBrackets(sentence);
    cout << endl;
    
    if (balanced == 1)
        cout << "The following sentence contains balanced brackets: " << sentence << endl;
    else if(balanced == -1)
        cout << "The following sentence does not contain balanced brackets: " << sentence << endl;
    else
        cout << "The following sentence does not contain any brackets: " << sentence << endl;
}

