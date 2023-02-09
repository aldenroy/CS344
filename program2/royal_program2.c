#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#define PREFIX "movies_"
//Struct for movie information.
struct movie{
    char* title;
    int year;
    char* languages;
    double rating;
    struct movie* next;
};

/* Parse the current line which is space delimited and create a
*  movie struct with the data in this line
*/
//IMPORTANT: In the movie case our token to look for is the , from a csv
struct movie *createMovie(char *currLine)
{
    struct movie* currMovie = malloc(sizeof(struct movie));

    // For use with strtok_r
    char* saveptr;

    // The first token is the title
    char *token = strtok_r(currLine, ",", &saveptr);
    currMovie->title = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currMovie->title, token);

    // The next token is the year
    //use atoi
    token = strtok_r(NULL, ",", &saveptr);
    int movieYear = atoi(token);
    currMovie->year = movieYear;

    // The next token is the langauges
    token = strtok_r(NULL, ",", &saveptr);
    currMovie->languages = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currMovie->languages, token);
    memmove(currMovie->languages, currMovie->languages + 1, strlen(currMovie->languages));
    currMovie->languages[strlen(currMovie->languages) - 1] = '\0';  //strips the last character off of the langauges string
    //languages will use a string

    // The last token is the rating
    token = strtok_r(NULL, "\n", &saveptr);
    char* excess; //stores excess from strtod
    double hold_rating = strtod(token, &excess);
    currMovie->rating = hold_rating;
    //rating will use a double and strod.

    // Set the next node to NULL in the newly created movie entry
    currMovie->next = NULL;

    return currMovie;
}


void processFile(char* fname){
    srand(time(0));
    int status;
    int random_number = rand() % 100000;    //seed the random number and get number between 0 and 99999
    char dir_name[100]; //my directory name won't be that long so I just set it to this.
    sprintf(dir_name, "royal.movies.%d", random_number);    //send new name to dirname
    mode_t mode = S_IRWXU | S_IRGRP | S_IXGRP; 
    status = mkdir(dir_name, mode);
    chmod(dir_name, 0750); //changes permissions to rwxr-x--- it will show up as drwxr-x--- cause its a directory.
    if (status == 0) {
        printf("Created directory with name %s\n\n",dir_name);
    } else {
        printf("Unable to create directory\n");
    }

    //Process code from assignment 1
    FILE* movieFile = fopen(fname, "r");


    char* currLine = NULL;
    size_t len = 0;
    int num_movies = 0;
    ssize_t nread;
    char* token;
    int lines = 0; //to skip the column headers

    // The head of the linked list
    struct movie* head = NULL;
    // The tail of the linked list
    struct movie* tail = NULL;

    // Read the file line by line
    while ((nread = getline(&currLine, &len, movieFile)) != -1)
    {
        if(lines != 0){
            // Get a new movie node corresponding to the current line
            num_movies++;
            struct movie* newNode = createMovie(currLine);

            // Is this the first node in the linked list?
            if (head == NULL)
            {
                // This is the first node in the linked link
                // Set the head and the tail to this node
                head = newNode;
                tail = newNode;
            }
            else
            {
                // This is not the first node.
                // Add this node to the list and advance the tail
                tail->next = newNode;
                tail = newNode;
            }            
        }
        lines++;
        //Lines will make sure the first line is skipped so we don't put the column headers in
    }
    free(currLine);
    fclose(movieFile);
    // printf("Processed file %s and parsed data for %d movies\n\n", fname, num_movies);   
    struct movie* curr = head;  

    //after creating directory parse structs and make txt files in the directories for each year
    int array_years[num_movies];
    int index = 0;
    int found;
    int num_years = 0;
    int curr_year;
    for(int i = 0; i < num_movies; i++){
        array_years[i] = 0;
    }

    for(int i = 0; i < num_movies; i++){
        found = 0;
        for(int j = 0; j < num_movies; j++){
            if(curr->year == array_years[j]){
                found = 1;
            }
        }
        if(found == 0){//not found meaning not in array.
            array_years[index] = curr->year;
            index++;
            num_years++;
        }

        curr = curr->next;
    }

    //create file for all the years iterate through for numyears to check
    //navigate to directory dirname then crete files, then read stuff into them then navigate back
    chdir(dir_name);
    char filename[20];
    int fd;
    FILE *file_ptr;
    for(int i = 0; i < num_years; i++){
        sprintf(filename, "%d.txt", array_years[i]);
        file_ptr = fopen(filename, "w");
        //this will give the txt files rw-r---- permissions, not sure why chmod didn't work so I tried this.
        if(chmod(filename, S_IRUSR | S_IWUSR | S_IRGRP) == -1){
            perror("chmod");
        }
        //IN FILE WRITE THE MOVIE DATA OF THAT YEAR TO IT
        //curr = head and go through years
        curr = head;
        while(curr != NULL){
            //check if currents year belongs in the file and write to the file.
            if(curr->year == array_years[i]){
                fprintf(file_ptr, "%s\n", curr->title);
            }
            curr = curr->next;
        }
        fclose(file_ptr);
    }
    chdir("..");//navigates back to main directory to repeat process again if needed.

}

void findLargestCSVFile()
{
    //Need to find largest .csv file that starts with movies_
    //DIR* setup from exploration on directories and replit page for that one.
    DIR* currDir = opendir(".");
    struct dirent* Dir;
    struct stat stat_of_dir;
    char* fname;
    int largest_size = 0;
    char name_largest[256];
    //256 is max file size

    // Go through all the entries
    while((Dir = readdir(currDir)) != NULL){
        fname = Dir->d_name;
        if((strncmp(PREFIX, Dir->d_name, strlen(PREFIX)) == 0) && (strcmp(&fname[strlen(fname) - 4], ".csv") == 0)){
            //This gets the metadata for the current file in the directory because i want to use this to find the file size.
            stat(Dir->d_name, &stat_of_dir);  
            //stat_of_dir is where the metadata is, use .st_size to get file size to compare.
            if(stat_of_dir.st_size > largest_size){
                largest_size = stat_of_dir.st_size;
                strcpy(name_largest, Dir->d_name);
            }
        }
        //printf("%s  %lu\n", Dir->d_name, Dir->d_ino);    
    }
    // Close the directory
    printf("Now processing the chosen file named %s\n", name_largest);
    processFile(name_largest);
    closedir(currDir);
}

void findSmallestCSVFile(){
    DIR* currDir = opendir(".");
    struct dirent* Dir;
    struct stat stat_of_dir;
    char* fname;
    int smallest_size = -1;
    char name_smallest[256];

    // Go through all the entries
    while((Dir = readdir(currDir)) != NULL){
        fname = Dir->d_name;
        if((strncmp(PREFIX, Dir->d_name, strlen(PREFIX)) == 0) && (strcmp(&fname[strlen(fname) - 4], ".csv") == 0)){
            //This gets the metadata for the current file in the directory because i want to use this to find the file size.
            stat(Dir->d_name, &stat_of_dir);  
            //stat_of_dir is where the metadata is, use .st_size to get file size to compare.
            if(smallest_size == -1){
                //if first file set smallest file to first file size
                smallest_size = stat_of_dir.st_size;
                strcpy(name_smallest, Dir->d_name);
            }
            else if(stat_of_dir.st_size < smallest_size){
                smallest_size = stat_of_dir.st_size;
                strcpy(name_smallest, Dir->d_name);
            }
        }  
    }
    // Close the directory

    printf("Now processing the chosen file named %s\n", name_smallest);
    //PROCESS
    processFile(name_smallest);
    closedir(currDir);
}

int enterOwnFile(){
    char fname[256];
    DIR* currDir = opendir(".");
    //open current directory to check for own file
    struct dirent* Dir;
    struct stat stat_of_dir;
    int found = 2;//not found

    printf("Enter the complete file name: ");
    scanf("%s", fname);

    while((Dir = readdir(currDir)) != NULL){
        if(strcmp(fname, Dir->d_name) == 0){
            //valid file
            found = 1;
        }
    }
    if(found == 2){
        printf("The file %s was not found. Try again\n\n", fname);
    }
    else if(found == 1){
        printf("Now processing the chosen file named %s\n", fname);
        processFile(fname);
    }
    closedir(currDir);
    return found;
}

int filePrompt(){
    int choice = 0;
    int fileFound = 0;

    printf("Which file you want to process?\n");
    printf("Enter 1 to pick the largest file\n");
    printf("Enter 2 to pick the smallest file\n");
    printf("Enter 3 to specify the name of a file\n\n");
    printf("Enter a choice from 1 to 3: ");
    scanf("%d", &choice);

    //pretty simple section, just calling the functions for each choice branch
    if(choice == 1){
        findLargestCSVFile();
    }
    else if(choice == 2){
        findSmallestCSVFile();
    }
    else if(choice == 3){
        fileFound = enterOwnFile();
    }
    else{
        printf("You did not enter a number from 1 to 3.\n");
    }
    return fileFound;
}

void promptUser(){
    int choice = 0;
    int fileFound = 0;

    //also very simple just executing all the branches based on user choices.
    while(choice != 2){
        printf("1. Select file to process\n");
        printf("2. Exit the program\n\n");
        printf("Enter a choice 1 or 2: ");
        scanf("%d", &choice);

        if(choice == 1){
            printf("\n");
            fileFound = filePrompt();
            while(fileFound == 2){
                fileFound = filePrompt();
            }
        }
        else if(choice == 2){
            printf("Exiting program.\n");
            break;
        }
        else{
            printf("Invalid choice. Please enter 1 or 2.\n\n");
        }
    }
}

int main(){
    promptUser();
}