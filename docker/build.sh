#!/usr/bin/env bash

# Check args
if [ "$#" -ne 3 ]; then
  echo "usage: ./build.sh user_name user_id group_id"
  exit
fi

echo USER = $1
echo UID = $2
echo GID = $3 		#cut -d: -f3 < <(getent group sudo)
echo HOME = $HOME
echo SHELL = $SHELL

sudo docker build\
  --build-arg user=$1\
  --build-arg uid=$2\
  --build-arg gid=$3\
  --build-arg home=$HOME\
  --build-arg shell=$SHELL\
  -t carnd-term2 .
