#!/usr/bin/env bash

group_id=$(cut -d: -f3 < <(getent group sudo))
echo $group_id
sudo docker build\
  --build-arg user=$USER\
  --build-arg uid=$UID\
  --build-arg gid=$group_id\
  --build-arg home=$HOME\
  --build-arg shell=$SHELL\
  -t carnd-term2 .
