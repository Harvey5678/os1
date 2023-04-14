//This is the keygen file for assignment 5
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int main(int argc, char* argv[]) {
	srand(time(0));
	int counter;

	//if no user specified key length, the length will be 5
	if (argc != 2) {
		counter = 5;
	}
	else {
		counter = atoi(argv[1]);
	}
	 
	char a;
	int x;
	int i = 0;
	char key[999999] = {'\0'};

	//generate a random number that 0 <= num < 27, and cast number to char to get random characters
	for (i; i < counter; i++) {
		
		x = rand() % 27;
		
		if (x == 0) {
			a = ' ';
		}
		else {
			x = x + 64;
			a = (char)x;
		}
		key[i] = a;
	}
	//output the key
	printf("%s\n", &key);
	return 0;
}