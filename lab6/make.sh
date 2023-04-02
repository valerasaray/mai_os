#!/bin/bash
lsof -i > ./log.txt
cat ./log.txt | grep TCP | grep server | awk '{ print $2 }' | xargs kill -9
cat ./log.txt | grep TCP | grep client | awk '{ print $2 }' | xargs kill -9
ps -a > ./log.txt
cat ./log.txt | grep server | awk '{print $1}' | xargs kill -9
rm -rf ./build
mkdir build && cd ./build
cmake ..
make
./client
ps -a > ./log.txt
cat ./log.txt | grep server | awk '{print $1}' | xargs kill -9
