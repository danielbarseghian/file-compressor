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

typedef struct node
{
    char letter;
    int repetition;
    struct node *right;
    struct node *left;
} node;

typedef struct pair
{
    node *first;
    node *seconde;
} pair;

int count = 0;

void free_list(list *first);
void initialize_list(list *l);
void print_all(list *l);
node *build_huffman(list *arr);
pair find_two_smallest(list **arr);

int node_count = 0;

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
        // initialize value
        list *temp = malloc(sizeof(list));
        initialize_list(temp);

        // Put in temp
        temp->letter = letter;
        temp->repetition = rep;

        // Put temp in front
        temp->next = metadata->next;
        metadata->next = temp;

        // Incremente
        node_count++;
    }

    print_all(metadata);
    printf("\n");

    // A list for looping on every metadata is easier
    list **meta_arr = malloc(sizeof(list *) * node_count);
    int b = 0;

    put_in_arr(metadata, meta_arr, b);
    for (int i = 0; i < node_count; i++)

    fclose(f);
    free(buffer);
    free_list(metadata);

    return 0;
}

void put_in_arr(list *metadata, list **arr, int buffer)
{
    // Put value
    arr[buffer] = metadata;

    // Incremente buffer by 1
    buffer++;

    // recall if arr->next is not null
    if (metadata->next != NULL)
    {
        put_in_arr(metadata->next, arr, buffer);
    }
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

pair find_two_smallest(list **arr)
{
    pair results;
    
    results.first = NULL;
    results.seconde = NULL;

    for (int i = 0; i < node_count; i++)
    {
        // Check for null
        if (arr[i] == NULL)
        {
            continue;
        }

        if (results.first == NULL || arr[i]->repetition < results.first->repetition)
        {
            // Swap them and put value
            results.seconde = results.first;
            results.first = arr[i];
        }

        else if (results.seconde == NULL || arr[i]->repetition < results.seconde->repetition)
        {
            results.seconde = arr[i];
        }
    }

    return results;
}

// Build a huffman tree and return the top element
node *build_huffman(list *arr)
{
    while (1)
    {
        // Find the two smallest ones globally
        pair values = find_two_smallest(arr);

        // If there is no second node, the tree is fully built
        if (values.seconde == NULL)
        {
            break; 
        }
        
        int sum = values.first->repetition + values.seconde->repetition;

        // Create a new parent node
        node *parent = malloc(sizeof(node));
        parent->repetition = sum;
        parent->letter = '\0';
        parent->right = values.first;
        parent->left = values.seconde;

        int f_value = -1;
        int s_value = -1;

        // Find the matching indices in the array to update them
        for (int j = 0; j < node_count; j++)
        {
            if (arr[j] == values.first && f_value == -1)
            {
                f_value = j;
            }
            if (arr[j] == values.seconde && s_value == -1)
            {
                s_value = j;
            }
        }
        
        // Remove the second node and replace the first one with the parent
        arr[s_value] = NULL;
        arr[f_value] = parent;
    }

    // Find and return the single remaining root node
    for (int i = 0; i < node_count; i++)
    {
        if (arr[i] != NULL)
        {
            return arr[i];
        }
    }
    
    return NULL;
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