#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{   
    char* buffer;
    size_t bufferSize = 32;
    size_t charsRead;

    //create buffer
    buffer = (char *)malloc(bufferSize * sizeof(char));
    if( buffer == NULL)
    {
        perror("Unable to allocate buffer");
        exit(1);
    }

    //prompt user
    printf("Please type a file name to load tasks from files. Otherwise, hit Enter to start fresh: ");
    charsRead = getline(&buffer,&bufferSize,stdin);

    
    if(buffer[0] == '\n' && charsRead == 1)
    {
        printf("You hit enter\n");
    }
    else
    {
        //remove last char (newline)
        buffer[charsRead - 1] = '\0';
        printf("File name: '%s'\n", buffer);
    }
    free(buffer);

    return 0;
}
