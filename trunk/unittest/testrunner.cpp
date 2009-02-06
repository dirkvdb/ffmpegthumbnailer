#include <unittest++/UnitTest++.h>

#include <locale.h>
#include <iostream>

using namespace std;

int main(void)
{
    if (!setlocale(LC_CTYPE, ""))
    {
        cerr << "Locale not specified. Check LANG, LC_CTYPE, LC_ALL" << endl;
        return 1;
    }
    return UnitTest::RunAllTests();
}
