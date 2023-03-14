#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

//Global Variables
//Size of the buffers
#define SIZE 1000
#define NUM_ITEMS 50

char *stop = "STOP\n";

//Buffer between Input Thread and Line Separator Thread
char buffer_1[NUM_ITEMS][SIZE];
// Number of items in the buffer
int count_1 = 0;
// Index where the input thread will put the next item
int prod_idx_1 = 0;
// Index where the square-root thread will pick up the next item
int con_idx_1 = 0;
// Initialize the mutex for buffer 1
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 1
pthread_cond_t full_1 = PTHREAD_COND_INITIALIZER;
//from replit source


//Buffer between Line Separator Thread and Plus Sign Thread
char buffer_2[NUM_ITEMS][SIZE];
// Number of items in the buffer
int count_2 = 0;
// Index where the square-root thread will put the next item
int prod_idx_2 = 0;
// Index where the output thread will pick up the next item
int con_idx_2 = 0;
// Initialize the mutex for buffer 2
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 2
pthread_cond_t full_2 = PTHREAD_COND_INITIALIZER;
//buffer 2 from replit


//Buffer between Plus Sign Thread Thread and Output Thread
char buffer_3[NUM_ITEMS][SIZE];
//Num of lines in the buffer
int count_3 = 0;
//Index where the Line Separator Thread will put the next item
int prod_idx_3 = 0;
//Index where the Plus Sign Thread will pick up the next item
int con_idx_3 = 0;
//Initialize the mutex for buffer 3
pthread_mutex_t mutex_3 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 3
pthread_cond_t full_3 = PTHREAD_COND_INITIALIZER;


/*
Input Thread
Get user input and put into buffer 1
*/
void *input_thread(void *args) {
    char input_buffer1[SIZE];
    for(int i = 0; i < NUM_ITEMS; i++) {
        memset(input_buffer1, 0, SIZE);
        fgets(input_buffer1, SIZE, stdin);
        fflush(stdin);
        
        // Lock mutex and add item to buffer
        pthread_mutex_lock(&mutex_1);
        strcpy(buffer_1[prod_idx_1], input_buffer1);
        count_1++;
        prod_idx_1++;
        pthread_mutex_unlock(&mutex_1);
        
        // Signal that buffer is no longer empty
        pthread_cond_signal(&full_1);
        
        // Check for stop condition
        if(strncmp(input_buffer1, "stop\n", 5) == 0) {
            break;
        }
    }
    return NULL;
}


/*
Line Separator Thread
Get input from buffer, remove \n char and replace with space,
put into buffer 2
*/
void *line_separator_thread(void *args){
    char input_buffer1[SIZE];
    char input_buffer2[SIZE];
    
    for(int i=0; i < NUM_ITEMS; i++){
        // Consume from buffer_1
        pthread_mutex_lock(&mutex_1);
        while(count_1==0) pthread_cond_wait(&full_1, &mutex_1);
        strcpy(input_buffer1, buffer_1[con_idx_1]);
        con_idx_1 = (con_idx_1 + 1) % SIZE;
        count_1--;
        pthread_mutex_unlock(&mutex_1);
    
        // Replace newline with space if not stop phrase
        if(strcmp(input_buffer1, stop) != 0) {
            input_buffer1[strcspn(input_buffer1, "\n")] = ' ';
        }

        // Produce to buffer_2
        pthread_mutex_lock(&mutex_2);
        while(count_2==SIZE) pthread_cond_wait(&full_2, &mutex_2);
        strcpy(buffer_2[prod_idx_2], input_buffer1);
        prod_idx_2 = (prod_idx_2 + 1) % SIZE;
        count_2++;
        pthread_cond_signal(&full_2);
        pthread_mutex_unlock(&mutex_2);

        // Check for stop phrase
        if(strcmp(input_buffer1, stop) == 0) {
            break;
        }
    }
    return NULL;
}

/*
Plus Sign Thread
Gets input from buffer 2, replaces ++ with ^, puts into buffer 3
*/
void *plus_sign_thread(void *args){
    char input_buffer3[SIZE];
    char searchBuffer[SIZE];
    for(int k=0; k<NUM_ITEMS; k++){
        //Initialize buffer
        memset(input_buffer3, '\0', SIZE);
        memset(searchBuffer, '\0', SIZE);

        //Consume
        //lock buffer 2
        pthread_mutex_lock(&mutex_2);
        //Buffer is empty, wait for the producer to signal that buffer is full
        while(count_2==0) pthread_cond_wait(&full_2, &mutex_2);
        //Get input from buffer
        strcpy(input_buffer3, buffer_2[con_idx_2]);
        //Increment consumer index
        con_idx_2++;
        count_2--;
        //unlock mutex
        pthread_mutex_unlock(&mutex_2);

        //Search string for ++
        char replace = '^';
        char* subStringPtr = strstr(input_buffer3, "++");
        while (subStringPtr != NULL) {
            //Find distance between sub-string ptr and beginning of string to determine how many bytes to copy
            int size = subStringPtr - input_buffer3;
            //Concatenate up to before replacement
            strncpy(searchBuffer, input_buffer3, size);
            //Concatenate char
            searchBuffer[strlen(searchBuffer)] = replace;
            //Concatenate rest of string if more
            if(strlen(input_buffer3) - size > 2) strncat(searchBuffer, subStringPtr+2, strlen(input_buffer3) - size - 2);
            //Replace inputBuffer with changed searchBuffer
            strcpy(input_buffer3, searchBuffer);
            memset(searchBuffer, '\0', SIZE);
            //Search for next occurrence of "++"
            subStringPtr = strstr(input_buffer3, "++");
        }

        //Produce
        //lock buffer_3
        pthread_mutex_lock(&mutex_3);
        //copy to buffer 3
        strcpy(buffer_3[prod_idx_3], input_buffer3);
        //increment counters
        prod_idx_3++;
        count_3++;
        //Signal that buffer is no longer empty
        pthread_cond_signal(&full_3);
        //unlock mutex
        pthread_mutex_unlock(&mutex_3);
        //Check for stop
        if(strcmp(input_buffer3, stop) == 0) break;
    }
    return NULL;
}

/*
Output Thread
Writes exactly 80 characters per line
*/
void *output_thread(void *args){
    char input_buffer4[SIZE];
    char print_buffer[81];
    int print_idx = 0;
    memset(print_buffer, 0, 81);

    while(1){
        memset(input_buffer4, 0, SIZE);
                
        //Consume
        //Lock buffer 3
        pthread_mutex_lock(&mutex_3);
        //Buffer is empty, wait for the producer to signal that buffer is full
        while(count_3==0) pthread_cond_wait(&full_3, &mutex_3);
        //copy from buffer 3
        strcpy(input_buffer4, buffer_3[con_idx_3]);
        con_idx_3++;
        count_3--;
        //Unlock mutex
        pthread_mutex_unlock(&mutex_3);

        if(strcmp(input_buffer4, stop) == 0) break;


        for(int input_idx = 0; input_idx < strlen(input_buffer4); input_idx++){
            //Print buffer is full
            if(print_idx == 80){
                print_buffer[80] = '\0';
                printf("%s\n", print_buffer);
                fflush(stdout);
                //Clear buffer
                memset(print_buffer, 0, 80);
                print_idx = 0;
            }
            print_buffer[print_idx] = input_buffer4[input_idx];
            print_idx++;
        }
    }
    return NULL;
}

int main(){
    
    pthread_t input_t, line_sep_t, plus_sign_t, output_t;
    //create threads
    pthread_create(&input_t, NULL, input_thread, NULL);
    pthread_create(&line_sep_t, NULL, line_separator_thread, NULL);
    pthread_create(&plus_sign_t, NULL, plus_sign_thread, NULL);
    pthread_create(&output_t, NULL, output_thread, NULL);

    //Wait for threads to terminate
    pthread_join(input_t, NULL);
    pthread_join(line_sep_t, NULL);
    pthread_join(plus_sign_t, NULL);
    pthread_join(output_t, NULL);
    
    return 0;
}