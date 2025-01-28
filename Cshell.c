#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFER_READ_SIZE 1024
#define BUFFER_ARGS_SIZE 64

char* shellRead();
char** shellParse(char* prompt);
int shellExecute(char **args);

int main()
{
    char *prompt;
    char **args;

    while(1)
    {
        printf("> ");

        //reading
        prompt = shellRead();

        //parsing
        args = shellParse(prompt);
        
        //execute
        shellExecute(args);
    }

    return EXIT_SUCCESS;
}

char* shellRead()
{
    int position = 0, bufferSize = BUFFER_READ_SIZE;
    char c;
    char *buffer = NULL;

    buffer = malloc(sizeof(char) * bufferSize);

    if(buffer == NULL)
    {
        fprintf(stderr, "malloc failed in shellRead\n");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        c = getchar();

        if(c == '\n' || c == EOF)
        {
            buffer[position] = '\0';
            return buffer;
        }else{
            buffer[position] = c;   
        }

        position++;

        if(position >= bufferSize)
        {
            bufferSize += BUFFER_READ_SIZE;
            buffer = realloc(buffer, bufferSize);
            if(buffer == NULL)
            {
                fprintf(stderr, "realloc failed in shellRead\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}
/*
* strip leading and trailing spaces from a string
*/
void promptPurify(char* prompt)
{
    short spaceCount = 0;
    char *index = prompt;

    // counting leading white spaces
    while(*index == ' ')
    {
        spaceCount++;
        index++;
    }

    index = prompt;
    prompt = prompt + spaceCount;

    //shifting the string to fill the spaces
    while(*prompt != '\0')
    {
        *index = *prompt;
        index++;
        prompt++;
    }

    *index = '\0';
    index--;

    //filling ending spaces with zero
    while(*index == ' ')
    {
        *index = '\0';
        index--;
    }
}

char** shellParse(char* prompt)
{
    char *begin, *end;
    char **args;
    int position = 0;
    char quoteFlag = 0; // used to handle the "" or '' on the prompt

    args = malloc(sizeof(char*) * BUFFER_ARGS_SIZE);
    if(args == NULL)
    {
        fprintf(stderr, "malloc failed in shellParse\n");
        exit(EXIT_FAILURE);
    }
    
    promptPurify(prompt); // ignoring leading and trailing spaces

    begin = prompt;
    end = begin;

    do
    {
        /*
        * when a quotation mark is uncountered for the first time the quoteFlag is set
        * and unset when uncountered for the second time
        */
        if(*end == '"' || *end == '\'')
            quoteFlag ^= 1; // set or unset the flag when the " or ' character is uncountered

        // if the quoteFlag is set we don't need to split the spaces between the quotation mark
        if(quoteFlag == 0 && (*end == ' ' || *end == '\t'))
        {

            *end = '\0';

            //add string to args
            args[position] = begin;
            position++;

            // move begin to the next string
            begin = end + 1;
        }

        end++;
    }while(*end != '\0');

    args[position] = begin; // adding the last remaining string in the prompt
    position++;

    args[position] = NULL; // args must end with NULL

    return args;
}

int shellExecute(char **args)
{
    pid_t pid, wpid;
    int status;
    pid = fork();
    if (pid == 0)
    {
        // Child process
        if (execvp(args[0], args) == -1)
        {
            perror("shellExecute");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("shellExecute");
    } else {
        // Parent process
        do
        {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}