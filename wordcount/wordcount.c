/*-
 * Copyright (c) 2019 Rodrigo Osorio <rodrigo@osorio.me>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in this position and unchanged.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STR_BUFF 	1024
#define END_NODE_FLG 	0x01

struct word_t {
	char * string;
	int count;
	struct word_t * next;
};
	
struct node_t {
	struct node_t * next;
	struct node_t * head;
	struct word_t * word;
	char flags;
	char value;
};

struct context_t {
	struct word_t * wordlist;
	struct node_t * dicthead;
	int wordcount;
};

/**
 * @brief print usage
 */
void
usage()
{
	fprintf(stderr,"usage: wordcount DICTFILE [FILE....]\n"
           "  DICTFILE : dictionary file\n"  
	   "  FILE     : files to parse\n");
	exit(1);
}

/**
 * @brief trim a string
 */
static void
trim_str(char ** pbuff)
{
	char * psz = *pbuff;

	while( *psz && isspace(*psz))
		psz++;	
	*pbuff = psz;
	
	for( psz = *pbuff; *psz ; psz++){
		if(isspace(*psz)){
			*psz = '\0';
			break;
		}
	}
}

/**
 * @brief insert a new node in the dictionary tree
 */
struct node_t * insert_new_node(struct node_t ** head,char c)
{
	struct node_t * pn = *head;
	struct node_t * new;
	
	while(pn) {
		if(c == pn->value) {
			return pn;
		} else {
			if(NULL == pn->next || pn->next->value > c)
				break;
			pn = pn->next;
		}
	}
		
	new = malloc(sizeof(struct node_t));
	if(NULL == new){
		perror("malloc");
		return NULL;
	}
	memset(new,0,sizeof(struct node_t));
	new->value = c;

	if(NULL == pn)
		*head = new;
	else {
		new->next = pn->next;
		pn->next  = new;
	} 

	return new;
}

/**
 * @brief insert a new word in the dictionary
 */
int
insert_dic_word(struct node_t ** head, char * word)
{
	struct node_t * pn;
	struct node_t ** ph = head;
	char * psz = word;

	while(*psz){
		pn = insert_new_node(ph,*psz);
		if(NULL == pn)
			return -1;
		ph = &(pn->head);
		psz++;
	}
	pn->flags = END_NODE_FLG;
	return 0;
}

/**
 * @brief load the dictionary file
 */
int
load_dic(char * filename, struct context_t * ctx)
{
	char * pbuff;
	char buffer[MAX_STR_BUFF+1];
	FILE * fs;
	int err = 0;

	fs = fopen(filename,"rb");
	if(NULL == fs){
		perror("load_dic : can't open dictionary file");
		return -1;
	}

	while(0 == err && (fgets(buffer,MAX_STR_BUFF,fs)) != NULL){
		err = ferror(fs);
		if(err)
		{
			perror("fgets");
			break;
		}
		pbuff = buffer;
		trim_str(&pbuff);

		/* First character is # or buffer is empty after trim */
		if('#' == *pbuff || '\0' == *pbuff)
			continue;
		else
			err = insert_dic_word(&(ctx->dicthead),pbuff);
	}

	fclose(fs);
	return err;
}

/**
 * @brief check if a word is valid in the dictionary
 */
struct node_t *
is_word_valid(struct node_t * head, char * word)
{
	struct node_t * ph = head,
		      * pn;	
	char * psz = word;
	
	while(*psz && ph){
		pn = ph;
		while(pn){
			if(*psz == pn->value){
				break;
			} else {
				pn = pn->next;
			}
		}
		ph = pn?pn->head:NULL;
		psz++;
	}
	
	if(*psz == '\0' && pn && END_NODE_FLG == pn->flags)
		return pn;

	return NULL;
}

/**
 * @brief process inputs from a file stream
 */
int
search_from_fs(FILE * fs,struct context_t * ctx) 
{
	char buffer[MAX_STR_BUFF+1];
	char word[MAX_STR_BUFF+1];
	int count,i,wpos = 0;
	struct node_t * pn;

	while((count = fread(buffer,1,MAX_STR_BUFF,fs))>0){
		for(i=0;i<count;i++){
			if(isspace(buffer[i])||
			   ispunct(buffer[i])){
				if(wpos){ 
					word[wpos] = '\0';
					ctx->wordcount++;
					pn = is_word_valid(ctx->dicthead,word);
					if(pn){
						if(pn->word == NULL){
							pn->word = malloc(sizeof(struct word_t));
							if(NULL == pn->word){
								perror("malloc");
								return -1;
							}
							memset(pn->word,0,sizeof(struct word_t));
							pn->word->string = strdup(word);
							if(NULL == pn->word->string){
								free(pn->word);
								perror("malloc");
								return -1;
							}
							pn->word->next = ctx->wordlist;
							ctx->wordlist = pn->word;
						}
						pn->word->count++;
							
					}
					wpos = 0;
				}
			} else {
				word[wpos++] = buffer[i];
			}
		}
	}
	
	return 0;
}

/**
 * @brief process inputs from a file name
 */
int
search_from_file(char * filename, struct context_t * ctx)
{
	int retval = 0;
	FILE * fs = fopen(filename,"rb");
	if(fs == NULL){
		perror("fopen");
		return -1;
	}

	retval = search_from_fs(fs,ctx);
	
	return retval;
	
}

/**
 * @brief free a dictionary node
 */
void
free_node( struct node_t * pnode )
{
	struct node_t * pn, * pnpre;
	for(pnpre = NULL, pn=pnode ; pn ; pn = pn->next ){
		if(pnpre){
			free_node(pnpre->head);
			free(pnpre);
		}
		pnpre = pn;
	}
	
}

/**
 * @brief free the context
 */
void
free_context(struct context_t * ctx)
{
	struct word_t * pw, * pwpre;
	struct node_t * pn, * pnpre;

	/* Free wordlist */
	for(pwpre = NULL, pw=ctx->wordlist ; pw ; pw = pw->next ){	
		if(pwpre){
			free(pwpre->string);
			free(pwpre);
		}
		pwpre = pw;
	}

	/* Free tree */
	free_node(ctx->dicthead);
	
}

/**
 * main
 */
int
main( int argc, char * argv[])
{
	struct context_t ctx;
	memset(&ctx,0,sizeof(ctx));
	struct word_t * pw;
	int i;
	int retval = 0;

	/* At least we need a distionary file */
	if(argc < 2)
		usage();

	/*load  the dictionary */
	retval = load_dic(argv[1],&ctx);

	/* If no error display the summary */
	if( 0 == retval )
	{
		/* Parse the incomming */
		if( 2 == argc)
			retval = search_from_fs(stdin,&ctx);
		else {
			for(i=2; i<argc;i++) {
				retval = search_from_file(argv[i],&ctx);
			}
		}
	}

	/* If no error display the summary */
	if(0 == retval){
		/* Print summary */
		for( pw=ctx.wordlist ; pw ; pw = pw->next ){
			printf("%5d\t%s\n",pw->count,pw->string);
		}
		printf("total words:\t%d\n",ctx.wordcount);
	}

	/* Free the memory */
	free_context(&ctx);

	return retval;
}
