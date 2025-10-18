@echo off & setlocal
    set Platform=%1
    if "%Platform%"=="" set Platform=arm64-v8a

    if exist build\android\%Platform% rd /s /q build\android\%Platform%
    mkdir build\android\%Platform%
    cd build\android\%Platform%
    REM 都统一一下,以S3这边为准,NDK 的环境变量名统一为 NDK_HOME
    set ANDROID_NDK=%NDK_HOME%
    set BUILD_TYPE=RelWithDebInfo
    del /s /q "./CMakeCache.txt"
    
    if "%Platform%"=="arm64-v8a" (
        set VERSION=21
        set "OPTIONS"=""
    )
    if "%Platform%"=="armeabi-v7a" (
        set VERSION=19
        set "OPTIONS"="-DCMAKE_ANDROID_ARM_NEON=TRUE"
    )

    cmake -DANDROID=ON ../../.. -GNinja ^
     -DCMAKE_SYSTEM_NAME=Android -DANDROID_NDK=%ANDROID_NDK% ^
     -DCMAKE_ANDROID_ARCH_ABI="%Platform%" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
     -DCMAKE_ANDROID_STL_TYPE=gnustl_static -DCMAKE_SYSTEM_VERSION=%VERSION% %OPTIONS%

    ninja -j8
endlocal & exit /b
