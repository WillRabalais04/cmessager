#!/bin/sh

gcc server.c -o server
gcc client.c -o client

./server