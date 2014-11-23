#!/bin/bash

if [ $1 = '-stdio' ];then
	./distvec stdio
else
	./distvec mp3_topology.txt mp3_messages.txt mp3_changes.txt
fi