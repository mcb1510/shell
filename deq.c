/* 
 * File: deq.c
 * Description: Implementation of the double-ended double-linked list.
 * 
 * 
 * Author(s): Jim Buffenbarger
 * Date: 9/8/25
 */

 #include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "deq.h"
#include "error.h"

// indices and size of array of node pointers
typedef enum {Head,Tail,Ends} End; //0,1,2

//Node = pointer to a node with next/prev and data
typedef struct Node { 
  //np is an array of size Ends that is 2 with pointers to next/prev neighbors 
  struct Node *np[Ends]; 
  Data data; // value stored in the node
} *Node; // pointer to a node


//Rep = pointer to the deque with head/tail and length
typedef struct { 
  Node ht[Ends];                // head/tail nodes // array of nodes/ the deque
  int len;                      // length of the deque
} *Rep; // pointer to the whole deque and shortcut type r

//helper function to access the deque’s internal structure
static Rep rep(Deq q) {
  if (!q) ERROR("zero pointer");
  return (Rep)q;
}


/*
 * Function: put
 * --------------
 * Inserts data into the deque at the specified end increasen
 * deque length.
 *
 * Parameters:
 *  r = Pointer to deque container to access head, tail, and length
 *  e = Which end to insert the new node at: Head or Tail, 0 or 1
 *  d = data to be stored in node
 * 
 * Returns:
 *    - Nothing
 */
static void put(Rep r, End e, Data d) 
{
  // First we allocate a new node
  Node new_node = malloc(sizeof(*new_node));
  if (!new_node) ERROR("malloc() failed");
  new_node->data = d;
  // initialize neighbor pointers, they dont exist yet
  new_node->np[Head] = new_node->np[Tail] = NULL; 
  // Link it to existing deque
  Node current_node = r->ht[e]; //current node at e
  // new node points toward current node at the opposite end
  // If inserting at head then points to current head. 
  // If inserting at tail then points to current tail.
  new_node->np[1-e] = current_node; 
  // current node points to new node
  if (current_node) {
    current_node->np[e] = new_node;
  }
  //update deque and increase length
  if (r->len == 0) { //in case deque is empty 
    //new node is head and tail of deque
    r->ht[Head] = r->ht[Tail] = new_node;
  }
  else {
    //chosen end updated to be the new node 
    r->ht[e] = new_node;
  }
  r->len++;
}

/*
 * Function: get
 * --------------
 * Returns data from an end and decreases length of deque.
 * If the deque is empty, the function exits with an error.
 *
 * Parameters:
 *  r = Pointer to deque container to access head, tail, and length
 *  e = Which end to insert the new node at: Head or Tail, 0 or 1
 * 
 * Returns:
 *    - data from specified node
 */
static Data get(Rep r, End e)         
{
   // if deque is empty, it triggers and error
  if (r->len == 0) ERROR("Deque is empty! Exiting program...");
  Node current_node = r->ht[e]; // current node at e that will be removed
  Data data = current_node->data; // data to return
  r->ht[e] = current_node->np[1-e]; // update head/tail of deque to next node
  // if there is a new head/tail, set its neighbor pointer to NULL
  if (r->ht[e]) {
    r->ht[e]->np[e] = NULL;
  } 
  // if deque is empty both head and tail pointers are NULL
  else {
    r->ht[1-e] = NULL; 
  } 
  free(current_node); // free the old/current head/tail node
  r->len--; 
  return data; 
}

/*
 * Function: ith
 * --------------
 * Returns data from ith position from deque
 * If the specified index is out of bounds, exits with error
 *
 * Parameters:
 *  r = Pointer to deque container to access head, tail, and length
 *  e = Which end to insert the new node at: Head or Tail, 0 or 1
 *  i = index from where data is going to be retreived (0-based)
 * 
 * Returns:
 *    - data from ith node
 */
static Data ith(Rep r, End e, int i)  {
  // We check if we are out of bounds
  if (i < 0 || i >= r->len) ERROR("Index out of bounds. Exiting program...");
  // we start at the node at the chosen end (head/tail)
  Node current_node = r->ht[e];
  // we move towar
  while (i > 0) {
    current_node = current_node->np[1-e]; //move toward the opposite end
    i--;
  }
  return current_node->data; // return data from node
}

/*
 * Function: rem
 * --------------
 * Removes node that contains the data specified if found and decreases length.
 * Error if data is not found.
 *
 * Parameters:
 *  r = Pointer to deque container to access head, tail, and length
 *  e = Which end to insert the new node at: Head or Tail, 0 or 1
 *  d = data value to remove
 * 
 * Returns:
 *    - data removed
 */
static Data rem(Rep r, End e, Data d) {
    Node node = r->ht[e]; // we start at the specified end

    while (node) {// go through all deque
        if (node->data == d) { // if we get a match of value
            //Save data that will be removed
            Data removed_data = node->data;

          // We symeetricly update neighbors to skip over the node
          //that will be removed
          for (int i = 0; i < Ends; i++) {
              if (node->np[i]) {
                  node->np[i]->np[1 - i] = node->np[1 - i];
              }
            }

          // We Symmetricly update of deque ends if the node that will
          // be eliminating is head/tail
          for (int i = 0; i < Ends; i++) {
              if (r->ht[i] == node) {
                  r->ht[i] = node->np[1-i]; // Move head/tail pointer to the next valid node
              }
          }
            free(node);
            r->len--;
            return removed_data;
        }
        node = node->np[1 - e]; // move to the next node toward opposite end
    }

    ERROR("Value not found. Exiting program...");
    return 0; 
}


extern Deq deq_new() {
  Rep r=(Rep)malloc(sizeof(*r));
  if (!r) ERROR("malloc() failed");
  r->ht[Head]=0;
  r->ht[Tail]=0;
  r->len=0;
  return r;
}

extern int deq_len(Deq q) { return rep(q)->len; }

extern void deq_head_put(Deq q, Data d) {        put(rep(q),Head,d); }
extern Data deq_head_get(Deq q)         { return get(rep(q),Head);   }
extern Data deq_head_ith(Deq q, int i)  { return ith(rep(q),Head,i); }
extern Data deq_head_rem(Deq q, Data d) { return rem(rep(q),Head,d); }

extern void deq_tail_put(Deq q, Data d) {        put(rep(q),Tail,d); }
extern Data deq_tail_get(Deq q)         { return get(rep(q),Tail);   }
extern Data deq_tail_ith(Deq q, int i)  { return ith(rep(q),Tail,i); }
extern Data deq_tail_rem(Deq q, Data d) { return rem(rep(q),Tail,d); }

extern void deq_map(Deq q, DeqMapF f) {
  for (Node n=rep(q)->ht[Head]; n; n=n->np[Tail])
    f(n->data);
}

extern void deq_del(Deq q, DeqMapF f) {
  if (f) deq_map(q,f);
  Node curr=rep(q)->ht[Head];
  while (curr) {
    Node next=curr->np[Tail];
    free(curr);
    curr=next;
  }
  free(q);
}

extern Str deq_str(Deq q, DeqStrF f) {
  char *s=strdup("");
  for (Node n=rep(q)->ht[Head]; n; n=n->np[Tail]) {
    char *d=f ? f(n->data) : n->data;
    char *t; asprintf(&t,"%s%s%s",s,(*s ? " " : ""),d);
    free(s); s=t;
    if (f) free(d);
  }
  return s;
}
