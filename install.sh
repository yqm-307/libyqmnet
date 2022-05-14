#!/bin/bash


evpath="$*"

uninstall()
{
    if [ -d "/usr/local/include/libyqmnet" ]
    then
        sudo rm -rf /usr/local/include/libyqmnet
        echo "rm -rf /usr/local/include/libyqmnet"
    fi
    if [ -f "/usr/lib/x86_64-linux-gnu/libyqmnet.so" ]
    then
        sudo rm /usr/lib/x86_64-linux-gnu/libyqmnet.so
        echo "rm /usr/lib/x86_64-linux-gnu/libyqmnet.so"
    fi
}

install()
{
    sudo cp -rf include libyqmnet
    echo "cp -rf include libyqmnet"
    sudo mv libyqmnet /usr/local/include
    echo "mv libyqmnet /usr/local/include"
    sudo cp -rf lib/libyqmnet.so /usr/lib/x86_64-linux-gnu/
    echo "cp -rf lib/libyqmnet.so /usr/lib/x86_64-linux-gnu/"
}

if [ "${evpath}" = "uninstall" ]
then
    uninstall
else
    install
fi