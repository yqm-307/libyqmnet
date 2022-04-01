#!/bin/bash

bin="bin"

isclear="$*"

#清理cmake和makefile文件
clearfile()
{
    rm cmake_install.cmake
    rm CMakeCache.txt
    rm -rf CMakeFiles
    rm *.log
    rm Makefile
    rm -rf */CMakeFiles
    rm */cmake_install.cmake
    rm */Makefile
    rm */CMakeCache.txt
    rm -rf */other/CMakeFiles
    rm */other/cmake_install.cmake
    rm */other/Makefile
    rm */other/CMakeCache.txt
    echo "清理成功"
}

#清理可执行文件和库文件
clearexec()
{
    rm -rf bin  #可执行文件
    rm -rf lib  #库文件
}

build()
{
    #bin目录已经存在，执行cmake就先清空以前的数据
    if [ -d ${bin} ]
    then
        clearexec
        clearfile
    fi

    cmake .
    make

    if [ -d ${bin} ]
    then
        echo "build over"
    fi
}




if [ "${isclear}" = "clearfile" ]
then
    clearfile
elif [ "${isclear}" = "clearexec" ]
then
    clearexec
elif [ "${isclear}" = "clearall" ]
then
    clearexec
    clearfile
else
    build
fi





