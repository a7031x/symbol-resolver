This program resolves duplicate symbol problem in XCode.
Sometimes when you introduce two third party libraries and they happen to incorporate the same sub-library, linking the two libraries will extert duplicate symbol errors. The error messages look like this,

duplicate symbol _tls1_new in:
    /<path1>/libssl.a(t1_lib.o)
    /<path2>/xxxSDK

This program renames the conflicting symbols inside the libraries, that means it will permenantly change the libraries. Generally the renamed symbols are only used inside each library, so the libraries won't behave differently, you don't need to change the source code. But you need to SAVE THE LIBRARIES as backups in case the changes corrupt the libraries. If you are ready, let's follow the steps to resolve the problems.

1. Create an XCode terminal project named symbol_resolver and add main.cpp, string_algo.hpp, stringable.hpp to the project.
2. Build the project.
3. Copy the error messages from your XCode project (the project which incurs duplicate symbol problem) and save to error.txt.
4. Copy error.txt to the path where symbol_resolver has been built.
5. Run the symbol_resolver.

Note that the error messages already contain the paths where the conflicting library are, so this program will locate the files and change them automatically.
