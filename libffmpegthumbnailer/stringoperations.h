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

#ifndef STRING_OPERATIONS_H
#define STRING_OPERATIONS_H

#include <string>
#include <vector>
#include <sstream>

namespace ffmpegthumbnailer
{

class StringOperations
{
public:
    static std::string& lowercase(std::string& aString);
    static std::string& dos2unix(std::string& line);
    static std::string& replace(std::string& aString, const std::string& toSearch, const std::string& toReplace);
    
    template<typename T>
    static std::string toString(const T& numeric)
    {
        std::stringstream ss;
        ss << numeric;
        
        return ss.str();
    }
    
    static std::vector<std::string> tokenize(const std::string& str, const std::string& delimiter);
    
private:
    class ToLower
    {
    public:
        char operator() (char c) const
        {
            return tolower(c);
        }
    };
};

}

#endif
