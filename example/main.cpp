#include "app.h"
#include <stdexcept>
#ifndef NDEBUG
#include <iostream>
#endif
#include <fstream>
#include <cstring>


bool frameworkArg(int argc, char** argv, int &i, RenderFramework *framework);


int main(int argc, char** argv) {
  try
    {
#ifndef NDEBUG
      std::cout << "In debug mode" << std::endl;
#endif
      RenderFramework framework = RenderFramework::VULKAN;
      for(int i = 1; i < argc; i++) {
	  bool argHandled = false;
	  argHandled |= frameworkArg(argc, argv, i, &framework);
	  if(!argHandled) {
	      std::cerr << "passed unrecognised arg to app\n";
	      std::cerr << "  recognised args: \n"
		           "      -r [param] -> choose rendering framework\n";
	      return 1;
	  }
      }
      App app(framework);
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


bool frameworkArg(int argc, char** argv, int &i, RenderFramework *framework) {
  if(strcmp(argv[i], "-r") == 0) {
    if(i + 1 < argc) {
      i++;
      if(strcmp(argv[i], "opengl") == 0) {
	*framework = RenderFramework::OPENGL;
	std::cout << "default framework opengl selected\n";
      }
      else if(strcmp(argv[i], "vulkan") == 0) {
	*framework = RenderFramework::VULKAN;
	std::cout << "default framework vulkan selected\n";
      } else {
	  std::cerr << "unrecognised framework passed, 'vulkan' or 'opengl'\n";
	  return false;
      }
      return true;
    }
  }
  return false;
}
