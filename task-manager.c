#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

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
    int incompleteTasks;
};

FILE* promptImport(char** buffer, size_t bufferSize, int retry);
void importTasks(struct taskList* tasks, FILE* importFile);
struct task* createTaskFromFile(char* currLine);
void createDueDate(struct task* currTask, char* dueDate);
void viewTasks(struct taskList* tasks);
void createTaskFromUser(struct taskList* tasks);
void freeTaskList(struct taskList* tasks);
void completeTask(struct taskList* tasks);
void exportTasks(struct taskList* tasks);

/**********************************************************************************
    ** Description: Prompt's user whether they would like to import tasks
    from a file or start fresh by creating a task. One of these must be done.
    ** Parameters: A buffer, buffer size, and a retry variable to indicate if this is
    the first time the user is prompted or if they are retrying after an error.
**********************************************************************************/
FILE* promptImport(char** buffer, size_t bufferSize, int retry)
{
    //if retry flag is set, prompt user to enter file name again, as first attempt failed
    if (retry == 1)
    {
        printf("|--------------------------------------------------\n|\n|   It looks like the file name you\n|   entered couldn't be opened.\n|\n|   Please try again below, either\n|   by hitting enter or entering\n|   a new file name.\n|\n|   : ");
    }
    //otherwise, prompt user to import tasks or start fresh
    else
    {
        printf("|--------------------------------------------------\n|\n|   Task Manager: Welcome!\n|\n|   To begin, you have 2 options:\n|\n|   1. To import tasks, type the name of a file\n|      from which to import tasks, and hit enter.\n|\n|      OR\n|\n|   2. To start with no tasks (from scratch),\n|      simply hit enter.\n|\n|   To see how import files should be formatted,\n|   type 'help' and hit enter.\n|\n|   : ");
    }

    //take in user input; getline can dynamically resize buffer
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

    //otherwise, if the user types 'help', call microservice for example formatting and reprompt with retry flag cleared
    else if(strcmp(*buffer, "help") == 0)
    {
        system("clear");
        printf("|--------------------------------------------------\n|   Generating 5 random tasks...\n|--------------------------------------------------\n");

        //open pipe.txt for writing
        FILE* pipeWrite = fopen("task-pipe.txt", "w");
        if(!pipeWrite)
        {
            perror("Error opening pipe");
            exit(1);
        }
        //write "gen" to pipe.txt to signal microservice to generate tasks
        fprintf(pipeWrite, "gen");
        fclose(pipeWrite);

        //wait for microservice to generate tasks
        sleep(5);

        //open pipe.txt for reading
        FILE* pipeRead = fopen("task-pipe.txt", "r");
        if(!pipeRead)
        {
            perror("Error opening pipe");
            exit(1);
        }

        //create buffer for reading from pipe
        char* taskBuffer = (char *)malloc(bufferSize * sizeof(char));
        memset(taskBuffer, '\0', bufferSize);
        size_t taskCharsRead = 0;

        //read and print line by line until end of file
        while((taskCharsRead = getline(&taskBuffer, &bufferSize, pipeRead)) != -1)
        {
            printf("%s", taskBuffer);
        }
        printf("\n|--------------------------------------------------\n|   Above and between the lines is exactly\n|   how any input file should be formatted\n|   (including the empty line at the end).\n|\n|   Go ahead! Copy the above contents\n|   into a new file, then type in that\n|   file name below to import those tasks!\n");

        //free buffer and close pipe
        free(taskBuffer);
        fclose(pipeRead);

        return promptImport(buffer, bufferSize, 0);
    }

    //otherwise, attempt to open the file entered
    else
    {
        //attempt to open file
        FILE* importFile = fopen(*buffer, "r");
        //if file couldn't be opened, return NULL
        if(!importFile)
        {
            //this will indicate that the file couldn't be opened in main()
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
    ** Parameters: A premade taskList struct and the file to import from
**********************************************************************************/
void importTasks(struct taskList* tasks, FILE* importFile)
{
    char *currLine = NULL;
    size_t len = 0;
    size_t charsRead = 0;

    //create tail pointer to make insertions easier
    struct task *tail = NULL;

    //if getline() fails to read any characters from the input stream, it returns -1 (end of file)
    while ((charsRead = getline(&currLine, &len, importFile)) != -1){

        tasks->numTasks++;

        //create a new task corresponding to the current line in file
        struct task *newTask = createTaskFromFile(currLine);
        
        //if this imported task is incomplete, increment incompleteTasks
        if(newTask->complete == 0){
            tasks->incompleteTasks++;
        }

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

    //print success message
    system("clear");
    printf("|--------------------------------------------------\n|   Imported %d tasks!\n", tasks->numTasks);

    //free buffer and file
    free(currLine);
    fclose(importFile);
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
    ** Parameters: Task list struct to be printed
**********************************************************************************/
void viewTasks(struct taskList* tasks)
{
    //no tasks to print, return to main menu
    if(tasks->numTasks == 0)
    {
        system("clear");
        printf("|--------------------------------------------------\n|   There are no tasks to view.\n|   Please create a task first!\n");
        return;
    }

    system("clear");
    printf("|--------------------------------------------------\n|\n|   Task Manager: View Tasks\n|\n");

    //traverse task list, printing each task and its attributes in correct format
    struct task* currTask = tasks->head;
    while(currTask != NULL)
    {
        printf("|   %s\n", currTask->name);
        if(currTask->complete == 1)
        {
            printf("|   Status: Complete\n");
        }
        else
        {
            printf("|   Status: Incomplete\n");
        }
        if(strcmp(currTask->category, "None") != 0)
        {
            printf("|   Category: %s\n", currTask->category);
        }
        printf("|   Due: %d/%d/%d\n|\n", currTask->dueDate.month, currTask->dueDate.day, currTask->dueDate.year);
        
        currTask = currTask->next;
    }
}

/**********************************************************************************
    ** Description: Prompts user for attributes of a task and creates the task.
    Note: Tasks resulting from this function are incomplete by default.
    ** Parameters: taskList struct to insert the task into
**********************************************************************************/
void createTaskFromUser(struct taskList* tasks)
{
    //create a new task
    struct task* newTask = malloc(sizeof(struct task));
    newTask->next = NULL;
    newTask->complete = 0;

    //fill out newTask info from user
    size_t bufferSize = 32;
    size_t charsRead = 0;

    //NAME: ask user for name
    system("clear");
    printf("|--------------------------------------------------\n|\n|   Task Manager: Create Task\n|\n|   Please enter the NAME of the task\n|   you would like to create, and hit\n|   enter.\n|\n|   To cancel, type 'cancel' and hit enter.\n|\n|   : ");
    
    //NAME: malloc
    newTask->name = (char *)malloc(bufferSize * sizeof(char));
    memset(newTask->name, '\0', bufferSize);

    //NAME: get user input
    charsRead = getline(&(newTask->name), &bufferSize, stdin);
    (newTask->name)[charsRead - 1] = '\0';

    //check if input is 'cancel'
    if(strcmp(newTask->name, "cancel") == 0)
    {
        free(newTask->name);
        free(newTask);
        return; //cancel create task operation
    }

    //CATEGORY: ask user for category
    printf("|--------------------------------------------------\n|\n|   Task Manager: Create Task\n|\n|   Please enter the CATEGORY of the\n|   task you would like to create, and\n|   hit enter.\n|\n|   To omit a category for this task,\n|   simply hit enter.\n|\n|   : ");

    //CATEGORY: malloc
    newTask->category = (char *)malloc(bufferSize * sizeof(char));
    memset(newTask->category, '\0', bufferSize);

    //CATEGORY: get user input
    charsRead = getline(&(newTask->category), &bufferSize, stdin);
    (newTask->category)[charsRead - 1] = '\0';

    //GATEGORY: if omitted, set to "None"
    if((newTask->category)[0] == '\0')
    {
        strcpy(newTask->category, "None");       
    }

    //DUE DATE: malloc buffer
    char* buffer = (char *)malloc(bufferSize * sizeof(char));
    memset(buffer, '\0', bufferSize);

    //DUE DATE: ask user for YEAR
    printf("|--------------------------------------------------\n|\n|   Task Manager: Create Task\n|\n|   Please enter the YEAR that this\n|   task is due.\n|\n|   : ");
    
    //DUE DATE: get year from user
    charsRead = getline(&buffer, &bufferSize, stdin);
    (buffer)[charsRead - 1] = '\0';
    newTask->dueDate.year = atoi(buffer);

    //DUE DATE: ask user for MONTH
    printf("|--------------------------------------------------\n|\n|   Task Manager: Create Task\n|\n|   Please enter the MONTH that this\n|   task is due.\n|\n|   : ");

    //DUE DATE: get month from user
    charsRead = getline(&buffer, &bufferSize, stdin);
    (buffer)[charsRead - 1] = '\0';
    newTask->dueDate.month = atoi(buffer);

    //DUE DATE: ask user for DAY
    printf("|--------------------------------------------------\n|\n|   Task Manager: Create Task\n|\n|   Please enter the DATE that this\n|   task is due.\n|\n|   : ");

    //DUE DATE: get day from user
    charsRead = getline(&buffer, &bufferSize, stdin);
    (buffer)[charsRead - 1] = '\0';
    newTask->dueDate.day = atoi(buffer);

    //free temp date buffer
    free(buffer);

    //insert newTask into tasks
    if(tasks->head == NULL)
    {
        //if list is empty, new task is head
        tasks->head = newTask;
    }
    else
    {
        struct task* currTask = tasks->head;

        //traverse to end of list
        while(currTask->next != NULL)
        {
            currTask = currTask->next;
        }

        //insert newTask at end of list
        currTask->next = newTask;
    }

    //increment total and incomplete task count
    tasks->numTasks++;
    tasks->incompleteTasks++;

    //print success message
    printf("|--------------------------------------------------\n|   Task called '%s' created!\n", newTask->name);
    return;
}

/**********************************************************************************
    ** Description: Frees all memory associated with a taskList
    ** Parameters: taskList struct to free
**********************************************************************************/
void freeTaskList(struct taskList* tasks)
{
    //curr and next pointers
    struct task* currTask = tasks->head;
    struct task* nextTask;

    //traverse list, 
    while(currTask != NULL)
    {
        nextTask = currTask->next;
        free(currTask->name);
        free(currTask->category);
        free(currTask);
        currTask = nextTask;
    }
}

/**********************************************************************************
    ** Description: Allows user to mark a task as complete. Note, the task to mark
    as complete isn't yet selected/passed in on this call. This function will ask
    the user which task they would like to complete within this function
    ** Parameters: taskList from which to mark a task as complete
**********************************************************************************/
void completeTask(struct taskList* tasks)
{
    //no tasks to mark as complete, return to main menu
    if(tasks->incompleteTasks == 0)
    {
        system("clear");
        printf("|--------------------------------------------------\n|   There are no incomplete tasks to mark\n|   as complete. Please create a task first!\n");
        return;
    }

    //display list of incomplete tasks
    system("clear");
    printf("|--------------------------------------------------\n|\n|   Task Manager: Complete Task\n|\n");
    //print task names in accordance with an incrementing number
    struct task* currTask = tasks->head;
    int i = 0;
    while(currTask != NULL)
    {
        //print incomplete tasks
        if(currTask->complete == 0)
        {
            i++;
            printf("|   %d. %s\n", i, currTask->name);
        }
        currTask = currTask->next;
    }

    //ask user which task they want to complete
    printf("|\n|   Please type the number that corresponds\n|   to the task you would like to mark as\n|   complete, and hit enter.\n|\n|   To cancel, type 'cancel' and hit enter.\n|\n|   : ");
    size_t bufferSize = 32;
    char* buffer = (char *)malloc(bufferSize * sizeof(char));
    size_t charsRead = getline(&buffer, &bufferSize, stdin);
    if(charsRead == -1)
    {
        perror("Error reading input");
        exit(1);
    }
    buffer[charsRead - 1] = '\0'; //remove newline character

    //check if input is 'cancel'
    if(strcmp(buffer, "cancel") == 0)
    {
        free(buffer);
        return; //cancel this operation
    }

    //convert input to int
    int selectedTask = atoi(buffer);

    //check if input is valid
    if(selectedTask < 1 || selectedTask > tasks->incompleteTasks)
    {
        printf("|\n|   Invalid input. Please enter a valid option.\n|\n");
        free(buffer);
        return;
    }

    //find the selectedTask-th incomplete task
    currTask = tasks->head;
    i = 0;
    while(currTask != NULL)
    {
        //if an incomplete task
        if(currTask->complete == 0)
        {
            i++;
            if(i == selectedTask)
            {
                //if selectedTask-th incomplete task is found
                break;
            }
        }
        currTask = currTask->next;
    }

    //mark selected task as complete
    currTask->complete = 1;
    tasks->incompleteTasks--;
    system("clear");

    printf("|--------------------------------------------------\n|   '%s' marked as complete.\n", currTask->name);

    free(buffer);
}

/**********************************************************************************
    ** Description: Exports tasks in the process to a file
    ** Parameters: taskList whose tasks to export
**********************************************************************************/
void exportTasks(struct taskList* tasks)
{
    //no tasks to export, return to main menu
    if(tasks->numTasks == 0)
    {
        system("clear");
        printf("|--------------------------------------------------\n|   There are no tasks to export.\n|   Please create a task first!\n");
        return;
    }
    int fileOption = 0;

    //create buffer
    size_t bufferSize = 32;
    size_t charsRead;
    char* buffer = (char *)malloc(bufferSize * sizeof(char));
    memset(buffer, '\0', bufferSize);
    if(buffer == NULL)
    {
        perror("Unable to allocate buffer");
        exit(1);
    }

    system("clear");
    int help = 1;
    while(help == 1)
    {
        //ask user if they would like to overwrite or append
        printf("|--------------------------------------------------\n|\n|   Task Manager: Export Tasks\n|\n|   1. Overwrite file contents\n|\n|      OR\n|\n|   2. Append to existing file contents\n|\n|   Please select 1 or 2 to specify how\n|   the file you're exporting to will \n|   be affected.\n|\n|   For more information, type 'help'\n|   and hit enter.\n|\n|   To cancel, type 'cancel' and hit enter.\n|\n|  : ");
        charsRead = getline(&buffer, &bufferSize, stdin);
        if(charsRead == -1)
        {
            perror("Error reading input");
            exit(1);
        }
        buffer[charsRead - 1] = '\0'; //remove newline character

        //check if input is 'cancel'
        if(strcmp(buffer, "cancel") == 0)
        {
            free(buffer);
            return; //cancel this operation
        }

        //check if input is 'help'
        if(strcmp(buffer, "help") == 0)
        {
            system("clear");
            printf("|--------------------------------------------------\n|\n|   Task Manager: Help\n|\n|   For more information on how exporing\n|   to files works, visit the link\n|   below for this project's README.\n|\n|   https://github.com/MasonRosenau/task-manager?tab=readme-ov-file#exporting-tasks\n|\n");
            continue;
        }
        help = 0;
    }    

    //obtain whether user wants to overwrite or append
    fileOption = atoi(buffer);

    //ask user for a file they would like to export to
    printf("|--------------------------------------------------\n|\n|   Task Manager: Export Tasks\n|\n|   Please enter the name of the file you\n|   would like to export your tasks to, and\n|   hit enter.\n|\n|   To cancel, type 'cancel' and hit enter.\n|\n|  : ");
    charsRead = getline(&buffer, &bufferSize, stdin);
    if(charsRead == -1)
    {
        perror("Error reading input");
        exit(1);
    }
    buffer[charsRead - 1] = '\0'; //remove newline character

    //check if input is 'cancel'
    if(strcmp(buffer, "cancel") == 0)
    {
        free(buffer);
        return; //cancel export operation
    }

    //create file pointer for opening
    FILE* exportFile;
    if(fileOption == 1)
    {
        //open file for overwriting
        exportFile = fopen(buffer, "w");
    }
    if(fileOption == 2)
    {
        //open file for appending
        exportFile = fopen(buffer, "a");
    }

    //couldn't open file
    if(!exportFile)
    {
        perror("Error opening file");
        free(buffer);
        exit(1);
    }

    //write tasks to file
    struct task* currTask = tasks->head;
    while(currTask != NULL)
    {
        fprintf(exportFile, "%d|%s|%d_%d_%d|%s\n", currTask->complete, currTask->name, currTask->dueDate.year, currTask->dueDate.month, currTask->dueDate.day, currTask->category);
        currTask = currTask->next;
    }
    printf("|\n|   Tasks exported to %s!\n", buffer);
    fclose(exportFile);
    free(buffer);
}

int main(int argc, char *argv[])
{   
    //fork child process to start up microservice
    pid_t childPID;
    pid_t spawnPID = fork();

    switch (spawnPID)
    {
        //error forking
        case -1:
            perror("Hull Breach!");
            exit(1);
            break;
        //child process
        case 0:
            //execute microservice python3 taskgen.py
            execlp("python3", "python3", "taskgen.py", NULL);
            break;
        //parent process
        default:
            //fork returns the child's PID in parent process
            childPID = spawnPID;
            break; //continue to main program to let user interact
    }

    //create buffer
    size_t bufferSize = 32;
    char* buffer = (char *)malloc(bufferSize * sizeof(char));
    memset(buffer, '\0', bufferSize);
    if(buffer == NULL)
    {
        perror("Unable to allocate buffer");
        exit(1);
    }
    
    //create taskList
    struct taskList tasks;
    tasks.head = NULL;
    tasks.numTasks = 0;
    tasks.incompleteTasks = 0;

    //prompt user to import tasks or start fresh
    system("clear");
    FILE* importFile = promptImport(&buffer, bufferSize, 0);
    
    //if file couldn't be opened, AND user didn't just hit enter to start fresh
    while(importFile == NULL && buffer[0] != '\0')
    {
        //keep reprompting user
        importFile = promptImport(&buffer, bufferSize, 1);
    }

    //if importFile is null here, user is starting fresh
    //otherwise, user is importing tasks
    if(!importFile)
    {
        createTaskFromUser(&tasks);
    }
    else
    {
        importTasks(&tasks, importFile);
    }

    //main menu loop
    while(1)
    {
        //display main menu options
        printf("|--------------------------------------------------\n|\n|   Task Manager: Home\n|\n|   1. View all tasks\n|   2. Mark a task as complete\n|   3. Create a new task\n|   4. Export your tasks to a file\n|\n|   Please type 1, 2, 3, or 4, and hit\n|   enter to do the corresponding action.\n|\n|   To exit, type 'exit' and hit enter.\n|\n|   : ");
        
        //get user input
        size_t charsRead = getline(&buffer, &bufferSize, stdin);
        if(charsRead == -1)
        {
            perror("Error reading input");
            exit(1);
        }
        buffer[charsRead - 1] = '\0'; //remove newline character

        //check user input and perform corresponding action
        if(strcmp(buffer, "1") == 0)
        {
            viewTasks(&tasks);
        }
        else if(strcmp(buffer, "2") == 0)
        {
            completeTask(&tasks);
        }
        else if(strcmp(buffer, "3") == 0)
        {
            createTaskFromUser(&tasks);
        }
        else if(strcmp(buffer, "4") == 0)
        {
            exportTasks(&tasks);
        }
        else if(strcmp(buffer, "exit") == 0)
        {
            break;
        }
        else
        {
            printf("Invalid input. Please enter a valid option.\n");
        }
    }

    //free dynamic memory
    freeTaskList(&tasks);
    free(buffer);

    //send termination to python microservice
    kill(childPID, SIGTERM);

    //wait for child process to terminate
    int status;
    waitpid(childPID, &status, 0);

    system("clear");
    printf("|--------------------------------------------------\n|   Thank you for using Task Manager!\n|--------------------------------------------------\n");
    return 0;
}