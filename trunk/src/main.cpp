#include <iostream>
#include <unistd.h>

#include "videothumbnailer.h"
#include "exception.h"

using namespace std;

void printUsage();

int main(int argc, char** argv)
{
	int 	option;
	int 	seekPercentage = 10;
	int 	thumbnailSize = 128;
	bool 	filmStripOverlay = false;
	char* 	inputFile = NULL;
	char* 	outputFile = NULL;
	
	while ((option = getopt (argc, argv, "i:o:s:t:fh")) != -1)
	{
		switch (option)
		{
			case 'i':
				inputFile = optarg;
				break;
			case 'o':
				outputFile = optarg;
				break;
			case 's':
				thumbnailSize = atoi(optarg);
				break;
			case 'f':
				filmStripOverlay = true;
				break;
			case 't':
				seekPercentage = atoi(optarg);
				break;
			case 'h':
				printUsage();
				return 0;
			case '?':
			default:
				cerr << "invalid arguments" << endl;
				printUsage();
				return -1;
		}
	}
	
	if (!inputFile || !outputFile)
	{
		cerr << "invalid arguments" << endl;
		printUsage();
	}
		   
	try
	{
		VideoThumbnailer videoThumbailer(inputFile);
		videoThumbailer.generateThumbnail(outputFile, thumbnailSize, filmStripOverlay, seekPercentage);
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

void printUsage()
{
	cout << "Usage: ffmpegvideothumbnailer [options]" << endl << endl
		 << "Options:" << endl
		 << "  -i<s>  : input file" << endl
		 << "  -o<s>  : output file" << endl
		 << "  -s<n>  : thumbnail size (default: 128)" << endl
		 << "  -t<n>  : time to seek to (percentage) (default: 10)" << endl
		 << "  -m     : create a movie strip overlay" << endl
		 << "  -h     : display this help" << endl;
}
