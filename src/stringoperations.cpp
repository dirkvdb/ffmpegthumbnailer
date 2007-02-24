#include "stringoperations.h"

#include <algorithm>

using namespace std;

string& StringOperations::lowercase(string& aString)
{
    transform(aString.begin(), aString.end(), aString.begin(), ToLower());
    return aString;
}

string& StringOperations::dos2unix(string& line)
{
    size_t pos = line.find_last_of('\r');
    
    if (string::npos != pos)
    {
        line.erase(pos, 1);
    }
    
    return line;
}

string& StringOperations::replace(string& aString, const string& toSearch, const string& toReplace)
{
    size_t startPos = 0;
    size_t foundPos;
    
    while (string::npos != (foundPos = aString.find(toSearch, startPos)))
    {
        aString.replace(foundPos, toSearch.length(), toReplace);
        startPos = foundPos + toReplace.size();
    }
    
    return aString;
}

vector<string> StringOperations::tokenize(const string& str, const string& delimiter)
{
    vector<string>  tokens;
    string          tempString = str;
    size_t          pos = 0;
        
    while ((pos = tempString.find(delimiter)) != string::npos)
    {
        tokens.push_back(tempString.substr(0, pos));
        tempString.erase(0, pos + delimiter.size());
    }
    tokens.push_back(tempString);
    
    return tokens;
}

