/*This is a linked list store movie information by Haofan Wang*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*REFERENCE: the createMovie(), processFile() function contains a few lines from the canvas homework 1 "Resources" section. (The student list project)
Here is the link to that file: https://replit.com/@cs344/studentsc#main.c */

/*struct for movie information*/
struct movie {
	char* title;
	int year;
	char* lan;
	double rating;
	struct movie* next;
};

/* Parse the current line which is space delimited and create a
*  movie struct with the data in this line
*/
struct movie* createMovie(char* currLine) 
{
	struct movie* currMovie = malloc(sizeof(struct movie));

	// To use strtok_r
	char* saveptr;

	//The first token is the title
	char* token = strtok_r(currLine, ",", &saveptr);
	currMovie->title = calloc(strlen(token) + 1, sizeof(char));
	strcpy(currMovie->title, token);

	//The next token is the year
	token = strtok_r(NULL, ",", &saveptr);
	currMovie->year = atoi(token);

	//The next token is the langurage
	token = strtok_r(NULL, ",", &saveptr);
	currMovie->lan = calloc(strlen(token) + 1, sizeof(char));
	strcpy(currMovie->lan, token);

	//The last token is the rating
	token = strtok_r(NULL, "\n", &saveptr);
	currMovie->rating = atof(token);

	//set the next pointer point to null
	currMovie->next = NULL;

	return currMovie;
}

/*Return a linked list of students by parsing data from each line of the specified file*/
struct movie* processFile(char* filePath) {
	//Open the specified file for reading
	FILE* movieFile = fopen(filePath, "r");

	if (movieFile == NULL) {
		printf("You didn't open the file\n");
	}

	char* currLine = NULL;
	size_t len = 0;
	ssize_t nread;
	int counter = 0;
	char* token;

	//The head of the linked list
	struct movie* head = NULL;
	//The tail of the linked list
	struct movie* tail = NULL;

	//get rid of the header
	if ((nread = getline(&currLine, &len, movieFile)) != -1) {

	}

	//Read the file line by line
	while ((nread = getline(&currLine, &len, movieFile)) != -1) {
		//Get a new student node based on the current line
		struct movie* newNode = createMovie(currLine);

		//Check if the node we just created is the head
		if (head == NULL) {
			//The node we created should be the head, so set it to be head
			head = newNode;
			tail = newNode;
		}
		else {
			//This is not the head, so connect it to the tail
			tail->next = newNode;
			tail = newNode;
		}
		counter++;
	}
	free(currLine);
	fclose(movieFile);
	printf("Processed file %s and parsed data for %d movies\n\n", filePath, counter);
	return head;
}


//search and print all movies made in a specified year
void printByYear(struct movie* list, int aYear) {

	//a record of whether at least one movie found
	int printed = 0;

	//traverse through the list to find and print matched movie
	while (list != NULL) {
		if (list->year == aYear) {
			printf("%s\n", list->title);
			list = list->next;
			printed++;
		}
		else {
			list = list->next;
		}
	}
	if (printed == 0) {
		printf("No data about movies released in the year %d\n", aYear);
	}
	printf("\n");
}


//print out the movie with the highest rating for each year
void printHighest(struct movie* list) {
	printf("\n");
	//mark the head that we can initialize later
	struct movie* head = list;
	double highest = 0;
	int aYear = 1900;

	//loop through 1900~2021, and find the highest rating movie for that year
	while(aYear < 2022) {
		list = head;  //initiate list to head for each year
		highest = 0;  //initiate highest for each year
		//find the highest rating in that year
		while (list != NULL) {

			if (list->year == aYear) {
				if (list->rating > highest) {
					highest = list->rating;
					list = list->next;
				}
				else {
					list = list->next;
				}
			}
			else {
				list = list->next;
			}
			
		}

		//find and print one movie in that year with the highest rating
		list = head;
		//print out the movie with the highest rate in the specific year
		while (list != NULL) {
			if (list->year == aYear && list->rating == highest) {
				printf("%d %.1f %s\n", list->year, list->rating, list->title);
				break;
			}
			list = list->next;
		}
		aYear++;
	}
	printf("\n");
}

//This function serch for matched substring in lan and print out corresponding movies
void printByLan(struct movie* list, char* aLan) {
	//Explain: the lan string is in the form: [...;...;...], so a langurage in this array could be at the beginning "[lan;",
	// or middle ";lan;", or the end ";lan]". What's more, if there's only one langurage, then it will be like "[lan]", so there are 4 types in totol.
	char buffer1[50] = "[";   //make a buffer for "[...]" type
	char buffer2[50] = ";";   //make a buffer for ";...]" type
	char buffer3[50] = "[";   //make a buffer for "[...;" type
	char buffer4[50] = ";";   //make a buffer for ";...;" type

	char* after1 = "]";
	char* after2 = ";";

	//turn aLan into "[aLan]" type
	int i;
	for (i = 0; i < strlen(aLan) + 1; i++) {
		buffer1[i + 1] = aLan[i];
	}
	buffer1[i] = after1[0];
	

	//turn aLan into ";aLan]" type
	for (i = 0; i < strlen(aLan) + 1; i++) {
		buffer2[i + 1] = aLan[i];
	}
	buffer2[i] = after1[0];
	
	//turn aLan into "[aLan;" type
	for (i = 0; i < strlen(aLan) + 1; i++) {
		buffer3[i + 1] = aLan[i];
	}
	buffer3[i] = after2[0];
	
	//turn aLan into ";aLan;" type
	for (i = 0; i < strlen(aLan) + 1; i++) {
		buffer4[i + 1] = aLan[i];
	}
	buffer4[i] = after2[0];
	

	char* type1;
	char* type2;
	char* type3;
	char* type4;

	//the strstr() function will check whether buffer is a substring of tmp
	char* tmp;
	int indicator = 0;
	while (list != NULL) {
		tmp = list->lan;
		type1 = strstr(tmp, buffer1);
		type2 = strstr(tmp, buffer2);
		type3 = strstr(tmp, buffer3);
		type4 = strstr(tmp, buffer4);

		//If one of the buffer is a substring, then we have the langurage
		if (type1 || type2 || type3 || type4) {
			printf("% d % s\n", list->year, list->title);
			indicator++;
		}
		list = list->next;
	}
	if (indicator == 0) {
		printf("No data about movies released in %s\n", aLan);
	}
	printf("\n");
}



int main(int argc, char *argv[]) {
	struct movie* list;
	if (argc < 2) {
		list = processFile("movies_sample_1.csv");
		//If I want to use my visual studio debugger, I need to use the floowing code instead.
		//list = processFile("/nfs/stak/users/wanghaof/vsprojects/homework1/movies_sample_1.csv");
	}
	else {
		list = processFile(argv[1]);
	}
	
	//printMovieList(list);
	
	//the interface of my program
	int choice = 0;
	printf("1. Show movies released in the specified year\n");
	printf("2. Show highest rated movie for each year\n");
	printf("3. Show the title and year of release of all movies in a specific language\n");
	printf("4. Exit from the program\n\n");

	printf("Enter a choce from 1 to 4: ");
	scanf("%d", &choice);

	while (choice != 4) {
		if (choice == 1) {
			int aYear = 0;
			printf("Enter the year for which you wnat to see movies: ");
			scanf("%d", &aYear);
			printByYear(list, aYear);
		}
		else if (choice == 2) {
			printHighest(list);
		}
		else if (choice == 3) {
			char* aLan;
			printf("Enter the language for which you wnat to see movies: ");
			scanf("%s", aLan);
			printByLan(list, aLan);
		}
		else {
			printf("You entered an incorrect choice. Try again.\n\n");
		}
		printf("1. Show movies released in the specified year\n");
		printf("2. Show highest rated movie for each year\n");
		printf("3. Show the title and year of release of all movies in a specific language\n");
		printf("4. Exit from the program\n\n");

		printf("Enter a choce from 1 to 4: ");
		scanf("%d", &choice);
	}

	return EXIT_SUCCESS;
}

