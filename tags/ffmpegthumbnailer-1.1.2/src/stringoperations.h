#ifndef STRING_OPERATIONS_H
#define STRING_OPERATIONS_H

#include <string>
#include <vector>
#include <sstream>

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

#endif
