
all:
	gcc module.c syncremover.c -o module && ./module test.mod
