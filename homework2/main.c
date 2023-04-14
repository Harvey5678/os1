/*This is the homework 2 by Haofan Wang*/

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#define PREFIX "movies_"

/*REFERENCE: the createMovie(), processFile() function contains a few lines from the canvas homework 1 "Resources" section. (The student list project)
Here is the link to that file: https://replit.com/@cs344/studentsc#main.c */

/*struct for movie information*/
struct movie {
	char* title;
	int year;
	char* lan;
	double rating;
	char* info;
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
	
	// copy the entile information
	currMovie->info = calloc(strlen(currLine) + 1, sizeof(char));
	strcpy(currMovie->info, currLine);
	
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
	
	return head;
}



int main(int argc, char *argv[]) {
	char* extcsv;
	int length;
	int file_descriptor;
	DIR* currDir = opendir(".");
	struct dirent* aDir;
	off_t curr_max;
	off_t curr_min;
	struct stat dirStat;
	int i = 0;
	int r = 0;
	int y = 0;
	srand(time(NULL));
	char entryName[256]; //POSIX specifies the max file-name length to be 255 bytes 
	char choiceName[256];
	char dirName[256];
	char buffer[512];
	char myDir[512];
	char myFile[512];
	struct movie* list;
	struct movie* head;
	closedir(currDir);  //we opened the directory before
	
	
	//the interface of my program
	int choice = 0;
	int choice2 = 0;
	int choice3 = 0;
	printf("1. Select file to process\n");
	printf("2. Exit the program\n\n");

	printf("Enter a choce 1 or 2: ");
	scanf("%d", &choice);

	while (choice != 2) {
		i = 0;
		if (choice == 1) {   //show user the second menu
			while (choice3 == 0) {   //when user insert an invalid filename, the program will show the second menu instead of the first one
				printf("\nWhich file you wnat to process?\n");
				printf("Enter 1 to pick the largest file\n");
				printf("Enter 2 to pick the smallest file\n");
				printf("Enter 3 to specify the name of a file\n\n");
				printf("Enter a choice from 1 to 3: ");
				scanf("%d", &choice2);

				if (choice2 == 1) {
					choice3 = 1;
					currDir = opendir(".");

					//Some of the following code come from the exploration
					//go through all the netries
					while ((aDir = readdir(currDir)) != NULL) {
						//make sure the prefix is correct and it's a csv file
						length = strlen(aDir->d_name);
						if (length >= 4) {
							//extcsv will store the extention of the file
							extcsv = &aDir->d_name[length - 4];
							if (strncmp(PREFIX, aDir->d_name, strlen(PREFIX)) == 0 && strcmp(extcsv, ".csv") == 0) {
								//get meta-data for the current entry
								stat(aDir->d_name, &dirStat);
								//compare the current file size to the current max, and update filename when needed
								if (i == 0 || dirStat.st_size > curr_max) {
									curr_max = dirStat.st_size;
									memset(entryName, '\0', sizeof(entryName));
									strcpy(entryName, aDir->d_name);
								}
								i++;
							}
						}

					}
					//Close the directory
					closedir(currDir);
					printf("Now processing the chosen file named %s \n", entryName);

					//process the corresponding file
					list = processFile(entryName);
					head = list;
					//create the directory
					r = rand() % 100000;  //make random number between 0 to 99999
					memset(myDir, '\0', sizeof(myDir));
					memset(dirName, '\0', sizeof(dirName));
					memset(buffer, '\0', sizeof(buffer));
					sprintf(myDir, "./wanghaof.movies.%d", r);  //make the actual directory path and name
					sprintf(dirName, "wanghaof.movies.%d", r);
					mkdir(myDir, 0750);
					strcpy(buffer, myDir);   //originally I ran into a bug related to myDir, so I created a copy of myDir named buffer

					printf("Created directory with name %s\n\n", dirName);
					//create directories based on years. Creat a directory for every year, the delect the directory with no content
					for (y = 1900; y < 2022; y++) {
						list = head;
						i = 0;
						//make a file corresponding to the year
						memset(myFile, '\0', sizeof(myFile));

						sprintf(myFile, "%s/%d.txt", buffer, y);
						//create the file
						file_descriptor = open(myFile, O_RDWR | O_CREAT | O_TRUNC, 0640);
						while (list != NULL) {
							if (list->year == y) {
								//write information to the file
								write(file_descriptor, list->info, strlen(list->info));
								list = list->next;
								i++;
							}
							else {
								list = list->next;
							}

						}
						close(file_descriptor);
						if (i == 0) {
							//delete file
							remove(myFile);
						}
					}
				}
				else if (choice2 == 2) {
					choice3 = 1;
					currDir = opendir(".");

					//go through all the netries
					while ((aDir = readdir(currDir)) != NULL) {
						//make sure the prefix is correct and it's a csv file
						length = strlen(aDir->d_name);
						if (length >= 4) {
							extcsv = &aDir->d_name[length - 4];
							if (strncmp(PREFIX, aDir->d_name, strlen(PREFIX)) == 0 && strcmp(extcsv, ".csv") == 0) {
								//get meta-data for the current entry
								stat(aDir->d_name, &dirStat);
								//compare the current file size to the current max, and update filename when needed

								if (i == 0 || dirStat.st_size < curr_min) {
									curr_min = dirStat.st_size;
									memset(entryName, '\0', sizeof(entryName));
									strcpy(entryName, aDir->d_name);
								}
								i++;
							}
						}

					}
					//Close the directory
					closedir(currDir);
					printf("Now processing the chosen file named %s \n", entryName);

					//basically the same procedure as before
					list = processFile(entryName);
					head = list;
					//create the directory
					r = rand() % 100000;
					memset(myDir, '\0', sizeof(myDir));
					memset(dirName, '\0', sizeof(dirName));
					memset(buffer, '\0', sizeof(buffer));
					sprintf(myDir, "./wanghaof.movies.%d", r);  //make the actual directory path and name
					sprintf(dirName, "wanghaof.movies.%d", r);
					mkdir(myDir, 0750);
					strcpy(buffer, myDir);  //originally I ran into a bug related to myDir, so I created a copy of myDir named buffer

					printf("Created directory with name %s\n\n", dirName);
					//create directories based on years
					for (y = 1900; y < 2022; y++) {
						list = head;
						i = 0;
						//make a file corresponding to the year
						memset(myFile, '\0', sizeof(myFile));

						sprintf(myFile, "%s/%d.txt", buffer, y);  //make the file name

						file_descriptor = open(myFile, O_RDWR | O_CREAT | O_TRUNC, 0640);  //create the file
						while (list != NULL) {
							if (list->year == y) {
								//write data to the file
								write(file_descriptor, list->info, strlen(list->info));
								list = list->next;
								i++;
							}
							else {
								list = list->next;
							}

						}
						close(file_descriptor);
						if (i == 0) {
							//delete file
							remove(myFile);
						}
					}
				}

				else if (choice2 == 3) {
					choice3 = 1;
					i = 0;
					printf("Enter the complete file name: ");
					scanf("%s", choiceName);
					currDir = opendir(".");
					while ((aDir = readdir(currDir)) != NULL) {
						if (strcmp(choiceName, aDir->d_name) == 0) {
							memset(entryName, '\0', sizeof(entryName));
							strcpy(entryName, aDir->d_name);
							i++;
						}
					}
					//if the file was not found, then set choice3 to 0, so the program will loop back and display the menu
					if (i == 0) {
						printf("The file %s was not found. Try again\n", choiceName);
						choice3 = 0;

					}
					else {
						printf("Now processing the chosen file named %s \n", choiceName);

						list = processFile(choiceName);
						head = list;
						//create the directory
						r = rand() % 100000;
						memset(myDir, '\0', sizeof(myDir));
						memset(dirName, '\0', sizeof(dirName));
						memset(buffer, '\0', sizeof(buffer));
						sprintf(myDir, "./wanghaof.movies.%d", r);  //make the actual directory path and name
						sprintf(dirName, "wanghaof.movies.%d", r);
						mkdir(myDir, 0750);
						strcpy(buffer, myDir);  //originally I ran into a bug related to myDir, so I created a copy of myDir named buffer

						printf("Created directory with name %s\n\n", dirName);
						//create directories based on years
						for (y = 1900; y < 2022; y++) {
							list = head;
							i = 0;
							//make a file corresponding to the year
							memset(myFile, '\0', sizeof(myFile));

							sprintf(myFile, "%s/%d.txt", buffer, y);
							//create the file
							file_descriptor = open(myFile, O_RDWR | O_CREAT | O_TRUNC, 0640);
							while (list != NULL) {
								if (list->year == y) {
									//write the data into each file
									write(file_descriptor, list->info, strlen(list->info));
									list = list->next;
									i++;
								}
								else {
									list = list->next;
								}

							}
							close(file_descriptor);
							if (i == 0) {
								//delete file
								remove(myFile);
							}
						}
					}



					memset(choiceName, '\0', sizeof(choiceName));
					closedir(currDir);
				}
				else {
					printf("Please enter a valid option.\n\n");
				}
			}
			choice3 = 0;
			
		}
		else {
			printf("You entered an incorrect choice. Try again.\n\n");
		}
		printf("1. Select file to process\n");
		printf("2. Exit the program\n\n");

		printf("Enter a choce 1 or 2: ");
		scanf("%d", &choice);
	}

	return EXIT_SUCCESS;
}

