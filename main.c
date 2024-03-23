//Imports
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>

//Define the four variables in the instructions
#define LOWER_NUM 1
#define UPPER_NUM 10000
#define BUFFER_SIZE 100
#define MAX_COUNT 10000

//Create buffer with the correct size
int buffer[BUFFER_SIZE];

//Count is used to track the top of the stack
int count = 0;

//A counter for the number of numbers consumed by the 2 consumers
int total_consumed = 0;

//A flag that's turned on after the producer finishes producing
int terminate = 0;

//Declare mutex lock
pthread_mutex_t mutex;

//Declare two semaphores: empty and full
sem_t empty;
sem_t full;

//Producer
void *producer(void *args) {
    //Open the all.txt file in writing mode
    FILE *all_file = fopen("all.txt", "w");
    
    //Loop MAX_COUNT number of times
    for (int i = 0; i < MAX_COUNT; i++) {
        //Generate a random number between LOWER_NUM and UPPER_NUM
        int num = LOWER_NUM + (rand() % (UPPER_NUM - LOWER_NUM + 1));
        
        //Wait until an empty space exists on the buffer
        //If one is present, decrement the number of empty spaces by 1
        sem_wait(&empty);
        
        //Acquire the mutex lock
        pthread_mutex_lock(&mutex);
        
        //Input the randomly generated number into its correct position on the buffer
        buffer[count] = num;
        
        //Increment count to update the pointer to the top of the stack
        count++;
        
        //Write the number to all.txt
        fprintf(all_file, "%d\n", num);
        fflush(all_file);
        
        //Release the mutex so it can be acquired by others
        pthread_mutex_unlock(&mutex);
        
        //Since a new number has been added, increment the number of full spaces by 1
        sem_post(&full);
    }
    //Close the all.txt file
    fclose(all_file);
  
    printf("Producer done\n");
    
    //Set the terminate flag to 1 since the producer has finished producing
    terminate = 1;
    
    //Increment full 1 more time. This is needed to wake up any blocked consumers on termination
    sem_post(&full);
}

//Even consumer
void *even_consumer(void *args) {
    //Open the even.txt file in writing mode
    FILE *even_file = fopen("even.txt", "w");
    
    //Loop as long as total_consumed is not the max count
    while (total_consumed != MAX_COUNT) {
      //Wait until a full space exists.
      //If one exists, decrease number of full spaces by 1
      sem_wait(&full);
      
      //Acquire mutex
      pthread_mutex_lock(&mutex);
      
      //If the producer is no longer producing numbers, and the buffer is empty
      //Release the mutex and stop the loop
      if (terminate && count == 0) {
            pthread_mutex_unlock(&mutex);
            break; 
      }
      
      //Take number from top of the stack
      int num = buffer[count - 1];
      
      //If even
      if (num % 2 == 0) {
          //Write to the even.txt file
          fprintf(even_file, "%d\n", num);
          fflush(even_file);
          
          //Decrement count to update the top of the stack
          count--;
          
          //Increment total_consumed by 1
          total_consumed++;
          
          //Increment the number of empty spaces by 1 (since it has just been consumed)
          sem_post(&empty);
      }
      
      else {
        //Revert the decrement to full spaces by incrementing it again
        sem_post(&full);
      }
      
      //Release mutex so others can acquire it
      pthread_mutex_unlock(&mutex);
    }
    printf("even consumer done\n");
    
    //Close the even.txt file
    fclose(even_file);
}

//Odd customer
void *odd_consumer(void *args) {
    //Open the odd.txt file in writing mode
    FILE *odd_file = fopen("odd.txt", "w");
    
    //Loop as long as total_consumed is not MAX_COUNT
    while (total_consumed != MAX_COUNT) {
      //Wait until a full space exists.
      //If one exists, decrease number of full spaces by 1
      sem_wait(&full);
      
      //Acquire mutex
      pthread_mutex_lock(&mutex);
      
      //If the producer is no longer producing numbers, and the buffer is empty
      //Release the mutex and stop the loop
      if (terminate && count == 0) {
            pthread_mutex_unlock(&mutex);
            break; 
      }
      
      //Take number from top of the stack
      int num = buffer[count - 1];
      
      //If odd
      if (num %2 != 0) {
          //Write to the odd.txt file
          fprintf(odd_file, "%d\n", num);
          fflush(odd_file);
          
          //Decrement count to update the top of the stack
          count--;
          
          //Increment total_consumed by 1
          total_consumed++;
          
          //Increment the number of empty spaces by 1 (since it has just been consumed)
          sem_post(&empty);
      }
      
      else {
        //Revert the decrement to full spaces by incrementing it again
        sem_post(&full);
      }
      
      //Release mutex so others can acquire it
      pthread_mutex_unlock(&mutex);
    }
    printf("odd consumer done\n");
    
    //Close the odd.txt file
    fclose(odd_file);
}

int main() {
    //This is to ensure numbers are random every time
    srand(time(NULL));
    
    //Declare all 3 threads
    pthread_t producer_thread, even_consumer_thread, odd_consumer_thread;
    
    //Initialize the mutex
    pthread_mutex_init(&mutex, NULL);
    
    //Initialize semaphores, where there are BUFFER_SIZE number of empty spaces, and 0 full spaces
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);
    
    //Creating the threads
    pthread_create(&producer_thread, NULL, &producer, NULL);
    pthread_create(&even_consumer_thread, NULL, &even_consumer, NULL);
    pthread_create(&odd_consumer_thread, NULL, &odd_consumer, NULL);
    
    //Joining the threads
    pthread_join(producer_thread, NULL);
    pthread_join(even_consumer_thread, NULL);
    pthread_join(odd_consumer_thread, NULL);
    
    //Destroying the mutex after use
    pthread_mutex_destroy(&mutex);
    
    //Destroying the semaphores after use
    sem_destroy(&empty);
    sem_destroy(&full);
    return 0;
}

