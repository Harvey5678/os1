//This is project 4 by Haofan Wang
//date 2/21/2022
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>

#define LARGE_SIZE 50050  //let the buffer space large enough to store 50 lines of 1000 characters
#define P_SIZE 80  //change this number to change the number of character in output lines

struct targs {
    sem_t* sem; /* semaphore for seperator buffer*/
    sem_t* sem1; //semaphore for plus sign converter buffer
    sem_t* sem2;  //semaphore for consumer buffer
    char* shm; /* seperator buffer */
    char* shm1; //consumer buffer
    char* shm2;  //plus sign converter buffer
    /* size of shared memory */
    size_t shm_sz; 
    size_t shm_sz1;
    size_t shm_sz2;
};

//This thread read characters from stdin and pass them to seperator buffer
void*
producer(void* targs)
{
    //create local variable for the target struct
    sem_t* sem = ((struct targs*)targs)->sem;
    char* shm = ((struct targs*)targs)->shm;
    size_t shm_sz = ((struct targs*)targs)->shm_sz;

    size_t idx = 0;
    while (1) {
        int val;
        do {
            /* Loop while buffer is full */
            sem_getvalue(sem, &val);
        } while (val >= shm_sz);

        /* Get character and put in a local buffer*/
        size_t b_len = 1001;
        ssize_t num_read;
        
        char get_buffer[1001];
        char* input = get_buffer;

        //read input from stdin into input
        num_read = getline(&input, &b_len, stdin);

        //convert the STOP signal to a special character
        if (strncmp(input, "STOP\n", 5) == 0) {
            get_buffer[0] = '\t';
            num_read = 1;
        }
        
        //put characters we get into seperator buffer
        int count = 0;
        while (count < num_read) {
            shm[idx] = get_buffer[count];
            //increment the semaphorer
            sem_post(sem);
            count++;
            idx = idx + 1 < shm_sz ? idx + 1 : 0;
        }
        
        //exit the thread after receiving stop sign
        if (get_buffer[0] == '\t') {
            break;
        }
    }
    return targs;
}

//This thread convert \n to space character
void*
seperator(void* targs)
{
    //create local variables for target struct
    sem_t* sem = ((struct targs*)targs)->sem;
    sem_t* sem1 = ((struct targs*)targs)->sem1;
    char* shm = ((struct targs*)targs)->shm;
    size_t shm_sz = ((struct targs*)targs)->shm_sz;
    char* shm2 = ((struct targs*)targs)->shm2;
    size_t shm_sz2 = ((struct targs*)targs)->shm_sz2;

    size_t idx = 0;
    while (1) {
        /* wait for data to be available and get it */
        sem_wait(sem);
        char c = shm[idx];

        //If detect \n, replace it with a space and send to next buffer
        if (c == '\n') {
            shm2[idx] = ' ';
        }
        //regular case, pass the character to next buffer
        else {
            shm2[idx] = c;
        }

        idx = idx + 1 < shm_sz ? idx + 1 : 0;
        //increase the semaphore for the next buffer
        sem_post(sem1);

        //if stop sign is detected, exit the thread
        if (c == '\t') {
            break;
        }
    }
    return targs;
}

//This thread convert ++ to ^
void*
plus_change(void* targs)
{
    //create local variables for target struct
    sem_t* sem1 = ((struct targs*)targs)->sem1;
    sem_t* sem2 = ((struct targs*)targs)->sem2;
    char* shm2 = ((struct targs*)targs)->shm2;
    size_t shm_sz2 = ((struct targs*)targs)->shm_sz2;
    char* shm1 = ((struct targs*)targs)->shm1;
    size_t shm_sz1 = ((struct targs*)targs)->shm_sz1;

    size_t idx = 0;  //index for shm2
    size_t idx1 = 0; //index for shm1
    char tempBuf[3];
    int emp_buf = 1;  //this is a flag to show whether the tempBuf should be empty
    int changeFlag = 0;  //this is a flag to show whetehr we have converted ++ to ^

    while (1) {
        /* wait for data to be available and get it */
        sem_wait(sem1);
        char c = shm2[idx];

        //move the character from last iteration to the second place, and place the character from this iteration to the first place
        tempBuf[1] = tempBuf[0];
        tempBuf[0] = c;

        //if detect stop sign, finish processing and pass stop sign and exit thread
        if (tempBuf[0] == '\t') {  //if we get stop sign at this iteration
            shm1[idx1] = '\t';
            idx1 = idx1 + 1 < shm_sz1 ? idx1 + 1 : 0;
            sem_post(sem2);
            break;
        }
        else if (tempBuf[1] == '\t') {  //if we get stop sign for some reason from the last iteration
            //finish processing
            shm1[idx1] = tempBuf[0];
            idx1 = idx1 + 1 < shm_sz1 ? idx1 + 1 : 0;
            sem_post(sem2);
            //pass the stop sign
            shm1[idx1] = tempBuf[1];
            idx1 = idx1 + 1 < shm_sz1 ? idx1 + 1 : 0;
            sem_post(sem2);
            break;
        }

        //if the tempBuf should not be empty, which is the regular case
        if (emp_buf == 0) {
            //if we get ++, convert to ^ and set the flag
            if (strncmp(tempBuf, "++", 2) == 0) {
                tempBuf[1] = '^';
                changeFlag = 1;
            }

            //pass the character from last itration to the next buffer
            shm1[idx1] = tempBuf[1];
            idx1 = idx1 + 1 < shm_sz1 ? idx1 + 1 : 0;
            sem_post(sem2);
        }

        //if we converted ++, then the tempBuf should be treated as empty
        if (changeFlag == 1) {
            emp_buf = 1;
            changeFlag = 0;
        }
        //if we didn't change ++, then the tempBuf shouldn't be empty
        else {
            emp_buf = 0;
        }

        idx = idx + 1 < shm_sz2 ? idx + 1 : 0;
    }
    return targs;
}

//This thread wait for 80 characters and print them out
void*
consumer(void* targs)
{
    //create local variables for target struct
    sem_t* sem2 = ((struct targs*)targs)->sem2;
    char* shm1 = ((struct targs*)targs)->shm1;
    size_t shm_sz1 = ((struct targs*)targs)->shm_sz1;

    char this_buffer[81];  //local buffer to contain the string to print out
    int this_count = 0;
    size_t idx = 0;
    while (1) {
        /* wait for data to be available and get it */
        sem_wait(sem2);
        char c = shm1[idx];

        //get data into local buffer
        this_buffer[this_count] = c;
        this_count++;

        //if there're enouth character to print and we don't have stop sign, print out the line
        if (this_count >= P_SIZE && c != '\t') {
            this_buffer[this_count] = '\n';  //add new line character after 80 characters
            printf("%s", this_buffer);
            this_count = this_count - P_SIZE; //initiate counter to reload buffer
        }
        else if (c == '\t') {  //detect stop sign, exit the program
            exit(0);
        }
        
        idx = idx + 1 < shm_sz1 ? idx + 1 : 0;

    }
    return targs;
}


int
main(int argc, char* argv[])
{
    //create the target struct and corresponding items.
    struct targs targs;
    sem_t sem;
    sem_t sem1;
    sem_t sem2;
    targs.sem = &sem;
    targs.sem1 = &sem1;
    targs.sem2 = &sem2;
    //initialize semaphores
    sem_init(&sem, 0, 0);
    sem_init(&sem1, 0, 0);
    sem_init(&sem2, 0, 0);

    //buffers for threads
    char buf[LARGE_SIZE];
    char buf1[LARGE_SIZE];
    char buf2[LARGE_SIZE];
    //make shared space points to buffers created above
    targs.shm = buf;
    targs.shm_sz = sizeof buf;
    targs.shm1 = buf1;
    targs.shm_sz1 = sizeof buf1;
    targs.shm2 = buf2;
    targs.shm_sz2 = sizeof buf2;

    pthread_t prod, cons, sepe, plus;

    //create threads
    pthread_create(&prod, NULL, &producer, &targs);
    pthread_create(&sepe, NULL, &seperator, &targs);
    pthread_create(&plus, NULL, &plus_change, &targs);
    pthread_create(&cons, NULL, &consumer, &targs);
    
    /* Exit from all thread */
    pthread_join(sepe, NULL);
    pthread_join(plus, NULL);
    pthread_join(cons, NULL);
}
