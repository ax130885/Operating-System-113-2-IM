CC = gcc
CFLAGS = -Wall -g
TARGETS = hw1_Q1 hw1_Q2 hw1_Q3

all: $(TARGETS)

hw1_Q1: hw1_Q1.c
	gcc $(CFLAGS) -o hw1_Q1 hw1_Q1.c

hw1_Q2: hw1_Q2.c
	gcc $(CFLAGS) -o hw1_Q2 hw1_Q2.c

hw1_Q3: hw1_Q3.c
	gcc $(CFLAGS) -lrt -o hw1_Q3 hw1_Q3.c

clean:
	rm -f $(TARGETS)