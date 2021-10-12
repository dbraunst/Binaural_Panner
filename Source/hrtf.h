#pragma once

#define HRTF_CHAN 2
#define HRTF_LEN 128
#define HRTF_FS 44100
#define NUM_HRTF 366

#define ELE_STEPS 14
#define DIRECTORY "compact"
#define PATH_LEN 80
#define SAMPLE_RATE 44100

#include <JuceHeader.h>
#include <iostream>



//static AudioBuffer<float> *hrtfBuffers[NUM_HRTF];

static int elevations[14] = { -40, -30, -20, -10, 0, 10, 20, 30, 40, 50, 60, 70, 80, 90};
static float azi_increments[14] = {6.43, 6.00, 5.00, 5.00,   5.00,  5.00, 5.00,
                             6.00, 6.43, 8.00, 10.00, 15.00, 30.00, 181};
static int azi_start_idx[14];

static bool switch_LR; /* if azimuth is negative, denote switching LR channels */


class HRTF{
public:
    static float hrtfArr[NUM_HRTF][HRTF_LEN * HRTF_CHAN];
    OwnedArray<AudioBuffer<float>> hrtfArray;
    
    HRTF();
    ~HRTF();
    
    /*
        Read in HRTF Data from sourced Dataset
        Format:
            14 Elevations from -40 to 90deg, increments of 10deg
            Varying azimuthal increments as follows:
            6.43, 6.00, 5.00, 5.00, 5.00, 5.00, 5.00 6.00, 6.42, 8.00, 10.00, 15.00, 30.00, N/A
            Folder only contains Azimuthal recordings for 0 <= azi <= 180deg
            For 180 < azi < 360, swap LR channels to get corresponding HRTF
    */
    int readHRTF();
    int readHRTFJUCE(juce::AudioFormatManager &formatManager);

    /*
        given the azimuth and elevation selection, determine the closest
        available azimuth index:
            (e.g elevation ele, azimuth  azi)
            ele=10, azi =12.5, nearest corresponding index is ele=10, azi = 15
        and return the int value of that index in the total HRTF array
        params:
            float _azi - azimuth value, 0 <= azi < 360
            float _ele - elevation value, -40 <= ele <= 90
    */
    int selectHRTF(float _azi, float _ele);


    /*
        given the index for the HRTF array from current azimuth + elevation,
        convolve HRTF with input buffer to produce output buffer
        params:
            float *x - pointer to input buffer
            float *y - pointer to output buffer
            _hrtf_idx
    */
    void convolveHRTF(const float *xL, const float *xR, float *yL, float *yR,
                      int _hrtf_idx, int _framesPerBuff, float _gain);

   
        
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HRTF)
};
