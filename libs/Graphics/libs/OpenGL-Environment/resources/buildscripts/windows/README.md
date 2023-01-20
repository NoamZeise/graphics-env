### note: these scripts assume you are using the msvc compiler, have the msvc command prompt executables in your path(vcvars64), have ninja installed and in your path and have a folder called build in your project root

Run these build scripts from the project root, so they would be ran as
```
"resources/buildscripts/windows/debug"
``` from a command  prompt (this makes it easier for me to use them with editors like vscode or atom for quick building)

If you change the name of the project in the root CMakeLists.txt, update the scripts to execute the new name of the final binary.
