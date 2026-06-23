#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

typedef struct list
{
    char letter;
    int repetition;
    struct list *next;
} list;

int count = 0;

void free_list(list *first);
void initialize_list(list *l);
void print_all(list *l);

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Usage: ./decompressor compressed decompressed\n");
        return 1;
    }

    // read the metadata
    FILE *f = fopen(argv[1], "r");
    if (f == NULL)
    {
        printf("Error while opening compressed file\n");
        return 1;
    }

    // Put it in a char of size of one byte
    const int BYTELEN = 8;
    char *buffer = malloc(sizeof(char) * BYTELEN);

    int doend = 0;

    // Create an array
    list *metadata = malloc(sizeof(list));
    initialize_list(metadata);

    char letter;
    int rep = 0;

    while (fscanf(f, " %c:%d|", &letter, &rep) == 2) 
    {
        list *temp = malloc(sizeof(list));
        initialize_list(temp);
        temp->letter = letter;
        temp->repetition = rep;
        temp->next = metadata->next;
        metadata->next = temp;
    }

    print_all(metadata); 

    fclose(f);
    free(buffer);
    free_list(metadata);

    return 0;
}

void print_all(list *l)
{
    if (l == NULL)
    {
        return;
    }

    if (l->letter != '\0')
    {
        // Print metadata
        printf("%c:%i||", l->letter, l->repetition);
    }

    if (l->next != NULL)
    {
        print_all(l->next);
    }
}

void free_list(list *first)
{
    if (first->next != NULL)
    {
        free_list(first->next);
    }

    free(first);
}

void initialize_list(list *l)
{
    if (l != NULL)
    {
        l->letter = '\0';
        l->repetition = 0;
        l->next = NULL;
    }
}