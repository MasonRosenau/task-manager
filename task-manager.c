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
        printf("|--------------------------------------------------\n|\n|   It looks like the file name you\n|   entered couldn't be opened.\n|\n|   Please try again below, either\n|   by hitting enter or entering\n|   a new file name.\n|\n|   : ");
    }
    else
    {
        printf("|--------------------------------------------------\n|\n|   Task Manager: Welcome!\n|\n|   To begin, you have 2 options:\n|\n|   1. To import tasks, type the name\n|      of a file from which to\n|      import tasks, and hit enter.\n|\n|      OR\n|\n|   2. To start with no tasks (from\n|      scratch), simply hit enter.\n|\n|   For more information, type 'help'\n|   and hit enter.\n|\n|   : ");
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
    else if(strcmp(*buffer, "help") == 0)
    {
        system("clear");
        printf("|--------------------------------------------------\n|\n|   Task Manager: Help\n|\n|   Importing from large files with\n|   lots of tasks may take awhile.\n|\n|   First time users with no existing\n|   files may want to start from scratch!\n|\n|   For more information importing\n|   tasks from files, visit the link\n|   below for this project's README.\n|\n|   https://github.com/MasonRosenau/task-manager?tab=readme-ov-file#importing-tasks\n|\n");
        return promptImport(buffer, bufferSize, 0);
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
    ** Parameters: A premade taskList struct the file to import
**********************************************************************************/
void importTasks(struct taskList* tasks, FILE* importFile)
{
    //import tasks
    char *currLine = NULL;
    size_t len = 0;
    size_t charsRead = 0;

    //create tail pointer to make insertions easier
    struct task *tail = NULL;

    //If getline() fails to read any characters from the input stream, it returns -1 (end of file)
    while ((charsRead = getline(&currLine, &len, importFile)) != -1){

        tasks->numTasks++;

        //create a new task corresponding to the current line in file
        struct task *newTask = createTaskFromFile(currLine);
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
    system("clear");
    printf("|--------------------------------------------------\n|   Imported %d tasks!\n", tasks->numTasks);
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
void viewTasks(struct taskList* tasks)
{
    if(tasks->numTasks == 0)
    {
        system("clear");
        printf("|--------------------------------------------------\n|   There are no tasks to view.\n|   Please create a task first!\n");
        return;
    }
    system("clear");
    printf("|--------------------------------------------------\n|\n|   Task Manager: View Tasks\n|\n");
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
    ** Description: Creates a task from user input and inserts it into tasks
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

    //obtain task name
    system("clear");
    printf("|--------------------------------------------------\n|\n|   Task Manager: Create Task\n|\n|   Please enter the NAME of the task\n|   you would like to create, and hit\n|   enter.\n|\n|   To cancel, type 'cancel' and hit enter.\n|\n|   : ");
    newTask->name = (char *)malloc(bufferSize * sizeof(char));
    memset(newTask->name, '\0', bufferSize);
    charsRead = getline(&(newTask->name), &bufferSize, stdin);
    (newTask->name)[charsRead - 1] = '\0';

    //check if input is 'cancel'
    if(strcmp(newTask->name, "cancel") == 0)
    {
        free(newTask->name);
        free(newTask);
        return; //cancel this operation
    }

    //obtain task category
    printf("|--------------------------------------------------\n|\n|   Task Manager: Create Task\n|\n|   Please enter the CATEGORY of the\n|   task you would like to create, and\n|   hit enter.\n|\n|   To omit a category for this task,\n|   simply hit enter.\n|\n|   : ");
    newTask->category = (char *)malloc(bufferSize * sizeof(char));
    memset(newTask->category, '\0', bufferSize);
    charsRead = getline(&(newTask->category), &bufferSize, stdin);
    (newTask->category)[charsRead - 1] = '\0';
    if((newTask->category)[0] == '\0')
    {
        //set category to the string "None"
        strcpy(newTask->category, "None");       
    }

    //obtain task due date (YYYY_MM_DD)
    printf("|--------------------------------------------------\n|\n|   Task Manager: Create Task\n|\n|   Please enter the YEAR that this\n|   task is due.\n|\n|   : ");
    char* buffer = (char *)malloc(bufferSize * sizeof(char));
    memset(buffer, '\0', bufferSize);
    //year
    charsRead = getline(&buffer, &bufferSize, stdin);
    (buffer)[charsRead - 1] = '\0';
    newTask->dueDate.year = atoi(buffer);
    //month
    printf("|--------------------------------------------------\n|\n|   Task Manager: Create Task\n|\n|   Please enter the MONTH that this\n|   task is due.\n|\n|   : ");
    charsRead = getline(&buffer, &bufferSize, stdin);
    (buffer)[charsRead - 1] = '\0';
    newTask->dueDate.month = atoi(buffer);
    //day
    printf("|--------------------------------------------------\n|\n|   Task Manager: Create Task\n|\n|   Please enter the DATE that this\n|   task is due.\n|\n|   : ");
    charsRead = getline(&buffer, &bufferSize, stdin);
    (buffer)[charsRead - 1] = '\0';
    newTask->dueDate.day = atoi(buffer);
    free(buffer);

    //insert newTask into tasks
    if(tasks->head == NULL)
    {
        tasks->head = newTask;
    }
    else
    {
        struct task* currTask = tasks->head;
        while(currTask->next != NULL)
        {
            currTask = currTask->next;
        }
        currTask->next = newTask;
    }
    tasks->numTasks++;
    tasks->incompleteTasks++;
    return;
}

/**********************************************************************************
    ** Description: Frees all memory associated with a taskList
    ** Parameters: taskList struct to free
**********************************************************************************/
void freeTaskList(struct taskList* tasks)
{
    struct task* currTask = tasks->head;
    while(currTask != NULL)
    {
        struct task* nextTask = currTask->next;
        free(currTask->name);
        free(currTask->category);
        free(currTask);
        currTask = nextTask;
    }
}

/**********************************************************************************
    ** Description: Allows user to mark a task as complete
    ** Parameters: taskList from which to mark a task as complete
**********************************************************************************/
void completeTask(struct taskList* tasks)
{
    if(tasks->incompleteTasks == 0)
    {
        system("clear");
        printf("|--------------------------------------------------\n|   There are no incomplete tasks to mark\n|   as complete. Please create a task first!\n");
        return;
    }

    //display list of incomplete tasks
    system("clear");
    printf("|--------------------------------------------------\n|\n|   Task Manager: Complete Task\n|\n");
    //print task names in accordance with their number
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
    printf("|\n|   '%s' marked as complete.\n", currTask->name);

    free(buffer);
}

/**********************************************************************************
    ** Description: Exports tasks to a file
    ** Parameters: taskList whose tasks to export
**********************************************************************************/
void exportTasks(struct taskList* tasks)
{
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
        return; //cancel this operation
    }

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

/**********************************************************************************
    ** Description: Main Function 
**********************************************************************************/
int main(int argc, char *argv[])
{   

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
    
    //if file couldn't be opened, and user didn't just hit enter to start fresh
    while(importFile == NULL && buffer[0] != '\0')
    {
        //keep reprompting user
        importFile = promptImport(&buffer, bufferSize, 1);
    }

    //if importFile is null here, user is starting fresh
    if(!importFile)
    {
        //create a task (tasks should be null)
        createTaskFromUser(&tasks);

    }
    else //user is importing tasks
    {
        //import tasks from importFile
        importTasks(&tasks, importFile);
    }

    //main menu loop
    while(1)
    {
        //display main menu options
        printf("|--------------------------------------------------\n|\n|   Task Manager: Home\n|\n|   1. View all tasks\n|   2. Mark a task as complete\n|   3. Create a new task\n|   4. Export your tasks to a file\n|\n|   Please type 1, 2, 3, or 4, and hit\n|   enter to do the corresponding action.\n|\n|   To exit, type 'exit' and hit enter.\n|\n|   : ");
        
        // Get user input
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
    system("clear");
    printf("|--------------------------------------------------\n|   Thank you for using Task Manager!\n|--------------------------------------------------\n");
    return 0;
}