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

volatile int num_occurences;

// a run for every thread, increments num of occurences of the word in article
void NumArticleOccurences(struct Article* article, char *word) {

    char *curr_word = article->words;
    
    for (int i = 0; i < article->numWords; i++) {
        if (*curr_word == *word) {
            //do I lock here???? wtf
            num_occurences++;
        }
        curr_word++;
    }
}




int CountOccurrences( struct  Library * lib, char * word )
{
    // create thread to run for each article
    // have global num_occurences value increment per each occurence
    // use ptr arithmetic to parse through ** char

    char *curr_article_ptr = lib->articles;

    for(int i = 0; i < lib->numArticles; i++) {
        pthread_create(...)
        curr_article_ptr++; //point to the next article
    }


    return 0;
}

