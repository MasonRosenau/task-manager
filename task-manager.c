#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct date
{
    int year;
    int month;
    int day;
};

struct task
{
    int complete;
    char* name;
    struct date dueDate;
    char* category;
    struct task* next;
};

struct taskList
{
    struct task* head;
    int numTasks;
};

void promptImport(char** buffer, size_t bufferSize);
void importTasks(struct taskList* tasks, const char* filepath);
struct task* createTask(char* currLine);
void createDueDate(struct task* currTask, char* dueDate);
void printTaskList(struct taskList tasks);


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
    if the user just hits enter, this line makes buffer null
    if the user enters a file name, this removes the newline
    */
    (*buffer)[charsRead - 1] = '\0';

    return;
}

/**********************************************************************************
    ** Description: Imports tasks from a correctly formatted text file.
    ** Parameters: A premade taskList struct and the filepath of the file to import
**********************************************************************************/
void importTasks(struct taskList* tasks, const char* filepath)
{
    //attempt to open file to import tasks from
    FILE* importFile = fopen(filepath, "r");
    if(!importFile)
    {
        perror("Error opening file");
        exit(1);
    }

    //import tasks
    printf("Importing tasks from file '%s'...\n", filepath);
    char *currLine = NULL;
    size_t len = 0;
    size_t numTasks = 0;
    size_t charsRead = 0;

    //create tail pointer to make insertions easier
    struct task *tail = NULL;

    //If getline() fails to read any characters from the input stream, it returns -1 (end of file)
    while ((charsRead = getline(&currLine, &len, importFile)) != -1){

        numTasks++;

        //create a new task corresponding to the current line in file
        struct task *newTask = createTask(currLine);

        //if list is empty
        if(tasks->head == NULL){
            //set the head and the tail to this new node
            tasks->head = newTask;
            tail = newTask;
        }
        //else, list is populated
        else{
            //add new node to the list and advance the tail
            tail->next = newTask;
            tail = newTask;
        }
    }

    //print success parse message and close file
    printf("\nSuccessfully processed file '%s', and parsed data for %d tasks.\n", filepath, numTasks);
    free(currLine);
    fclose(importFile); //close the file pointer
}

/**********************************************************************************
    ** Description: Takes in a line from a file and creates a task struct from it.
    ** Parameters: The current line corresponding to a task in the file.
**********************************************************************************/
struct task* createTask(char* currLine){
    //malloc new task object
    struct task* currTask = malloc(sizeof(struct task));

    //for use with strtok_r. see https://man7.org/linux/man-pages/man3/strtok_r.3.html
    char *saveptr;
    const char *delim = "|";
    
    //complete bool
    char *token = strtok_r(currLine, delim, &saveptr);
    currTask->complete = atoi(token);

    //task name
    token = strtok_r(NULL, delim, &saveptr);
    currTask->name = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currTask->name, token);

    //task due date
    token = strtok_r(NULL, delim, &saveptr);
    createDueDate(currTask, token);

    //task category
    token = strtok_r(NULL, "\n", &saveptr);
    currTask->category = calloc(strlen(token) + 1, sizeof(char));
    strcpy(currTask->category, token);

    //set task node's next to null
    currTask->next = NULL;

    return currTask;
}

/**********************************************************************************
    ** Description: Creates a due date struct from a string when importing tasks.
    ** Parameters: The current task corresponding to due date, and the string
    of the due date of the form YYY_MM_DD.
**********************************************************************************/
void createDueDate(struct task* currTask, char* dueDate){
    //dueDate takes the form of YYYY_MM_DD

    char* token;
    char* saveptr;
    const char* delim = "_";

    //year
    token = strtok_r(dueDate, delim, &saveptr);
    currTask->dueDate.year = atoi(token);

    //month
    token = strtok_r(NULL, delim, &saveptr);
    currTask->dueDate.month = atoi(token);

    //day
    token = strtok_r(NULL, delim, &saveptr);
    currTask->dueDate.day = atoi(token);
}

/**********************************************************************************
    ** Description: Prints a task list
    ** Parameters: Task list struct
**********************************************************************************/
void printTaskList(struct taskList tasks)
{
    struct task* currTask = tasks.head;
    while(currTask != NULL)
    {
        printf("Task: %s\n", currTask->name);
        printf("Complete: %d\n", currTask->complete);
        printf("Due Date: %d_%d_%d\n", currTask->dueDate.year, currTask->dueDate.month, currTask->dueDate.day);
        printf("Category: %s\n", currTask->category);
        printf("-------------------------\n");
        currTask = currTask->next;
    }
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
    tasks.numTasks = 0;
    
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
    printTaskList(tasks);
    
    free(buffer);
    return 0;
}
