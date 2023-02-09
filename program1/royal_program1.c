#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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


struct movie* processFile(char* filePath)
{
    // Open the specified file for reading only
    FILE* movieFile = fopen(filePath, "r");


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
    printf("Processed file %s and parsed data for %d movies\n\n", filePath, num_movies);
    return head;        

}



/*
* Print data for the given movie
*/
void printMovie(struct movie* aMovie){
    printf("%s, %s, %s, %s\n", aMovie->title, aMovie->year, aMovie->languages, aMovie->rating);
}

/*
* Print the linked list of movies
*/
void printMovieList(struct movie* list)
{
    while (list != NULL)
    {
        printMovie(list);
        list = list->next;
    }
}

void promptUser(struct movie* list){
    int num = -1;
    int i, j, a, curr_year, index, available;
    double max;
    char* array_langs[5];
    char* token;
    char* save_ptr;
    char* hold_lang;
    char language_input[20]; 
    char* language_str;//we know from the assumptions it's at max 20 characters.
    int year_placeholder = 0;
    int num_movies = 0;
    int num_unique_years = 0;
    struct movie* curr = list;
    while(curr != NULL){
        num_movies++;
        curr = curr->next;
    }
    int array_years[num_movies];    //create array store store all year numbers to help sort highest rated.
    

    while(num != 4){
        curr = list;
        int count = 0;
        printf("1. Show movies released in the specified year\n");
        printf("2. Show highest rated movie for each year\n");
        printf("3. Show the title and year of release of all movies in a specific language\n");
        printf("4. Exit from the program\n\n");
        printf("Enter a choice from 1 to 4: ");
        scanf("%d", &num);

        //this starts the condition for movies released in a certain year
        if(num == 1){
            printf("Enter the year for which you want to see movies: ");
            scanf("%d", &year_placeholder);
            
            //iterates through linked list and finds if year entered equals currs year and prints title.
            while(curr != NULL){        
                if(curr->year == year_placeholder){
                    printf("%s\n", curr->title);
                    count++;
                }
                curr = curr->next;
            }
            //if no data found this specifies that.
            if(count == 0){
                printf("No data about movies released in the year %d\n", year_placeholder);
            }
            printf("\n");   //extra line break
        }
        else if(num == 2){
            //create array of year numbers
            for(i = 0; i < num_movies; i++){
                array_years[i] = curr->year;
                curr = curr->next;
            }
            //simple selection sort to sort year numbers
            for (i = 0; i < num_movies; ++i){
                for (j = i + 1; j < num_movies; ++j){
                    if (array_years[i] > array_years[j]){
                        a = array_years[i];
                        array_years[i] = array_years[j];
                        array_years[j] = a;
                    }
                }
            }

            curr_year = array_years[0];
            num_unique_years = 0;
            num_unique_years++; //always at least the first is unique
            for(i = 1; i < num_movies; i++){
                if(array_years[i] == curr_year){
                    array_years[i] = 0;//essentially delete it.
                }
                else{
                    curr_year = array_years[i];
                    num_unique_years++;
                }
            }
            //create array to store year rating and title next to each other and populate the years into it.
            int year_sorted[num_unique_years];
            double ratings[num_unique_years];
            char* titles[num_unique_years];

            //create the year part of array of year and highest rating pairs
            index = 0;
            for(i = 0; i < num_movies; i++){
                if(array_years[i] != 0){
                    year_sorted[index] = array_years[i];
                    index++;
                }
            }

            index = 0; //reset index counter of year to check year in ascending order.
            for(j = 0; j < num_unique_years; j++){
                max = 0;
                curr = list; //reset linked list position.
                for(i = 0; i < num_movies; i++){
                    if(curr->year == year_sorted[index]){
                        if(curr->rating > max){
                            max = curr->rating;
                            titles[index] = curr->title;
                        }
                    }
                    curr = curr->next;
                }
                ratings[index] = max;
                index++; //makes sure to increment this so we can assign to the corresponding year's spot.    
            }

            for(i = 0; i < num_unique_years; i++){
                printf("%d %.1f %s\n", year_sorted[i], ratings[i], titles[i]);
            }
            printf("\n");
            

        }
        else if(num == 3){
            available = 0;
            printf("Enter the language for which you want to see movies: ");
            scanf("%s", language_input);     //read in language to search for
            curr = list;
            
            
            for(i = 0; i < num_movies; i++){
                hold_lang = calloc(strlen(curr->languages)+1, sizeof(char));
                strcpy(hold_lang, curr->languages);
                token = strtok_r(hold_lang, ";", &save_ptr);//use languages to tokenize
                
                while(token != NULL){
                    if(!strcmp(token, language_input)){  //if strcmp returns something different than 0 it means these are the same.
                        printf("%d %s\n", curr->year, curr->title);
                        available++;
                    }
                    token = strtok_r(NULL, ";", &save_ptr);
                    
                }
                curr = curr->next;
                free(hold_lang);
            }
            if(available == 0){
                printf("No data about movies released in %s.\n", language_input);
            }
            printf("\n");
        }
        else if(num == 4){
            break;
        }
        else{
            printf("You entered an incorrect choice. Try again.\n\n");
        }

    }


}

/*
*   Process the file provided as an argument to the program to
*   create a linked list of movie structs and print out the list.
*   Compile the program as follows:
*       gcc --std=gnu99 -o movies royal_program1.c
*/
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("You must provide the name of the file to process\n");
        printf("Example usage: ./movies movie_info1.txt\n");
        return EXIT_FAILURE;
    }
    if(!fopen(argv[1], "r")){
        printf("Invalid file. Please run again with a valid file.\n");
        return EXIT_FAILURE;
    }
    else{
        struct movie* list = processFile(argv[1]);
        promptUser(list);
        //printMovieList(list);        
    }

    

    return EXIT_SUCCESS;
}