#include "app.h"
#include <stdexcept>
#ifndef NDEBUG
#include <iostream>
#endif
#include <fstream>


int main(int argc, char** argv)
{
	try
	{
#ifndef NDEBUG
		std::cout << "In debug mode" << std::endl;
#endif
		RenderFramework defaultFramework = RenderFramework::VULKAN;
		for(int i = 1; i < argc; i++) {
		    if(strcmp(argv[i], "-r") == 0) {
			if(i + 1 < argc) {
			    i++;
			    if(strcmp(argv[i], "opengl") == 0) {
				defaultFramework = RenderFramework::OPENGL;
				std::cout << "default framework opengl selected\n";
			    }
			    if(strcmp(argv[i], "vulkan") == 0) {
				defaultFramework = RenderFramework::VULKAN;
				std::cout << "default framework vulkan selected\n";
			    }
			}
		    }
		}
		App app(defaultFramework);
		app.run();
	}
	catch (const std::exception& e)
	{
		#ifndef NDEBUG
		std::cerr << e.what() << std::endl;
		#else
		std::ofstream crashFile("CrashInfo.txt");
		if (crashFile.is_open())
		{
			crashFile.seekp(0);
			crashFile << e.what();
			crashFile.close();
		}
		#endif
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
