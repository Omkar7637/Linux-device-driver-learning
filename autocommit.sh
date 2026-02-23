#!/bin/bash

REPO=~/kernel_dev/Linux-device-driver-learning
cd $REPO || exit

# pull first to avoid conflicts
git pull --rebase origin main >/dev/null 2>&1

# add files
git add .

# only commit if something actually changed
if ! git diff --cached --quiet
then
    git commit -m "kernel driver heartbeat : $(date '+%d-%m-%Y %H:%M:%S')"
    git push origin main >/dev/null 2>&1
fi
