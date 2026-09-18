/* EECS 348 Assignment 1 - Number Guessing Game 

Brief description:
 the program reads a list of emails then uses commands from and uses a an dynmaic array based max heap to act as aqueue for a ceo inbox 
 in the program emails are ordered first by the sender category (boss subordinate peer importantperson otherperson) and then by newest date first within the same category
 
 Inputs:
 lines inputs are either an EMAIL line with category subject and date separated by commas 
 or
 NEXT READ or COUNT command
 
 Outputs:
 for NEXT the next email to read or a message that there are none 
 for COUNT the number of unread emails 
 for READ it produces no output just removes

 Collaborators: Claude code (Sonnet 5) 
 Other sources: None 
 Author: Carter Gray
 Creation date: 8/9/2026
 Revision date: N/A
 Revisions: N/A

*/


#include <stdio.h> // i need printf fgets sscanf
#include <stdlib.h> // i need malloc realloc free exit
#include <string.h> // i need strcmp strncpy strchr memmove strlen
#include <ctype.h>  // i need the isspace

#define INITIAL_CAPACITY 16  // starting size here
#define MAX_LINE 512 // max length of one input line

typedef struct {
    char sender[32]; // sender category boss subordinate etc
    char subject[256]; // subject line can spaces
    char date[16];     // date kept for display
    int  priority;   //  boss is highest
    long dateValue; // date 
} Email;

typedef struct {
    Email *data; // dynamic array that controls the heap
    int size; // total num of emails  stored
    int capacity;  //  size of data array
} MaxHeap;

/* ======================= max heap array based ======================= */

void heapInit(MaxHeap *h) {
    h->capacity = INITIAL_CAPACITY;  // start with the initial capacity
    h->size = 0;  // heap is already empty
    h->data = (Email *)malloc(sizeof(Email) * h->capacity); // allocate the backing array
    if (!h->data) {  // check that memory is allocated correctly  succeeded
        fprintf(stderr, "Memory allocation failed.\n"); // if the memory breaks reportit 
        exit(1); // stop 
    }
}

void heapFree(MaxHeap *h) {
    free(h->data);  // free the backing array
    h->data = NULL;  // make null to avoid dangling pointer
    h->size = 0;// reset size
    h->capacity = 0;  // reset capacity
}

// changes the size dynamicly to make sure everything is good
void heapResize(MaxHeap *h) {
    h->capacity *= 2;  // double the capacity by adding a layer
    Email *newData = (Email *)realloc(h->data, sizeof(Email) * h->capacity); // make the array bigger
    if (!newData) {  // check reallocation 
        fprintf(stderr, "Memory reallocation failed.\n");    // if the memory breaks reportit 
        exit(1); // stop 
    h->data = newData; // point at the new memory
}
}

void swapEmails(Email *a, Email *b) {
    Email temp = *a;// hold a copy of the first email
    *a = *b;        // move second into first
    *b = temp;    // move saved first into second
}

/* returns 1 if email a outranks should be read before email b */
int hasHigherPriority(Email *a, Email *b) {
    if (a->priority != b->priority) { // different categories
        return a->priority > b->priority; // higher category number wins
    }
    return a->dateValue > b->dateValue;  // otherwise newer date wins
}

void heapifyUp(MaxHeap *h, int index) {
    while (index > 0) { // stop once at the root
        int parent = (index - 1) / 2;  // index of the parent node
        if (hasHigherPriority(&h->data[index], &h->data[parent])) { // child outranks parent
            swapEmails(&h->data[index], &h->data[parent]);       // swap child up
            index = parent;   // continue from parent
        } else {
            break;   // heap property restored
        }
    }
}

void heapifyDown(MaxHeap *h, int index) {
    while (1) { // loop until no swap needed
        int left = 2 * index + 1; // index of left child
        int right = 2 * index + 2; // index of right child
        int largest = index;       // assume current is largest

        if (left < h->size && hasHigherPriority(&h->data[left], &h->data[largest])) { // check left child
            largest = left; // left child outranks current largest
        }
        if (right < h->size && hasHigherPriority(&h->data[right], &h->data[largest])) { // check right child
            largest = right;  // right child outranks current largest
        }
        if (largest == index) { // no child outranks current
            break;              // heap property restored
        }
        swapEmails(&h->data[index], &h->data[largest]);   // swap down into place
        index = largest; // continue from new index
    }
}

void heapInsert(MaxHeap *h, Email e) {
    if (h->size == h->capacity) { // array is full
        heapResize(h);  // grow the array
    }
    h->data[h->size] = e;  // place new email at the end
    heapifyUp(h, h->size); // restore heap property upward
    h->size++;        // one more email stored
}

/* removes and returns the highest priority email caller must ensure size is greater than zero */
Email heapExtractMax(MaxHeap *h) {
    Email top = h->data[0]; // save the root email
    h->size--; // one fewer email stored
    h->data[0] = h->data[h->size]; // move last email to the root
    heapifyDown(h, 0); // restore heap property downward
    return top;       // return the removed email
}

/* returns a pointer to the highest priority email without rem it or null if empty */
Email *heapPeek(MaxHeap *h) {
    if (h->size == 0) return NULL;  // nothing to look at
    return &h->data[0]; // look at root it is the biggest priority
}

int heapSize(MaxHeap *h) {
    return h->size;  // current num of emails
}

/* ============================ parsing ================================= */

int categoryPriority(const char *category) {
    if (strcmp(category, "Boss") == 0)            return 5;      // boss reads first
    if (strcmp(category, "Subordinate") == 0)     return 4;      // subordinate reads
    if (strcmp(category, "Peer") == 0)            return 3;      // peer reads 
    if (strcmp(category, "ImportantPerson") == 0) return 2;      // important person reads 
    if (strcmp(category, "OtherPerson") == 0)     return 1;      // other person reads last
    return 0;  // unrecognized category lowest priority
}

/* converts mm dd yyyy into an integer yyyymmdd for easy comparison */
long dateToValue(const char *date) {
    int month, day, year; // holders for parsed date parts
    if (sscanf(date, "%d-%d-%d", &month, &day, &year) != 3) {   // try to parse the date
        return 0;         // fallback value on bad input
    }
    return (long)year * 10000L + (long)month * 100L + (long)day; // combine into one comparable number
}

void trim(char *str) {
    int start = 0; // index of first non space char
    while (isspace((unsigned char)str[start])) start++;   // skip leadingwhitespace
    if (start > 0) memmove(str, str + start, strlen(str) - start + 1); // shift string left

    int len = (int)strlen(str);   // current length of string
    while (len > 0 && isspace((unsigned char)str[len - 1])) { // check trailing whitespace
        str[len - 1] = '\0';          // cutoff trailing char
        len--;              // shrink 
    }
}

/* parses category subject date into an email struct only the first two commas act as delimiters */
int parseEmailFields(char *line, Email *e) {
    char *fields[3];  // pointers to the three fields
    int count = 0;   // num of fields found so far
    char *token = line; // current start of remaining text
    char *comma;// next comma found

    while (count < 3) {   // gather up to three fields
        comma = strchr(token, ','); // look for the next comma
        if (comma != NULL && count < 2) {    // more fields still expected
            *comma = '\0';              // end the current field here
            fields[count++] = token;  // store the field
            token = comma + 1;       // move past the comma
        } else {
            fields[count++] = token; // store the final field
            break;                   // done gathering fields
        }
    }

    if (count < 3) {   // not enough fields found
        return 0;   // signal a malformed line
    }

    trim(fields[0]);   // clean up category field
    trim(fields[1]);// clean up subject field
    trim(fields[2]);// clean up date field

    strncpy(e->sender, fields[0], sizeof(e->sender) - 1);  // copy category into sender
    e->sender[sizeof(e->sender) - 1] = '\0'; // ensure null termination

    strncpy(e->subject, fields[1], sizeof(e->subject) - 1);               // copy subject text
    e->subject[sizeof(e->subject) - 1] = '\0';  // ensure null termination

    strncpy(e->date, fields[2], sizeof(e->date) - 1);// copy date text
    e->date[sizeof(e->date) - 1] = '\0';// ensure null termination

    e->priority = categoryPriority(e->sender); // compute category priority
    e->dateValue = dateToValue(e->date);      // compute comparable date value
    return 1;// signal good
}

/* ========================= command handlers ============================ */

void handleEmailCommand(MaxHeap *h, char *rest) {
    Email e;  // holds the parsed email
    if (parseEmailFields(rest, &e)) { // try to parse the fields
        heapInsert(h, e);           // add the email to the heap
    }
}

void handleNextCommand(MaxHeap *h) {
    Email *top = heapPeek(h); // look at the top email
    if (top == NULL) {        // heap is empty
        printf("No emails to read.\n");    // report nothing to read
        return;     // go back
    }
    printf("Next email:\n");       // header line
    printf("Sender: %s\n", top->sender);// print sender 
    printf("Subject: %s\n", top->subject);// print subject 
    printf("Date: %s\n", top->date);    // print date
}

void handleReadCommand(MaxHeap *h) {
    if (heapSize(h) == 0) { // nothing to remove
        return; // do noting 
    }
    heapExtractMax(h);   // discard the top email
}

void handleCountCommand(MaxHeap *h) {
    printf("There are %d emails to read.\n", heapSize(h));  // report emails left inW count
}

/* ================================ main ================================= */

int main(void) {
    MaxHeap heap; // priority que
    heapInit(&heap);   // create the empty heap

    char line[MAX_LINE];  // stop for one  line

    while (fgets(line, sizeof(line), stdin) != NULL) { // iterate and read each line of input
        trim(line); // strip leading trailing space

        if (strlen(line) == 0) { // skip any blank lines
            continue;// then move on
        }

        if (strncmp(line, "EMAIL", 5) == 0 && // if line starts with email
            (line[5] == '\0' || isspace((unsigned char)line[5]))) { // followed by space or end
            char *rest = line + 5;      // text after the keyword
            while (isspace((unsigned char)*rest)) rest++;// skip the separating space
            handleEmailCommand(&heap, rest);   // process the email line
        } else if (strcmp(line, "NEXT") == 0) {  // if next command
            handleNextCommand(&heap);          // show the next email
        } else if (strcmp(line, "READ") == 0) {   // if read command
            handleReadCommand(&heap);             // remove the top email
        } else if (strcmp(line, "COUNT") == 0) { // if count command
            handleCountCommand(&heap);          // show count
        } // any other line is ignored
    }

    heapFree(&heap);  // release the memory created from the heap array
    return 0; // kill program
}
