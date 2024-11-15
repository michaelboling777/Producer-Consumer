//********************************************************************
//
// Michael Boling
// Operating Systems
// Programming Project #4: Producer/Consumer Problem
// February 28, 2023
// Instructor: Dr. Siming Liu
//
//********************************************************************

#ifndef _BUFFER_H_DEFINED_
#define _BUFFER_H_DEFINED_

typedef int buffer_item;

#define BUFFER_SIZE 5

#define max_item_generation 100

#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>

// global variables:
sem_t sem_empty;
sem_t sem_full;
int keep_running = 1; // flag global variable
pthread_mutex_t mutex;
int head = 0;
int tail = 0;
int buff_count = 0; // Initializing buffer count to be "0"
int max_sleep_prior_exec = 0;

// for maintenaince of circular array:
buffer_item buffer_array[BUFFER_SIZE]; // this is the fixed size array of type buffer_item

// printing at the end of main() :
int rem_items = 0;
int items_prod = 0;
int items_cons = 0;
int times_buff_full = 0;
int times_buff_empty = 0;

char cml_string[3];
int show_snap = 3;

//********************************************************************
//
// insert_item Function
//
// This function is appropriately inserting items
// into the buffer. It checks if the buffer size is full and returns false if so.
// It increments buff_count if an item was inserted into the buffer and returns a true.
// This function is maintaining the queue of buffer_items using the
// fixed size-array of type buffer_item.
//
// Return Value
// ------------
// bool                         True/False if item was inserted
//
// Value Parameters
// ----------------
// buffer_item      item        The random number to be inserted in appropriate position
//
// Reference Parameters
// --------------------
// NONE
//
// Local Variable Data Dictionary: NONE
//
//*******************************************************************

bool insert_item(buffer_item item)
{

    // circular buffer logic:
    if (buff_count == BUFFER_SIZE)
    {
        times_buff_full++;
        return false;
    }
    buffer_array[head] = item;
    // since we added item, head++
    head = head + 1;
    // Now, as long as head doesnt equal buff_count size ... it wont be reset to "0" (because modulo arithmetic)

    head = (head % BUFFER_SIZE);

    // uppon successful insertion, increment buff_count
    buff_count = (buff_count + 1);

    if (buff_count == BUFFER_SIZE)
    {
        if (show_snap == 4)
        {
            printf("All buffers full.  Producer %ld waits.\n\n", pthread_self());
        }
        times_buff_full++;
    }

    return true;
}

//********************************************************************
//
// remove_item Function
//
// This function is appropriately removing items
// from the buffer. It checks if the buffer is empty and returns a false if so.
// It increments "tail" if an item was removed from the buffer and returns a true.
// This function is maintaining the queue of buffer_items using the
// fixed size-array of type buffer_item.
//
// Return Value
// ------------
// bool                         True/False if item was removed
//
// Value Parameters
// ----------------
// buffer_item      *item       The random number to be removed from appropriate position
//
// Reference Parameters
// --------------------
// NONE
//
// Local Variable Data Dictionary: NONE
//
//*******************************************************************

bool remove_item(buffer_item *item)
{
    // circular buffer logic:

    if (buff_count == 0)
    {
        times_buff_empty++;
        return false;
    }
    *item = buffer_array[tail];

    // since we removed item, tail++
    tail = tail + 1;
    // Now, as long as tail doesnt equal buff_count size ... it wont be reset to "0" (because modulo arithmetic)
    tail = (tail % BUFFER_SIZE);

    // uppon successful removal, decrement buff_count
    buff_count = (buff_count - 1);

    if (buff_count == 0)
    {
        if (show_snap == 4)
        {
            printf("All buffers empty.  Consumer %ld waits.\n\n", pthread_self());
        }
        times_buff_empty++;
    }

    return true;
}

//********************************************************************
//
// initialization_buffer Function
//
// This function is appropriately initialzing the buffer at the start of
// the program (to -1). We are also creating semaphores and mutex. Mutex's help
// us "lock" a certain section of code when inserting/removing items from the buffer.
// And the semaphore allows the threads to work together more efficiently.
//
// Return Value
// ------------
// void
//
// Value Parameters
// ----------------
// NONE
//
// Reference Parameters
// --------------------
// NONE
//
// Local Variable Data Dictionary: NONE
//
//*******************************************************************

void initialization_buffer()
{
    // only 1 process and no fully occupied slots when full
    sem_init(&sem_full, 0, 0);
    // only 1 process and all slots available when empty
    sem_init(&sem_empty, 0, BUFFER_SIZE);
    // creating the mutex:
    pthread_mutex_init(&mutex, NULL);
    // initalizing all buffer positions to "-1" at the start of the program:
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        buffer_array[i] = -1;
    }
}

//********************************************************************
//
// b_print Function
//
// This function is appropriately printing the values obtained from the circular buffer
// at optimal times (when this function is called). It uses global variables
// (visible among all threads) and situational calls (from the producer and consumer functions)
// in order to print out when a certain thread is
// "reading" or "writing" to a specific buffer and general formatting when wanting to output
// the "individual buffer snapshots" when declared "yes" from the command line arguments.
//
// Return Value
// ------------
// void
//
// Value Parameters
// ----------------
// NONE
//
// Reference Parameters
// --------------------
// NONE
//
// Local Variable Data Dictionary: NONE
//
//*******************************************************************
void b_print()
{
    printf("\n(Buffers occupied: %d)\nbuffers: ", buff_count);
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        if ((buff_count <= 9) && (buff_count >= 0))
            printf("  %d ", buffer_array[i]);
        else
            printf(" %d ", buffer_array[i]);
    }
    printf("\n        ");

    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        printf(" ----");
    }

    printf("\n         ");

    for (int i = 0; i < BUFFER_SIZE; i++)
    {

        if ((head == i) || (tail == i))
        {

            if (buff_count == 0)
            {
                printf("RW   ");
            }
            else if (buff_count == BUFFER_SIZE)
            {
                printf("WR   ");
            }
            else if (head == i)
            {
                printf("W    ");
            }
            else
            {
                printf("R    ");
            }
        }

        else
            printf("     ");
    }
    printf("\n");
}

//********************************************************************
//
// determine_if_prime Function
//
// This function determines if a number passed to it is indeed prime or not.
// The function will loop from 2 to the number passed through the function and
// evaluate if any of those cases turn out to be divisors to the number passed
// to the function itself. If its a divisor = not prime, else: it is prime.
//
// Return Value
// ------------
// bool                         True/False depending on if the # is prime.
//
// Value Parameters
// ----------------
// a        int     The number to be determined if it is prime or not
//
// Reference Parameters
// --------------------
// NONE
//
// Local Variable Data Dictionary: NONE
//
//*******************************************************************

bool determine_if_prime(int a)
{
    if (a < 2)
        return false;
    // if "i" is a divisor of "a" anywhere from 2 but not including "a"
    // then, its not prime... else it is prime
    for (int i = 2; i < a; i++)
    {

        if ((a % i) == 0)
            return false;
    }
    return true;
}

#endif // _BUFFER_H_DEFINED_
