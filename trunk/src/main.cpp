#include <iostream>

#include "videothumbnailer.h"
#include "exception.h"

using namespace std;

int main(int argc, char** argv)
{
	try
	{
	
	if (argc == 2)
	{
		if (0 == strcmp(argv[1], "-h"))
		{
			cout << "Usage: ffmpegthumbnailer: infile outfile size [overlayfilmstrip seekpercentage]" << endl;
			return 0;
		}
	}
	else if (argc == 4)
	{
		VideoThumbnailer videoThumbailer(argv[1]);
		videoThumbailer.generateThumbnail(argv[2], atoi(argv[3]), 1, 10);
	}
	else if (argc == 6)
	{
		VideoThumbnailer videoThumbailer(argv[1]);
		videoThumbailer.generateThumbnail(argv[2], atoi(argv[3]), atoi(argv[4]), atoi(argv[5]));
	}
	else
	{
		cout << "Usage: ffmpegthumbnailer: infile outfile size [overlayfilmstrip seekpercentage]" << endl;
		return -1;
	}
	
	
		
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
