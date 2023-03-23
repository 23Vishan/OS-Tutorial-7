sigtrap:
	gcc -o process sigtrap.c
	./process

1:
	gcc -o question1 one.c
	./question1

2:
	gcc -o question2 two.c
	./question2

3:
	gcc -o question3 three.c
	./question3
4:
	gcc -o question4 four.c
	./question4
5:
	gcc -o question5 five.c
	./question5

all:
	gcc -o question5 2.c
	./question5