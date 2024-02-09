#include "Application.h"
#include <iostream>

int main(int argc, char** argv)
{
	Application app;

	try
	{
		app.Run();
	}
	catch (std::exception& ex)
	{
		std::cerr << ex.what();
	}


	return 0;
}