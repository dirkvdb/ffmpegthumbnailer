#include <iostream>

#include "videothumbnailer.h"
#include "exception.h"

using namespace std;

int main(int argc, char** argv)
{
	if (argc != 4)
	{
		return -1;
	}
	
	try
	{
		VideoThumbnailer videoThumbailer(argv[1]);
		videoThumbailer.generateThumbnail(argv[2], atoi(argv[3]));
	}
	catch (Exception& e)
	{
		cerr << "Error: " << e.getMessage() << endl;
		return -1;
	}		
	
	return 0;
}
