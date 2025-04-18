CC = g++
CFLAGS = -Wall -g -std=c++11 -pthread
TARGETS = hw2_Q1 hw2_Q2 hw2_Q3

all: $(TARGETS)

hw2_Q1: hw2_Q1.cpp
	$(CC) $(CFLAGS) hw2_Q1.cpp -o hw2_Q1

hw2_Q2: hw2_Q2.cpp
	$(CC) $(CFLAGS) hw2_Q2.cpp -o hw2_Q2

hw2_Q3: hw2_Q3.cpp
	$(CC) $(CFLAGS) hw2_Q3.cpp -o hw2_Q3

clean:
	rm -f $(TARGETS)