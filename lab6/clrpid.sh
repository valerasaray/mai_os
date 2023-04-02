#!/bin/bash
lsof -i > ./log.txt
cat ./log.txt | grep TCP | grep server | awk '{ print $2 }' | xargs kill -9