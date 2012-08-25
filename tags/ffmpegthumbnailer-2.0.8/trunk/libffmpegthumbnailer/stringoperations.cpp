//    Copyright (C) 2010 Dirk Vanden Boer <dirk.vdb@gmail.com>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "stringoperations.h"

#include <algorithm>

using namespace std;

namespace ffmpegthumbnailer
{

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

}
