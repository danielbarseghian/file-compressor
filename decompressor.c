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
    unsigned int repetition;
    struct node *right;
    struct node *left;
} node;

typedef struct pair
{
    node *first;
    node *seconde;
} pair;

int count = 0;

void build_codes(node *root, char *buffer, int depth, char **codes);
void put_in_arr(list *l, int buffer, node **arr);
pair find_two_smallest(node **arr);
node *build_huffman(node **arr);
void initialize_list(list *l);
void free_list(list *first);
void print_all(list *l);


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

    list *temp_list = malloc(sizeof(list));
    initialize_list(temp_list);

    int doend = 0;

    char letter;
    int rep = 0;

    while (fscanf(f, "%c:%d|", &letter, &rep) == 2) 
    {
        // Malloc 
        list *temp = malloc(sizeof(temp));

        // Put the values
        temp->letter = letter;
        temp->repetition = rep;

        // Put temporary pnt to point to the front
        list *tmp_pnt = temp_list;

        // Put the head to the temp
        temp_list = temp;

        // Put the next to the old head
        temp_list->next = tmp_pnt;

        // Incremente
        node_count++;
    }

    // I cant put directly here since i can't know the count before.
    node **meta_arr = malloc(sizeof(node *) * node_count);
    if (meta_arr == NULL)
    {
        printf("error while malloc\n");
        return 1;
    }
    
    // Malloc every index
    for (int i = 0; i < node_count; i++)
    {
        meta_arr[i] = malloc(sizeof(node));
    }
    
    // Put it in the meta_arr
    unsigned int b = 0;
    put_in_arr(temp_list, b, meta_arr);

    for (int i = 0; i < node_count; i++)
    {
        printf("%c:%i||", meta_arr[i]->letter, meta_arr[i]->repetition);
    }
    printf("\n");

    // Build the three
    node *tree = build_huffman(meta_arr);
    if (tree == NULL)
    {
        printf("error while building\n");
        return 1;
    }

    char *bfr = malloc(sizeof(char) * node_count);
    if (bfr == NULL)
    {
        printf("error while malloc\n");
        return 1;
    }

    int depth = 0;
    char **codes = calloc(256, sizeof(char *));
    if (codes == NULL)
    {
        printf("error while calloc\n");
        return 1;
    }

    build_codes(tree, bfr, depth, codes);

    for (int i = 0; i < 256; i++)
    {
        if (codes[i] != NULL)
        {
            printf("%s | %c\n", codes[i], i);
        }
    }

    fclose(f);
    free(buffer);

    return 0;
}

void build_codes(node *root, char *buffer, int depth, char **codes)
{
    if (!root) return;

    // LEAF
    if (root->left == NULL && root->right == NULL)
    {
        buffer[depth] = '\0';  // CRITICAL FIX
        codes[(unsigned char)root->letter] = strdup(buffer);
        return;
    }

    // LEFT = 0
    if (root->left)
    {
        buffer[depth] = '0';
        build_codes(root->left, buffer, depth + 1, codes);
    }

    // RIGHT = 1
    if (root->right)
    {
        buffer[depth] = '1';
        build_codes(root->right, buffer, depth + 1, codes);
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

// Build a huffman tree and return the top element
node *build_huffman(node **arr)
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

pair find_two_smallest(node **arr)
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

void put_in_arr(list *l, int buffer, node **arr)
{
    // put in arr
    arr[buffer]->letter = l->letter;
    arr[buffer]->repetition = l->repetition;

    // initialize the other values
    arr[buffer]->right = NULL;
    arr[buffer]->left = NULL;

    // Incremente
    buffer++;
    
    // do next if not null
    if ((l->next != NULL) && (buffer < node_count))
    {
        put_in_arr(l->next, buffer, arr);
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