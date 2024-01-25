#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

struct date
{
    int year;
    int month;
    int day;
};

struct task
{
    bool complete;
    char* name;
    struct date dueDate;
    char* category;
    struct task* next;
};

struct taskList
{
    struct task* head;
};

/**********************************************************************************
    ** Description: Prompt's the user on whether they would like to import tasks
    from a file or start fresh with no tasks.
    ** Parameters: Takes in a buffer, the bufferSize, and a variable to keep track
    of the number of characters read.
**********************************************************************************/
void promptImport(char** buffer, size_t bufferSize)
{
    //can be absolute or relative filepath ending in the filename of the file you want to import
    printf("Enter a filepath/name to import tasks from!\nOtherwise, just hit 'Enter' to start fresh.\n");

    //take in user input; getline can dynamically resize buffer to >32
    size_t charsRead = getline(buffer, &bufferSize, stdin);
    if(charsRead == -1)
    {
        perror("Error reading input");
        exit(1);
    }

    /*
    getline will always return a newline at the end of the input.
    if the user just hits enter, this line creates
    if the user enters a file name, this removes the newline
    */
    (*buffer)[charsRead - 1] = '\0';

    return;
}

/**********************************************************************************
    ** Description: Imports tasks from a correctly formatted text file.
    ** Parameters: 
**********************************************************************************/
void importTasks(struct taskList* list, const char* filepath)
{
    //attempt to open file to import tasks from
    FILE* importFile = fopen(filepath, "r");
    if(!importFile)
    {
        perror("Error opening file");
        exit(1);
    }

    printf("Importing tasks from file '%s'...\n", filepath);
        
    fclose(importFile); //close the file pointer
}

/**********************************************************************************
    ** Description: Main Function 
**********************************************************************************/
int main(int argc, char *argv[])
{   
    char* buffer = NULL;
    size_t bufferSize = 32;

    //create buffer
    buffer = (char *)malloc(bufferSize * sizeof(char));
    if(buffer == NULL)
    {
        perror("Unable to allocate buffer");
        exit(1);
    }
    
    //create taskList
    struct taskList tasks;
    tasks.head = NULL;
    
    //prompt user to import tasks or start fresh
    promptImport(&buffer, bufferSize);

    //if the first character in buffer exists (isn't just a \0)
    if(buffer[0] != '\0')
    {
        //open file and import it's tasks
        importTasks(&tasks, buffer);
    }
    else
    {
        //otherwise start fresh with 0 tasks
        printf("Starting fresh!\n");
        //create a task
    }
    
    free(buffer);
    return 0;
}
