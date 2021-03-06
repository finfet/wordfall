/*
This file is part of Word Fall.

Word Fall is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Word Fall is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Word Fall.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include "gamestates.h"
#include "state.h"
#include "render.h"
#include "main.h"
#include "game-state.h"
#include "hashtable.h"

int letterDrag = 0;
int letter1 = 0;
int letter2 = 0;
int randomLetter = 0;
int randomFallSpot = 0;
char containerAscii[8] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', '\0'};
int scoreValues[26] = {1,4,4,2,1,4,3,3,1,10,5,2,4,2,1,4,10,1,1,1,2,5,4,8,3,10};
int scoreArray[7] = {-1,-1,-1,-1,-1,-1,-1};
// This is the scrabble distribution for letters
int letterDistribution[] = {
0,0,0,0,0,0,0,0,0,1,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,5,5,6,6,6,7,7,8,8,8,8,8,8,8,8,8,
9,10,11,11,11,11,12,12,13,13,13,13,13,13,14,14,14,14,14,14,14,14,15,15,16,
17,17,17,17,17,17,18,18,18,18,19,19,19,19,19,19,20,20,20,20,21,21,22,22,23,24,24,25
};
int addScore = 0;
int addScoreFail = 0;
int addScoreWin = 0;
int addTransition = 0;
float addTransitionY = 550;

int lettersY[4][26] = {{0}};
int lettersX[4][26] = {{0}};

SDL_Rect lettersRect[4][26];
SDL_Rect submitRect;
SDL_Rect containerRect[7];
SDL_Rect pauseRect;

int theScore = 0;
int finalScore = 0;
int oldScore = 0;

void game_init(void)
{
	letterDrag = 0;
	letter1 = 0;
	letter2 = 0;
	randomLetter = 0;
	randomFallSpot = 0;
	for(int i = 0; i < 8; i++) {
		if(i == 7) {
			containerAscii[i] = '\0';
		}
		else {
			containerAscii[i] = ' ';
		}
	}
	for(int i = 0; i < 7; i++) {
		scoreArray[i] = -1;
		containerLetters[i] = 0;
	}

	addScore = 0;
	addScoreFail = 0;
	addTransition = 0;
	addTransitionY = 550;

	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 26; j++) {
			lettersX[i][j] = 0;
			lettersY[i][j] = 0;
		}
	}

	theScore = 0;
	finalScore = 0;

	submitRect.x = submitX;
	submitRect.y = submitY;
	submitRect.w = submit->clip_rect.w;
	submitRect.h = submit->clip_rect.h;

	pauseRect.x = pauseX;
	pauseRect.y = pauseY;
	pauseRect.w = pause->clip_rect.w;
	pauseRect.h = pause->clip_rect.h;

	for(int i = 0; i < 7; i++) {
		containerRect[i].x = containerX[i];
		containerRect[i].y = containerY;
		containerRect[i].w = container[i]->clip_rect.w;
		containerRect[i].h = container[i]->clip_rect.h;
	}

	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 26; j++) {
			lettersX[i][j] = (i*75)+45;
			lettersY[i][j] = 45;
			lettersRect[i][j].w = letters[i][j]->clip_rect.w;
			lettersRect[i][j].h = letters[i][j]->clip_rect.h;
		}
	}
	timeStart = SDL_GetTicks();
}

void game_events(void)
{
	while(SDL_PollEvent(&event)) {
		if(event.type == SDL_QUIT) {
			set_next_state(STATE_EXIT);
		}
		else if(event.key.keysym.sym == SDLK_ESCAPE) {
			set_next_state(STATE_EXIT);
		}
		else if(event.type == SDL_MOUSEBUTTONDOWN) {
			if(event.button.button == SDL_BUTTON_LEFT) {
				if((event.motion.x > submitRect.x) &&
					(event.motion.x < submitRect.x + submitRect.w) &&
					(event.motion.y > submitRect.y) &&
					(event.motion.y < submitRect.y + submitRect.h)) {
					// Dangling pointers all around.
					char *safeWord = strdup(containerAscii);
					sanitize(safeWord);
					if(isword(safeWord) == 1){
						addScore = 1;
						addScoreWin = 1;
						for(int i = 0; i < 7; i++) {
							if(scoreArray[i] != -1) {
								theScore += scoreValues[scoreArray[i]];
							}
							containerLetters[i] = 0;
							containerAscii[i] = 32;
							scoreArray[i] = -1;
						}
						finalScore += theScore;

						// Play win sound
						if(sound) {
							if(Mix_PlayChannel(-1, win, 0) == -1) {
								fprintf(stderr, "win sound failed!\n%s\n", Mix_GetError());
							}
						}
					}
					else {
						addScore = 1;
						addScoreFail = 1;
						addScoreWin = 0;
						for(int i = 0; i < 7; i++) {
							scoreArray[i] = -1;
							containerLetters[i] = 0;
							containerAscii[i] = 32;
						}
						if(sound) {
							if(Mix_PlayChannel(-1, error, 0) == -1) {
								fprintf(stderr, "error sound failed!\n%s\n", Mix_GetError());
							}
						}
					}
					free(safeWord);
				}
				else if((event.motion.x > pauseRect.x) &&
					(event.motion.x < pauseRect.x + pauseRect.w) &&
					(event.motion.y > pauseRect.y) &&
					(event.motion.y < pauseRect.y + pauseRect.h)) {
					set_next_state(STATE_PAUSE);
				}
				for(int i = 0; i < 4; i++) {
					for(int j = 0; j < 26; j++) {
						if((event.motion.x > lettersRect[i][j].x) &&
							(event.motion.x < lettersRect[i][j].x + lettersRect[i][j].w) &&
							(event.motion.y > lettersRect[i][j].y) &&
							(event.motion.y < lettersRect[i][j].y + lettersRect[i][j].h)
							&& (event.motion.y > 40)) {
							letterDrag = 1;
							letter1 = i;
							letter2 = j;
						}
					}
				}
			}
			if(event.button.button == SDL_BUTTON_RIGHT) {
				for(int i = 0; i < 7; i++) {
					if((event.motion.x > containerRect[i].x) &&
					(event.motion.x < containerRect[i].x + containerRect[i].w) &&
					(event.motion.y > containerRect[i].y) &&
					(event.motion.y < containerRect[i].y + containerRect[i].h)) {
						containerLetters[i] = 0;
						containerAscii[i] = 32;
						scoreArray[i] = -1;
					}
				}
			}
		}
		else if(event.type == SDL_MOUSEBUTTONUP) {
			for(int i = 0; i < 7; i++) {
				if((lettersX[letter1][letter2] > containerRect[i].x) &&
				(lettersX[letter1][letter2] < containerRect[i].x + containerRect[i].w) &&
				(lettersY[letter1][letter2]+lettersRect[0][0].h > containerRect[i].y) &&
				(lettersY[letter1][letter2]+lettersRect[0][0].h < containerRect[i].y + containerRect[i].h)) {
					lettersX[letter1][letter2] = containerRect[i].x;
					lettersY[letter1][letter2] = containerRect[i].y;
					containerLetters[i] = letters[letter1][letter2];
					containerAscii[i] = letter2+97;
					scoreArray[i] = letter2;
					if(sound) {
						Mix_VolumeChunk(click, 64);
						if(Mix_PlayChannel(-1, click, 0) == -1) {
							fprintf(stderr, "click sound failed!\n%s\n", Mix_GetError());
						}
					}
				}
				else if((lettersX[letter1][letter2] > containerRect[i].x) &&
				(lettersX[letter1][letter2] < containerRect[i].x + containerRect[i].w) &&
				(lettersY[letter1][letter2] > containerRect[i].y) &&
				(lettersY[letter1][letter2] < (containerRect[i].y + containerRect[i].h))) {
					lettersX[letter1][letter2] = containerRect[i].x;
					lettersY[letter1][letter2] = containerRect[i].y;
					containerLetters[i] = letters[letter1][letter2];
					containerAscii[i] = letter2+97;
					scoreArray[i] = letter2;
					if(sound) {
						Mix_VolumeChunk(click, 64);
						if(Mix_PlayChannel(-1, click, 0) == -1) {
							fprintf(stderr, "click sound failed!\n%s\n", Mix_GetError());
						}
					}
				}
			}
			letterDrag = 0;
		}
	}
}

void game_logic(void)
{
	if(SDL_GetTicks() - timeStart >= 1000) {
		randomFallSpot = rand() % 4;
		randomLetter = rand() % 98;
		int randomLetterSpot = letterDistribution[randomLetter];
		lettersY[randomFallSpot][randomLetterSpot]++;
		timeStart = SDL_GetTicks();
	}

	if(letterDrag) {
		drag_letter(letter1, letter2);
	}

	if(addScore) {
		static int called = 0;
		if(!called) {
			addTransition = SDL_GetTicks();
			called += 1;
		}
		if(SDL_GetTicks() - addTransition <= 250) {
			addTransitionY -= 0.25;
		}
		if(SDL_GetTicks() - addTransition >= 250) {
			addTransitionY = 550;
			called -= 1;
			addScore = 0;
			theScore = 0;
			if(addScoreFail == 1) {
				addScoreFail = 0;
			}
			if(addScoreWin == 1) {
				addScoreWin = 0;
			}
			addTransition = SDL_GetTicks();
		}
	}

	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 26; j++) {
			if(lettersY[i][j] != 45) {
				lettersRect[i][j].x = lettersX[i][j];
				lettersRect[i][j].y = lettersY[i][j];
				lettersY[i][j]++;
			}
			if((lettersY[i][j] >= (GRASS_Y+lettersRect[i][j].h+20)) && letters[i][j] != letters[letter1][letter2]) {
				lettersY[i][j] = 45;
				lettersX[i][j] = (i*75)+45;
				lettersRect[i][j].x = lettersX[i][j];
				lettersRect[i][j].y = lettersY[i][j];
				lettersRect[i][j].w = letters[i][j]->clip_rect.w;
				lettersRect[i][j].h = letters[i][j]->clip_rect.h;
			}
		}
	}
}

void game_render(void)
{	
	render_image(0,0,background,screen);

	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 26; j++) {
			if(lettersY[i][j] <= GRASS_Y && lettersY[i][j] != 1) {
				render_image(lettersX[i][j], lettersY[i][j], letters[i][j], screen);
			}
		}
	}

	render_image(cloudPos1.x, cloudPos1.y, cloud1, screen);
	render_image(cloudPos2.x, cloudPos2.y, cloud2, screen);
	render_image(cloudPos3.x, cloudPos3.y, cloud3, screen);

	render_image(0,GRASS_Y,grass,screen);

	// Containers
	for(int i = 0; i < 7; i++) {
		if(containerLetters[i] != NULL) {
			render_image(containerX[i]+2, containerY+2, containerLetters[i], screen);
		}
		render_image(containerX[i], containerY, container[i], screen);
	}
	// Submit check
	render_image(submitX, submitY, submit, screen);

	// Pause button
	render_image(pauseX, pauseY, pause, screen);

	// Score yellow: 254,210,6
	SDL_Color scoreColor = {0,0,0};
	char finalScoreString[64];
	sprintf(finalScoreString, "SCORE: %d", finalScore);
	if(oldScore != finalScore || oldScore == 0) {
		score = render_font(scoreFont, finalScoreString, scoreColor);
		oldScore = finalScore;
	}
	render_image(32, ((score->clip_rect.h - pause->clip_rect.h)/2), score, screen);
	if(oldScore != finalScore || oldScore == 0) {
		SDL_FreeSurface(score);
		score = NULL;
		oldScore = finalScore;
	}

	if(letterDrag) { 
		if(lettersY[letter1][letter2] >= (GRASS_Y - lettersRect[0][0].h) && lettersY[letter1][letter2] != 1) {
			render_image(lettersX[letter1][letter2], lettersY[letter1][letter2], letters[letter1][letter2], screen);
		}
	}

	if(addScore) {
		if(addScoreWin) {
			SDL_Color scoreColor = {0,0,0};
			char tmpScore[16];
			sprintf(tmpScore, "+%d", theScore);
			scorePopup = render_font(scoreFontPopup, tmpScore, scoreColor);
			render_image(310, addTransitionY, scorePopup, screen);
			SDL_FreeSurface(scorePopup);
			scorePopup = NULL;
		}
		else if(addScoreFail) {
			SDL_Color failColor = {255,0,0};
			notWord = render_font(notWordFont, "X", failColor);
			render_image(320, addTransitionY, notWord, screen);
			SDL_FreeSurface(notWord);
			notWord = NULL;
		}
	}
	
	if(SDL_Flip(screen) != 0) {
		fprintf(stderr, "screen update failed\n");
	}
}

void drag_letter(int letter1, int letter2)
{
	// There are serious issues with this code right now
	// but it is good enough for now. For the majority of the
	// time the user isn't going to be dragging the letter
	// off of the screen.

	if(event.motion.x >= 360 - lettersRect[0][0].w) {
		event.motion.x = 360 - lettersRect[0][0].w;
	}
	else if(event.motion.y >= 640 - lettersRect[0][0].h) {
		event.motion.y = 640 - lettersRect[0][0].h;
	}

	lettersX[letter1][letter2] = event.motion.x;
	lettersY[letter1][letter2] = event.motion.y;

	lettersRect[letter1][letter2].x = event.motion.x;
	lettersRect[letter1][letter2].y = event.motion.y;
}

// Sanitize looks to see if there is a space in the word sent to it,
// and/or takes off the spaces at the end of the word.

void sanitize(char *word)
{
	for(int i = 0; i < strlen(word); i++) {
		if(word[i] == ' ') {
			word[i] = '\0';
		}
	}
}

int isword(char *word)
{
	int isWord = -1;
	// make sure they didn't just enter 1 letter, because technically I is a
	// word, but I'm not going to allow it here.
	if(strlen(word) < 2) {
		return 0;
	}
	// cast to (const char *) should be fine here
	if(dict_search(dictionary, (const char *)word) != NULL) {
		isWord = 1;
	}
	else {
		isWord = 0;
	}
	return isWord;
}

char *strdup(const char *s)
{
	char *d = malloc(strlen(s)+1);
	if(d == NULL) {
		return NULL;
	}
	strcpy(d,s);

	return d;
}
