//********************************************************************
//
// Michael Boling
// Operating Systems
// Programming Project #4: Producer/Consumer Problem
// February 28, 2023
// Instructor: Dr. Siming Liu
//
//********************************************************************

#include <stdlib.h> /* required for rand() */
#include <stdio.h>
#include "buffer.h"
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#include <unistd.h>
#include <string.h>

typedef struct thread_t
{
    pthread_t thread;
    int itemCount;
} thread_t;

/////////////////////////////////////////////

// Organization of all the functions:
void *consumer(void *param);
void *producer(void *param);
bool insert_item(buffer_item item);
bool remove_item(buffer_item *item);
void initialization_buffer();
void b_print();
bool determine_if_prime(int a);

////////////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------//
////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{

    // main() layout as such:
    //   Get command line arguments
    //   Initialize buffer
    //   Create producer thread(s)
    //   Create consumer thread(s)
    //   Sleep
    //   Join Threads
    //   Display Statistics
    //   Exit

    srand(time(NULL));

    char yes[] = "yes";
    char no[] = "no";
    int time_main_sleep = 0;
    int thread_consumer = 0;
    int sleep_prior = 0;
    int thread_produced = 0;

    // 1.) Getting CML arguments:

    // total length sleep time
    // max sleep time for producer and consumer threads
    // # of producer threads
    //  # of consumer threads
    // yes or no

    if (argc == 6)
    {
        for (int i = 1; i <= argc; i++)
        {
            if (i == 1)
                time_main_sleep = atoi(argv[i]);
            else if (i == 2)
                sleep_prior = atoi(argv[i]);
            else if (i == 3)
                thread_produced = atoi(argv[i]);
            else if (i == 4)
                thread_consumer = atoi(argv[i]);
            else if (i == 5)
                strcpy(cml_string, argv[i]);
            else
                continue;
        }
    }

    if (strcmp(cml_string, yes) == 0)
    {
        show_snap++;
    }

    max_sleep_prior_exec = sleep_prior;
    // 2.) Initialize buffer
    initialization_buffer();

    printf("Starting Threads...");
    b_print();
    if (show_snap != 4)
    {
        printf("You have chosen the \"no\" option... please wait for the program to display the statistics: \n\n");
    }

    // 3.) Create producer thread(s)

    thread_t prod_thread[thread_produced];
    for (int i = 0; i < thread_produced; i++)
    {
        prod_thread[i].itemCount = 0;

        pthread_create(&prod_thread[i].thread, NULL, producer, (void *)&prod_thread[i]);
    }

    // 4.) Create consumer thread(s)
    thread_t cons_thread[thread_consumer];

    for (int j = 0; j < thread_consumer; j++)
    {
        cons_thread[j].itemCount = 0;

        pthread_create(&cons_thread[j].thread, NULL, consumer, (void *)&cons_thread[j]);
    }

    // 5.) Sleep

    sleep(time_main_sleep);

    // setting flag to false:
    keep_running = 0;

    // 6.) Join Threads

    for (int i = 0; i < thread_produced; i++)
    {

        // So the program wont "hang":
        pthread_cancel(prod_thread[i].thread);

        if (pthread_join(prod_thread[i].thread, NULL) != 0)
        {

            printf("Failed to join thread");
        }
    }

    for (int j = 0; j < thread_consumer; j++)
    {

        // So the program wont "hang":
        pthread_cancel(cons_thread[j].thread);

        if (pthread_join(cons_thread[j].thread, NULL) != 0)
        {

            printf("Failed to join thread");
        }
    }

    rem_items = buff_count;

    // 7.) Display Statistics

    printf("PRODUCER / CONSUMER SIMULATION COMPLETE\n");
    printf("=======================================\n");
    printf("Simulation Time:                        %d\n", time_main_sleep);
    printf("Maximum Thread Sleep Time:              %d\n", sleep_prior);
    printf("Number of Producer Threads:             %d\n", thread_produced);
    printf("Number of Consumer Threads:             %d\n", thread_consumer);
    printf("Size of Buffer:                         %d\n\n", BUFFER_SIZE);
    printf("Total Number of Items Produced:         %d\n", items_prod);

    for (int i = 0; i < thread_produced; i++)
    {
        printf("  Thread %d:                %d\n", (i + 1), prod_thread[i].itemCount);
    }
    printf("\n");

    printf("Total Number of Items Consumed:         %d\n", items_cons);

    for (int i = thread_produced; i < (thread_consumer + thread_produced); i++)
    {
        printf("  Thread %d:                %d\n", (i + 1), cons_thread[i - thread_produced].itemCount);
    }
    printf("\n");
    printf("Number Of Items Remaining in Buffer:    %d\n", rem_items);
    printf("Number Of Times Buffer Was Full:        %d\n", times_buff_full);
    printf("Number Of Times Buffer Was Empty:       %d", times_buff_empty);
    printf("\n\n");
    // 8.) Exit

    return -1;
}

////////////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------//
////////////////////////////////////////////////////////////////////////////////////

//********************************************************************
//
// *producer Function
//
// This function will produce a random #. And prior to any "produces".. it will sleep
// for a random time (less than the max specified via cml args). It will "wait" if
// the buffer is "full"... then enter a critical section (via mutex's and semaphores)
// where the random # is inserted into the buffer by calling insert_item(). It will then
// "unlock" the critical state. This function will run until keep_running has value of "0" (reached upon main() funtion).
//
// Return Value
// ------------
// VOID
//
// Value Parameters
// ----------------
// param        void
//
// Reference Parameters
// --------------------
// NONE
//
// Local Variable Data Dictionary:
// buffer_item item = This is an int value of buffer_item created when producer()
// is called and inserted into the buffer when insert_item() function is called.
// sec_sleep = this is the random time (less than max specified via command line arg)
// that the producer thread sleeps for prior to insertion of an item.
// bool t = this returns the value of either T/F if an item was inserted into the buffer.
// thread_t *thr = used for getting how many times a certain thread produces
//*******************************************************************

void *producer(void *param)
{
    thread_t *thr = (thread_t *)param;
    buffer_item item;
    int sec_sleep = 0;

    while (keep_running == 1)
    {

        sec_sleep = (rand()) % (max_sleep_prior_exec);

        sleep(sec_sleep);

        if (keep_running != 0)
        {
            // generates "item" less than 100
            item = (rand()) % (max_item_generation);
            sem_wait(&sem_empty);
            pthread_mutex_lock(&mutex);

            /*** CRITICAL SECTION ***/
            bool t = insert_item(item);
            if (keep_running == 1)
            {
                // if insert_item() returned false:
                if (t == false)
                {
                    if (show_snap == 4)
                    {

                        printf("All buffers full.  Consumer %ld waits.\n\n", pthread_self());
                    }
                }
                else
                {
                    if (show_snap == 4)
                    {
                        printf("Producer %ld writes %d", pthread_self(), item);

                        b_print();
                        printf("\n");
                    }

                    items_prod++;
                }
            }
            thr->itemCount++;
        }

        pthread_mutex_unlock(&mutex);
        // added element to buffer...so incrementing semaphore
        sem_post(&sem_full);

        /***  END CRITICAL SECTION ***/
    }
}

//********************************************************************
//
// *consumer Function
//
// This function will consume the random # produced by the producer() function.
// Prior to any "consumes".. it will sleep for a random time (less than the max specified via cml arg).
// It will "wait" if the buffer is "empty"... then enter a critical section (via mutex's and semaphores)
// where the random #  will be consumed. The consumed # will be checked if it was prime and print
// out if so. It will then "unlock" the critical state. This function will run until
// keep_running has value of "0" (reached upon main() funtion).
//
// Return Value
// ------------
// VOID
//
// Value Parameters
// ----------------
// param        void
//
// Reference Parameters
// --------------------
// NONE
//
// Local Variable Data Dictionary:
// buffer_item item = This is an int value of buffer_item created when producer()
// is called and inserted into the buffer when insert_item() function is called.
// sec_sleep = this is the random time (less than max specified via command line arg)
// that the consumer thread sleeps for prior to consumption of an item.
// bool t = this returns the value of either T/F if an item was removed from the buffer.
// thread_t *thr = used for getting how many times a certain thread consumes
//*******************************************************************

void *consumer(void *param)
{
    thread_t *thr = (thread_t *)param;
    buffer_item item;
    int sec_sleep = 0;

    while (keep_running == 1)
    {
        // sleeps less than 10 seconds atleast
        sec_sleep = (rand()) % (max_sleep_prior_exec);

        sleep(sec_sleep);

        // NOTE this "if" statement makes so that when the global flag variable is called in "main"
        // sem_wait and mutex_lock is called so that it will prevent an infinite "hang" of
        // the program which occurs when "sleep" is called but you call the pthread_join() .. hence, there can be an overlap.
        if (keep_running != 0)
        {

            sem_wait(&sem_full);
            pthread_mutex_lock(&mutex);

            /*** CRITICAL SECTION ***/

            bool t = remove_item(&item);
            if (keep_running == 1)
            {
                // if remove_item() returned false:
                if (t == false)
                {
                    if (show_snap == 4)
                    {

                        printf("All buffers empty.  Consumer %ld waits.\n\n", pthread_self());
                    }
                }
                else
                {
                    if (show_snap == 4)
                    {
                        printf("Consumer %ld reads %d", pthread_self(), item);

                        // detecting if # consumed is prime:
                        if (determine_if_prime(item) == true)
                        {
                            printf("   * * * PRIME * * *");
                        }

                        b_print();
                        printf("\n");
                    }

                    items_cons++;
                }
            }
            thr->itemCount++;
        }

        pthread_mutex_unlock(&mutex);
        // posting that there is an empty slot in semaphore because we consumed
        sem_post(&sem_empty);

        /***  END CRITICAL SECTION ***/
    }
}

// You have reached the end of my .c file ... :)
