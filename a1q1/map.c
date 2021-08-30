/* map.c
 * ----------------------------------------------------------
 *  CS350
 *  Assignment 1
 *  Question 1
 *
 *  Purpose:  Gain experience with threads and basic 
 *  synchronization.
 *
 *  YOU MAY ADD WHATEVER YOU LIKE TO THIS FILE.
 *  YOU CANNOT CHANGE THE SIGNATURE OF CountOccurrences.
 * ----------------------------------------------------------
 */
#include "data.h"
#include <string.h>
#include <stdlib.h>
/* --------------------------------------------------------------------
 * CountOccurrences
 * --------------------------------------------------------------------
 * Takes a Library of articles containing words and a word.
 * Returns the total number of times that the word appears in the 
 * Library.
 *
 * For example, "There the thesis sits on the theatre bench.", contains
 * 2 occurences of the word "the".
 * --------------------------------------------------------------------
 */

int volatile num_occurences = 0;

int volatile num_threads_done = 0;
int num_threads; // global for num_articles

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;

// also create a struct to pass in the arguments
// structure used to pass as argument to threads
struct Finder {
    struct Article *article;
    char* word;
};


// a run for every thread, increments num of occurences of the word in article
void* NumArticleOccurences(void *my_finder) {

    struct Finder* finder = (struct Finder*)my_finder;

    char **curr_word = (finder->article)->words;
    
    for (int i = 0; i < (finder->article)->numWords; i++) {
        if (strcmp(curr_word[i], finder->word) == 0) {
            pthread_mutex_lock(&mutex2);
            num_occurences++; 
            pthread_mutex_unlock(&mutex2);
        }
    }

    pthread_mutex_lock(&mutex);
    num_threads_done++;
    if (num_threads_done == num_threads) {
        pthread_cond_signal(&cv);
    }
    pthread_mutex_unlock(&mutex);

    free(finder);
    pthread_exit(NULL);
}




int CountOccurrences( struct  Library * lib, char * word )
{
    // create thread to run for each article
    // have global num_occurences value increment per each occurence
    // use ptr arithmetic to parse through ** char

    num_threads = lib->numArticles;

    pthread_t workerThreads[num_threads];

    for(int i = 0; i < num_threads; i++) {

        // create Finder struct to pass through 
        struct Finder *finder = malloc(sizeof(void*));
        finder->article = lib->articles[i];
        finder->word = word;
        pthread_create(&workerThreads[i], NULL, NumArticleOccurences, (void*) finder);
        
    }

    pthread_mutex_lock(&mutex);
    while(num_threads_done < num_threads) {
        pthread_cond_wait(&cv, &mutex);
    }
    pthread_mutex_unlock(&mutex);

    return num_occurences;

}

