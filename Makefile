# Makefile
# Project 1: Quash - A Simple Command Shell
# Authors: Caitlyn Lynch and Kristian Morgan

shell: shell.c
	gcc -Wall -g -std=c99 -o shell shell.c

clean:
	rm -f shell
