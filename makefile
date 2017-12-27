all:
	gcc -g -o graph tgraph.c graph.c list.c hash.c -lm #-Wall -Werror
