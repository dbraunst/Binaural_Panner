//
//  main.cpp
//  Binauralizer
//
//  Created by Daniel Braunstein on 4/26/20.
//  Copyright © 2020 Daniel Braunstein. All rights reserved.
//

//#include "sndfile.h"
#include <iostream>
#include <stdio.h>
#include "string.h"
#include "hrtf.h"
#include "main.h"  
#include "stdbool.h"
//#include "portaudio.h"
//#include "paUtils.h"
//#include "ncurses.h"

struct Source_struct{
	SNDFILE *sndfile;	/* soundfile structure */

	int hrtf_index;		/* used to stored the index of the HRTF */
	bool swapChannels;	/* used when 180 < azimuth < 360 */

	float buf[FRAMES_PER_BUFFER + HRTF_LEN - 1]; /* for convolution */

	bool isMute;		/* muting for testing */
};
// typedef struct Source_struct Source;

static int paCallback(
	const void *inputBuffer,
	void *outputBuffer,
	unsigned long framesPerBuffer, 
	const PaStreamCallbackTimeInfo* timeInfo, 
	PaStreamCallbackFlags statusFlags,
	void *userData
	);

int main(int argc, char * argv[]) {

	Source source, *psource = &source;
	psource->isMute = true;
	psource->swapChannels = false;

    /* values to be changed in NCurses */
    int azimuth = 0, elevation = 0;

    /* libsndfile structures */
    SF_INFO sfinfo;

    /* portAudio structures */
    PaStream *stream;

    /* zero libsndfile structure */
    memset(&sfinfo, 0, sizeof(sfinfo));

    /* zero source data structure */
    psource -> hrtf_index = 0;
    for (int i = 0; i < FRAMES_PER_BUFFER + HRTF_LEN-1; i++)
    {
    	psource->buf[i] = 0.0f;
    }

    char *ifile;
    ifile = argv[1];
 
    if (readHRTF() != 0){
    	fprintf(stderr, "Error reading HRTF\n");
    	return -1;
    }

	if (argc != 2)
	{
		printf("%s\n", "Usage: main input_file.wav");
		return -1;
	}

	/* open input file */
	if ((psource->sndfile = sf_open(ifile, SFM_READ, &sfinfo)) == NULL)	{
		fprintf(stderr, "Error: cannot open input wav file %s\n", ifile);
		return -1;
	}

	/* print out wav file information */
	printf("Input audio file %s:\n", ifile);
	printf("    Frames: %8d, Channels: %d Samplerate: %d\n",
			(int)sfinfo.frames, sfinfo.channels, sfinfo.samplerate);

	/* Initialize ncurses 
     * to permit interactive character input 
     */
  	initscr(); /* Start curses mode */
  	cbreak();  /* Line buffering disabled */
  	noecho(); /* Uncomment this if you don't want to echo characters when typing */  

    printw("Q to quit, m to mute\n");		/* line 0 */				
    printw("\n");			 	/* line 1 */	
    printw("Use the arrow keys to move the sound source around:\n");
    //printw("Select: ");			/* line 3 */

	/* start up Port Audio */
	stream = startupPa(1, 2, sfinfo.samplerate, FRAMES_PER_BUFFER, paCallback, psource);

	/* j instantiated outside while loop for check on ncurses exit */
    wchar_t ch = '\0';

    /* enable arrow and function keys */
    keypad(stdscr, TRUE);

    refresh();
    /* unless user quits (q, Q), take input for which audio to stream */
    while (ch != 'q' && ch != 'Q') {
        mvprintw(3, 0, "Azimuth: %d degrees. Elevation: %d degrees.\n", azimuth, elevation);
        psource->hrtf_index  = selectHRTF((float)azimuth, (float)elevation);
        mvprintw(5, 0, "Muted: %d\n", psource->isMute);

    	ch = getch();

        switch (ch)
        {
            case KEY_UP:
                elevation += 10;
                break;
            case KEY_DOWN:
                elevation -= 10;
                break;
            case KEY_LEFT:
                azimuth -= 5;
                break;
            case KEY_RIGHT:
                azimuth += 5;
                break;
            /* for testing/debugging and letting me listen to music */
            case ('m'):
            	if (!psource->isMute){
            		psource->isMute = true;
            	}
            	else
            	{
            		psource->isMute = false;
            	}
            	break;
            default:
                mvprintw(3, 0, "Character pressed was = %c, please use the arrow keys.", ch);
                refresh();
                break;
        }

        /* keep azimuth bounded to (-180, 180] */
        if (azimuth > 180)
        {
            azimuth -= 360;
        } 
        else if (azimuth <= -180) 
        {
            azimuth += 360;
        }

        /* keep elevation bounded to [-40, 90] */
        if (elevation < -40)
        {
            elevation = -40;
        }
        else if (elevation > 90)
        {
            elevation = 90;
        }

    	refresh();
    }

    endwin();
  
	shutdownPa(stream);

	/* close WAV file */
	sf_close(psource->sndfile);

    std::cout << "Hello, World!\n";
    return 0;
}

/* This routine will be called by the PortAudio engine when audio is needed.
 * It will be called in the "real-time" thread, so don't do anything
 * in the routine that requires significant time or resources.
 */
static int paCallback(const void *inputBuffer, void *outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void *userData)
{
    Source *pb = (Source *)userData;\
    //float *input = (float *)inputBuffer;
    float *output = (float *)outputBuffer;
    //unsigned int num_samples = framesPerBuffer * NUM_CHAN;
    unsigned int count, hrtf_idx; 

    hrtf_idx = pb->hrtf_index;

    /* originally read to output, changed to data structure array */
    count = sf_readf_float(pb->sndfile, &pb->buf[HRTF_LEN-1], framesPerBuffer);

    if (count < framesPerBuffer)
    {
    	sf_seek(pb->sndfile, 0, SEEK_SET);
    	count = sf_readf_float(pb->sndfile, &pb->buf[count + HRTF_LEN-1], framesPerBuffer-count);
    }

    /* convolveHRTF(float *x, float *y, int _hrtf_idx, int framesPerBuffer) */
    convolveHRTF(&pb->buf[HRTF_LEN], output, pb->hrtf_index, framesPerBuffer);

	//if (pb->isMute){
    if (pb->isMute){
		for (int i = 0; i < framesPerBuffer * NUM_CHAN; i++)
		{
		
			output[i] *= 0.0f;
		}
	}

	float *px = pb->buf;
    for (int i=0; i<HRTF_LEN-1; i++) {
    	px[i] = px[framesPerBuffer+i];
    }

    return 0;
}
