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

FILE* promptImport(char** buffer, size_t bufferSize, int retry);
void importTasks(struct taskList* tasks, const char* filepath, FILE* importFile);
struct task* createTaskFromFile(char* currLine);
void createDueDate(struct task* currTask, char* dueDate);
void printTaskList(struct taskList tasks);


/**********************************************************************************
    ** Description: Prompt's the user on whether they would like to import tasks
    from a file or start fresh with no tasks.
    ** Parameters: Takes in a buffer, the bufferSize, a variable to keep track
    of the number of characters read, and a retry variable to indicate if this is
    the first time the user is prompted or if they are retrying after an error.
**********************************************************************************/
FILE* promptImport(char** buffer, size_t bufferSize, int retry)
{
    //can be absolute or relative filepath ending in the filename of the file you want to import
    //prompt user to enter file name again, as first attempt failed
    if (retry == 1)
    {
        printf("|-------------------------------------------\n|\n|   It looks like the file name you\n|   entered couldn't be opened.\n|\n|   Please try again below, either\n|   by hitting enter or entering\n|   a new file name.\n|\n|   : ");
    }
    else
    {
        printf("|-------------------------------------------\n|\n|   Task Manager: Welcome!\n|\n|   To begin, you have 2 options:\n|\n|   1. To import tasks, type the name\n|      of a file from which to\n|      import tasks, and hit enter.\n|\n|      OR\n|\n|   2. To start with no tasks (from\n|      scratch), simply hit enter.\n|\n|   For more information, type 'help'\n|   and hit enter.\n|\n|   : ");
    }

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

    //if the user just hits enter, return NULL to start fresh
    if(*buffer[0] == '\0')
    {
        return NULL;
    }
    else
    {
        //attempt to open file
        FILE* importFile = fopen(*buffer, "r");
        //if file couldn't be opened, return NULL
        if(!importFile)
        {
            return NULL;
        }
        //otherwise, return the file pointer
        else
        {
            return importFile;
        }
    }
}

/**********************************************************************************
    ** Description: Imports tasks from a correctly formatted text file.
    ** Parameters: A premade taskList struct and the filepath of the file to import
**********************************************************************************/
void importTasks(struct taskList* tasks, const char* filepath, FILE* importFile)
{
    //import tasks
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
        struct task *newTask = createTaskFromFile(currLine);

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
    fclose(importFile); //close the file pointer;
}

/**********************************************************************************
    ** Description: Takes in a line from a file and creates a task struct from it.
    ** Parameters: The current line corresponding to a task in the file.
**********************************************************************************/
struct task* createTaskFromFile(char* currLine){
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
    ** Description: Creates a task from user input and inserts it into tasks
    ** Parameters: taskList struct to insert the task into
**********************************************************************************/
void createTaskFromUser(struct taskList* tasks)
{
    return;
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
    FILE* importFile = promptImport(&buffer, bufferSize, 0);
    
    //if file couldn't be opened, and user didn't just hit enter to start fresh
    while(importFile == NULL && buffer[0] != '\0')
    {
        //keep reprompting user
        importFile = promptImport(&buffer, bufferSize, 1);
    }

    //if importFile is null here, user is starting fresh
    if(!importFile)
    {
        printf("Starting fresh!\n");
        //create a task (tasks should be null)
        createTaskFromUser(&tasks);

    }
    else //user is importing tasks
    {
        //import tasks from importFile
        importTasks(&tasks, buffer, importFile);
    }
    printTaskList(tasks);
    printf("show main menu now. 1+ task should be in\n");
        
    free(buffer);
    return 0;
}
