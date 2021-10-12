#include "hrtf.h"
#include "stdio.h"
#include <stdlib.h>
#include <string.h>
//#include "sndfile.h"
#include <math.h>
//#include "ncurses.h"
#include <cstdio>
#include "stdbool.h"


/*
	Read in HRTF Data from sourced Dataset
	Format:
		14 Elevations from -40 to 90deg, increments of 10deg
		Varying azimuthal increments as follows:
		6.43, 6.00, 5.00, 5.00, 5.00, 5.00, 5.00, 6.00, 6.42, 8.00, 10.00, 15.00, 30.00, N/A
		Folder only contains Azimuthal recordings for 0 <= azi <= 180deg
		For 180 < azi < 360, swap LR channels to get corresponding HRTF
*/

//int readHRTF()
//{
//	SNDFILE *sndfile;
//	SF_INFO sfinfo;
//	int hrtfIdx = 0, numSamples = 0;
//
//	char ifile[PATH_LEN];
//	/* for each height, iterate through azimuth */
//	for (int i=0; i < ELE_STEPS; i++)
//	{
//		azi_start_idx[i] = hrtfIdx;
//		printf("azi_start_idx[%i] = %d\n", i, hrtfIdx);
//
//		/* number of azi steps changes depending on height */
//		for (float j = 0; j <= 180; j += azi_increments[i])
//		{
//			/* read in sndfile into hrtfArr*/
//
//			/* copy filename */
//			/* sprintf( char* str, const char* format, args) */
//			sprintf(ifile, "%s/elev%d/H%de%03.fa.wav", DIRECTORY, elevations[i], elevations[i], round(j));
//
//			/* zero libsndfile structure */
//    		memset(&sfinfo, 0, sizeof(sfinfo));
//
//			/* open file */
//			if ( (sndfile = sf_open(ifile, SFM_READ, &sfinfo)) == NULL)
//			{
//				fprintf(stderr, "%s %s\n", "Error: Unable to open HRTF file ", ifile);
//				return -1;
//			}
//
//			/* check validity with sfinfo params: channels, samplerate */ 
//			if (sfinfo.channels != HRTF_CHAN)
//			{
//				fprintf(stderr, "%s %s\n", "Error: incorrect channel count in file: ", ifile);
//				return -1;
//			}
//
//			if (sfinfo.samplerate != SAMPLE_RATE)
//			{
//				fprintf(stderr, "%s %s\n", "Error: incorrect sample rate in file: ", ifile);
//				return -1;
//			}
//
//			numSamples = sfinfo.channels * (int)sfinfo.frames;
//
//			/* print */
//			printf("File %s: Chan: %d Frames: %d\n", ifile, sfinfo.channels, (int)sfinfo.frames);
//
//			/* copy to hrtfArr */
////			if ( (count = sf_read_float(sndfile, hrtfArr[hrtfIdx], numSamples)) != numSamples){
////				fprintf(stderr, "%s\n", "Error: Unable to read in HRTF signal");
////				fprintf(stderr, "numSamples: %d count: %d\n", numSamples, count);
////				return -1;
////			}
//			/* close file*/
//			sf_close(sndfile);
//
//			hrtfIdx++;
//		}
//	}
//	return 0;
//}

HRTF::HRTF()
{
    
}
HRTF::~HRTF()
{
    
}

int HRTF::readHRTF()
{
    return 0;
}

int HRTF::readHRTFJUCE(juce::AudioFormatManager &formatManager)
{
    formatManager.registerBasicFormats();
    
    int hrtfIdx = 0;
    char ifile[PATH_LEN];
    
    /* for each height, iterate through azimuth */
    for (int i=0; i < ELE_STEPS; i++)
    {
        azi_start_idx[i] = hrtfIdx;

        /* number of azi steps changes depending on height */
        for (float j = 0; j <= 180; j += azi_increments[i])
        {
            hrtfArray.add(new AudioBuffer<float>(HRTF_CHAN, HRTF_LEN));

            /* Get Filepath */
            /* sprintf( char* str, const char* format, args) */
            sprintf(ifile, "%s/elev%d/H%de%03.fa.wav", DIRECTORY, elevations[i], elevations[i], round(j));
            
            String filepath;
            
            //Juce, the little bastard, has different number of directories between plugins + standalone
            //this only took me ~6 hours to figure out what the issue was
            if (JUCEApplicationBase::isStandaloneApp())
            {
                filepath = File::getCurrentWorkingDirectory().getParentDirectory().getParentDirectory()
                    .getParentDirectory().getParentDirectory().getFullPathName();
            }
            else
            {
                std::cout << "Not standalone!" << std::endl;
                filepath = "/Users/dbraunst/Documents/JUCE_Proj/_NYU_C_Programming/Binauralizer";
                std::cout << "Filepath: " << filepath << std::endl;
            }
            
            filepath = filepath + "/Source/" + (std::string)ifile;
            
            File file (filepath);
            
            /* print filepath */
//            std::cout << "Filepath: " << filepath << "\nNumSamp: " << hrtfArray[hrtfIdx]->getNumSamples()
//            << ", NumChan: " << hrtfArray[hrtfIdx]->getNumChannels() << std::endl;
//            std::cout << file.getFullPathName() << std::endl;
            
            /* create reader for file */
            AudioFormatReader* reader = formatManager.createReaderFor(file);
            
            jassert(reader != nullptr);
            jassert(hrtfArray[i] != nullptr);
            
            /* open file */
            reader->read(hrtfArray[hrtfIdx], 0, (int)reader->lengthInSamples, 0, true, true);
            
            delete reader;
            
            hrtfIdx++;
        }
    }
    return 0;
} 

/*
	given the azimuth and elevation selection, determine the closest
	available azimuth index
		float _azi - azimuth value, 0 <= azi < 360
		float _ele - elevation value, -40 <= ele <= 90
*/
int HRTF::selectHRTF(float _azi, float _ele)
{
	/* convert elevation param into index for azi_increments[] */
	int _eleIdx = _ele + 40; 
	if (_eleIdx != 0)
	{
		_eleIdx /= 10;
	}

	float increment = azi_increments[_eleIdx];
	int offset_count = 0;
	float azi_offset = 0.0f;

	/* save to swap LR channels in convolution later if needed */
	switch_LR = false;
	if (_azi < 0)
	{
		switch_LR = true;
		_azi = -_azi;
	}

	//mvprintw(4, 0, "Elevation index: %d, Azi increments: %.2f\n", _eleIdx, increment);

	/* get current azimuth index by comparing azimuth to n * azi_increments, 
	   when greater than or equal, grab that + previous, returning whichever is the closest estimation
	 */
	if(_azi == 0)
	{
		offset_count = 0;
	}
	else
	{
		while (azi_offset < _azi)
		{
			azi_offset += increment;
			offset_count++;
		}
	}

	/* check for whether n*increment or n-1*increment is closer to actual azimuth */
	if (abs((increment * offset_count) - _azi) > abs((increment * (offset_count - 1) - _azi)))
	{
		offset_count--;
	}

	/* return index of corresponding HRTF */
	return offset_count + azi_start_idx[_eleIdx];
}

/*
	given the index for the HRTF array from current azimuth + elevation, 
	convolve HRTF with input buffer to produce output buffer
	params: 
		float *x - pointer to input buffer
		float *y - pointer to output buffer
		_hrtf_idx - index from HRTF array
	USES:
		switch_LR - swap left + right channels
 */

void HRTF::convolveHRTF(const float *xL, const float *xR, float *yL, float *yR,
int _hrtf_idx, int _framesPerBuff, float _gain)
{

    /*  for 1 <= k <= M-1 where M = len(x) + HRTF_LEN - 1
        y[n] += x(n-k)h(k)

        for n = 0->_frames_per_Buff
            for k = 0 -> HRTF_LEN
                for c = 0 -> numchan (2)
     */

    /* zero output buffer */
    for (int i=0; i<_framesPerBuff; i++)
    {
        yL[i] = 0.0;
        yR[i] = 0.0;
    }
    
    for (int n = 0; n < _framesPerBuff; n++)
    {
        auto* hrtfConvArrL = hrtfArray[_hrtf_idx]->getReadPointer(0);
        auto* hrtfConvArrR = hrtfArray[_hrtf_idx]->getReadPointer(1);
        
        if (!switch_LR)
        {
            for (int k = 0; k < HRTF_LEN; k++)
            {
                yL[n] += xL[n - k] * hrtfConvArrL[k];
                yR[n] += xR[n - k] * hrtfConvArrR[k];
            }
        }
        else //swap LR channels if value from selectHRTF is true
        {
            for (int k = 0; k < HRTF_LEN; k++)
            {
                yL[n] += xL[n - k] * hrtfConvArrR[k];
                yR[n] += xR[n - k] * hrtfConvArrL[k];
            }
        }
        //multiply by gain parameter
        yL[n] *= juce::Decibels::decibelsToGain(_gain);
        yR[n] *= juce::Decibels::decibelsToGain(_gain);
    }
    return;
}
