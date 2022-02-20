#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME_SIZE 16

typedef struct node
{
    int key;
    char name[NAME_SIZE];
    struct node *next;

} Node;

int insert_node(Node **head, char *newName, int newKey)
{
    Node *newNode = (Node*)malloc(sizeof(Node));
    Node *last = *head;

    strcpy(newNode->name, newName);
    newNode->key = newKey;
    newNode->next = NULL;

    if (*head == NULL)
    {
        *head = newNode;
        return 0;
    }

    while (last->next != NULL)
        last = last->next;
    
    last->next = newNode;
    return 0;
}

//delete node with specified key
int delete_node(Node **head, int keyT)
{
    Node *temp = *head, *prev;

    if (temp->next != NULL && temp->key == keyT)
    {
        *head = temp->next;
        free(temp);
        return 0;
    }

    while (temp != NULL && temp->key != keyT)
    {
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL)
        return 1;

    prev->next = temp->next;
    free(temp);
}

int sum_list(Node *head)
{
    int sum = 0;
    while (head != NULL)
    {
        sum += head->key;
        head = head->next;
    }
    return sum;
}

int print_list(Node *head)
{
    Node *t = head;
    for (int i = 0; t != NULL; i++)
    {
        printf("Name: %s\nKey: %d\n\n", t->name, t->key);
        t = t->next;
    }
}

int main(void)
{
    Node *head = 0;
    for (int i = 0; i < 10; i++) //insert 10 nodes into list
    {
        char name[NAME_SIZE];
        sprintf(name, "Node %d", i);
        insert_node(&head, name, i);
    }

    print_list(head);
    printf("sum = %d\n", sum_list(head));

    delete_node(&head, 9);

    print_list(head);
    printf("sum = %d\n", sum_list(head));

    return 0;
}