#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**********************************************************************************
    ** Description: Prompt's the user on whether they would like to import tasks
    from a file or start fresh with no tasks.
    ** Parameters: Takes in a buffer, the bufferSize, and a variable to keep track
    of the number of characters read.
**********************************************************************************/
char* promptImport(char* buffer, size_t bufferSize, size_t charsRead)
{
    printf("Enter a filename/filepath to load import tasks!\nOtherwise, just hit 'Enter' to start fresh.\n");

    //take in user input; getline can dynamically resize buffer to >32
    charsRead = getline(&buffer,&bufferSize,stdin);

    /*
    getline will always return a newline at the end of the input.
    if the user just hits enter, this line creates
    if the user enters a file name, this removes the newline
    */
    buffer[charsRead - 1] = '\0';
    
    return buffer;
}

int main(int argc, char *argv[])
{   
    char* buffer;
    size_t bufferSize = 32;
    size_t charsRead;

    //create buffer
    buffer = (char *)malloc(bufferSize * sizeof(char));
    if(buffer == NULL)
    {
        perror("Unable to allocate buffer");
        exit(1);
    }
    
    //prompt user to import tasks or start fresh
    promptImport(buffer, bufferSize, charsRead);

    //if the first character in buffer exists (isn't just a \0)
    if(*buffer)
    {
        printf("Opening file '%s'...\n", buffer);
    }
    else
    {
        printf("Starting fresh!\n");
    }
    
    free(buffer);
    return 0;
}
