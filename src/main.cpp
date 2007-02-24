#include <iostream>

#include "videothumbnailer.h"
#include "exception.h"

using namespace std;

int main(int argc, char** argv)
{
	if (argc == 2)
	{
		if (0 == strcmp(argv[1], "-h"))
		{
			cout << "Usage: ffmpegthumbnailer: infile outfile size" << endl;
			return 0;
		}
	}
	else if (argc != 4)
	{
		cout << "Usage: ffmpegthumbnailer: infile outfile size" << endl;
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
	catch (...)
	{
		return -1;
	}
	
	return 0;
}
