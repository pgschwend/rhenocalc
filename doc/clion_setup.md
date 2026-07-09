
# Setup CLion for RhenoCalc

To be able to run and debug the Qt Application natively from CLion.

## Windows

### Settings in Build, Execution, Deployment - CMake
![scr_settings_cmake.png](img/scr_settings_cmake.png)

Enter the Qt build path, see example below (modify according to your system)

CMake options: ````-DCMAKE_PREFIX_PATH="C:\Qt\6.9.3\mingw_64````

Environment: ````PATH=C:\Qt\6.9.3\mingw_64\bin````

### Settings in Edit Configuration - Debug
![scr_edit_configuration.png](img/scr_edit_configuration.png)

Environment variables: ````PATH=C:\Qt\6.9.3\mingw_64\bin````

## Ubuntu

### Settings in Build, Execution, Deployment - CMake
![img.png](img/img.png)

Enter the Qt build path, see example below (modify according to your system)

CMake options: ````-DCMAKE_PREFIX_PATH=/opt/qt/6.9.3/gcc_64````