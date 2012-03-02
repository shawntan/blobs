/* Copyright (c) 2012, Rahul AG
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <wchar.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <assert.h>

typedef enum {
	BLOB_R,
	BLOB_RO,
	BLOB_L,
	BLOB_LO
} blob_t;

typedef struct my_node {
	char fname[255];
	struct my_node* next;
} ll_node;

void screen_clr(FILE* out) {
	fprintf(out, "\x1b[2J\n");
}

void line_clr(FILE* out) {
	fprintf(out, "\x1b[1A\x1b[2K\n");
}
/*
void blob_pr(blob_t blob, FILE* out) {
	switch (blob) {
		case BLOB_R:
			fprintf(out, "(^.^) \n");
			break;
		case BLOB_RO:
			fprintf(out, " (^.^)\n");
			break;
		case BLOB_L:
			fprintf(out, "(^.^) \n");
			break;
		case BLOB_LO:
			fprintf(out, " (^.^)\n");
			break;
		default:
			break;
	}
}
*/
void blob_pr(blob_t blob, FILE* out) {
	switch (blob) {
		case BLOB_R:
			fprintf(out,"%ls", L"\x1b[0;35m \x00002308\x1b[0;36m(^.^)\x1b[0;35m\x0000230B \n");
			break;
		case BLOB_RO:
			fwprintf(out, L"\x1b[0;35m\x00002308\x0000203E\x1b[0;36m(^.^)\x1b[0;35m_\x0000230B\n");
			break;
		case BLOB_L:
			fwprintf(out, L"\x1b[0;35m \x0000230A\x1b[0;36m(^.^)\x1b[0;35m\x00002309 \n");
			break;
		case BLOB_LO:
			fwprintf(out, L"\x1b[0;35m\x0000230A_\x1b[0;36m(^.^)\x1b[0;35m\x0000203E\x00002309\n");
			break;
		default:
			break;
	}
	fflush(stdout);
}
int main (int argc, char** argv) {
	pid_t whoami;
	blob_t blob = BLOB_R;
	int count = 0, ii, jj, cmode=0, ttycnt=0;
	FILE** files;
	DIR* dev = opendir("/dev");
	char* loc;
	struct dirent* ent;
	ll_node* head, *cur;
	
	head = malloc(sizeof(ll_node));
	cur = head;
	
	assert(dev);
	
	while ((ent = readdir(dev)) != NULL) {
		if (!strncmp(ent->d_name, "tty", 3)) {
			if (ent->d_name[3] >= '0' && ent->d_name[3]<= '9') {
				strncpy(cur->fname, ent->d_name, 254);
				cur->next = malloc(sizeof(ll_node));
				cur = cur->next;
				++ttycnt;
			}
		}
	}
	
	files = malloc(sizeof(FILE*) * ttycnt);
	
#ifdef FORKBG
	whoami = fork();
	if (whoami > 0) {
		return 0;
	}
#endif
	setbuf(stdout,NULL);
	cur = head;
	loc = malloc(sizeof(char) * 128);
	strcpy(loc, "/dev/");
	
	for (jj = 0; jj < ttycnt; ++jj) {
		strcpy(&(loc[5]), cur->fname);
		fprintf(stdout, "Capturing: %s\n", loc);
		files[jj] = fopen(loc, "w");
		assert(files[jj]);
		screen_clr(files[jj]);
		cur = cur->next;
	}
	
	//setlocale(LC_ALL, "");
	
	while (1) {
		for (jj = 0; jj < ttycnt; ++jj) {
			for (ii = 0; ii < count; ++ii) {
				fprintf(files[jj], " \n");
			}
			blob_pr(blob, files[jj]);
		}
		printf("tick\n");
		usleep(400000);
		if (blob % 2) {
			--blob;
		} else {
			++blob;
		}
		if (cmode) {
			--count;
		} else {
			++count;
		}
		if (count == 10 || count == 0) {
			if (blob > 1) {
				blob = 0;
			} else {
				blob = 2;
			}
			cmode = !cmode;
		}
		for (jj = 0; jj < ttycnt; ++jj) {
			line_clr(files[jj]);
		}
	}
	
	return 0;
}
