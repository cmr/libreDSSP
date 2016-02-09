#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "dict.h"
#include "elem.h"
#include "util.h"
#include "corewords.h"

// Searches non-core dictionaries, returns word if it exists
word * wordSearch(char * name, dict * vocab){
	coreword * tempCore;
	word * tempWord;

	// Function must have a name greater than 1 char
	if(name[0] == '\0') return NULL;

	if(vocab->sub->wordlist != NULL){
		// Search subdicts (for now just one)
		tempWord = vocab->sub->wordlist;
		do{
			if(!strcmp(tempWord->name, name)){
				return tempWord;
			}
			tempWord = tempWord->next;
		}while(tempWord != NULL);
	}

	return NULL;
}

// Searches dictionaries, runs a word if possible
elem * wordRun(elem * sequence, elem * stack, dict * vocab){
	char * elemName = sequence->chars;
	coreword * tempCore;
	word * tempWord;

	if(elemName[0] == '\0') return stack;

	// Search core dict first
	tempCore = vocab->core;
	do{
		if(!strcmp(tempCore->name, elemName)){
			// Run built-in function. sequence provided so that conditionals can conditionally delete adjacent words.
			stack = tempCore->func(stack, sequence);
			return stack;
		}
		tempCore = tempCore->next;
	}while(tempCore != NULL);

	if(vocab->sub->wordlist != NULL){
		// Search subdicts (for now just one)
		tempWord = vocab->sub->wordlist;
		do{
			if(!strcmp(tempWord->name, elemName)){
				// run programmed word
				stack = run(stack, parseInput(tempWord->definition), vocab);
				return stack;
			}
			tempWord = tempWord->next;
		}while(tempWord != NULL);
	}

	fprintf(stderr,"ERROR: %s unrecognized\n",elemName);
	return stack;
}

word * newWord(subdict * dict){
	assert(dict != NULL);
	word * temp; // Lets us find the next empty spot

	if(dict->wordlist == NULL){ // First word in dictionary
		dict->wordlist = malloc(sizeof(word));
		temp = dict->wordlist;
	} else { // Not first word in dictionary
		temp = dict->wordlist;
		while(temp->next != NULL) {
			temp = temp->next;
		}
		temp->next = malloc(sizeof(word));
		temp = temp->next;
	}
	// temp should now point to fresh word
	return temp;
}

// Attempts to define a new function
// TODO Should replace if function exists, or error if core function
elem * defWord(elem * seq, dict * vocab){
	word * temp;
	assert(vocab != NULL);
	assert(vocab->sub != NULL);

	// Skip over ':'
	if(seq->next != NULL){
		seq = seq->next;
	}else{
		fprintf(stderr,"ERROR: Incomplete definition\n");
	}

	// Assign function name
	if(seq->next != NULL){
		// See if we already have this word
		temp = wordSearch(seq->chars, vocab);
		if(temp == NULL){
			// Append the new word
			temp = newWord(vocab->sub);
			// Name the new word
			strcpy(temp->name, seq->chars);
		}else{
			// Wipe old definition
			strcpy(temp->definition, "");
		}
		seq = seq->next;
	}else{
		fprintf(stderr,"ERROR: Incomplete definition\n");
	}

	// TODO This limits the size of a declaration and should be dynamic
	while(seq->chars[0] != ';'){
		strcat(temp->definition, seq->chars);
		strcat(temp->definition, " ");
		if(seq->next == NULL){
			fprintf(stderr,"ERROR: Incomplete definition\n");
			return NULL; // TODO We should free the sequence before returning
		}else{
			seq = seq->next;
		}
	}
	printf("%s\n",temp->name);

	// TODO We should free the entire sequence
	return NULL;
}

void defCore(char * name, elem * (*func)(elem *, elem*), dict * vocab){
	coreword * temp = vocab->core;
	if(vocab->core == NULL){
		temp = malloc(sizeof(coreword));
		strcpy(temp->name, name);
		temp->func = (*func);
		temp->next = NULL;
	}
	// Find last defined func
	while(temp->next != NULL){
		temp = temp->next;
	}
	temp->next = malloc(sizeof(coreword));
	strcpy(temp->next->name, name);
	temp->next->func = (*func);
	temp->next->next = NULL;
	return;
}


