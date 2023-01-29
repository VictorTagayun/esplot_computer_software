# Deployment on different platforms (Version 0.9)

First, compile the project on the desired platform (supported platforms at the moment: Win10 x64, Ubuntu > 16.04, MacOS > High Sierra) with Qt Version 5.12.0 in Release mode

## Deployment on Windows
* Open a terminal and go to the build directory of the *ESPlot-Computer-Software*
* Delete all files except the .exe-file
* Run *windeployqt* with the following parameters *windeployqt ESPlot.exe*
* Copy *LibFT4222-64.dll* (from the library folder) into the build directory
* Copy *ftd2xx64.dll* (from driver storage of the *C:/Windows* folder) into the build directory
* Copy *libgcc_s_seh-1.dll*, *libstdc++-6.dll*, *libwinpthread-1.dll* (from the MinGW package in *C:/Qt/5.12.0/mingw73_64/bin*) into the build directory

## Deployment on Linux

* For deployment (and only for deployment) the oldest still-available LTS version of Ubuntu has to be used. In this case this was made on Ubuntu 16.04.
* Open a terminal and go to the build directory of the *ESPlot-Computer-Software*  
* Run make clean in that directory to remove all object files
* Add the path of the QT libraries to your path: *export PATH=/opt/qt/5.12.0/gcc_64/bin:$PATH*
* Run the tool *linuxdeployqt-6-x86_64* with the following parameters: *linuxdeployqt-6-x86_64.AppImage ESPlot*
* Manually copy the library file *libft4222.so.1.4.2.184* into the lib folder of the build folder

## Deployment on MacOS
* If not installed, install *boost@1.55* with the help of *homebrew*
* Open a terminal and rename the folder */usr/local/lib/boost@1.55* to */usr/local/lib/boost*
* Go to the build directory of the *ESPlot-Computer-Software*  
* Run make clean in that directory to remove all object files
* Run the tool *macdeployqt* with the following parameters: */Users/**USERNAME**/Qt/5.12.0/clang_64/bin/macdeployqt ESPlot.app/*
* Go into the created app directory
* Go to *Contents/Frameworks*
* Manually copy the library *libft4222.1.4.2.184.dylib* into this folder
* Manually copy the *libboost_system.dylib* into this folder
* Use *otool* to check the dependencies of the *libft4222*: *otool -L libft4222.1.4.2.184.dylib*
* Then use the *install_name_tool* to change the relative path of the boost library with the following parameters: *install_name_tool -change /usr/local/opt/boost/lib/libboost_system.dylib @rpath/libboost_system.dylib libft4222.1.4.2.184.dylib*
* Countercheck with *otool*: *otool -L libft4222.1.4.2.184.dylib*
* Then navigate to *ESPlot.app/Contents*
* Use *otool* to check the dependencies of the *ESPlot*: *otool ESPlot otool ./ESPlot -L*
* Then use the *install_name_tool* to change the relative path of the *libft4222* library with the following parameters: *install_name_tool -change build-x86_64/libft4222.1.4.2.184.dylib @rpath/libft4222.1.4.2.184.dylib ESPlot*
* Finally, countercheck with *otool*: *otool -L ESPlot*