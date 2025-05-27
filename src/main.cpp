/**************************************
 * RT Embedded Challenge Spring 2024 
 * Project: TrmrGlv v1.0
 * Team : Kyle Six, Kahlia Gronthos, Josh Manogaran
 * 
 * 
 * Dependency Graph
 * |-- BSP_DISCO_F429ZI @ 0.0.0+sha.53d9067a4feb / Note that BSP_DISCO_F429ZI drivers were modified by me to replace "wait_ms()" with "thread_sleep_for()"
 * |-- LCD_DISCO_F429ZI @ 0.0.0+sha.dc55a068bc1a
 * |-- TS_DISCO_F429ZI @ 0.0.0+sha.4f8b6df8e235
 * |-- GUI
 * |-- Gyroscope
 * |-- MovingAverage
 * |-- cmsis-dsp
 * 
 * 
 * Code Structure MAIN(): 
 * Following from the "GUI" class, touch events and LCD drawing are bundled with state id's
 *  in a single layer state machine. There are the following states:
 *      | Main Menu 
 *          | Tremor 
 *          | Freq
 *          | Info
 *  These may be started/exited by the user via touch. (Note that fft and gyro sampling
 *  currently block touch IO in this version! Threading to come in future)
 * 
 * Usage:
 *  To identify Parkinson's tremors, the user is meant to put on the hand medical brace and strap in the board.
 *  It should be such that the text is right side up (with arms resting at side) and the screen is visible.
 *  After supplying USB power, the user may select "Tremor" on the LCD to begin capturing gyroscope data along
 *  the X axis as the board moves with the user's hand. After the sample buffer is full, a fft is used to determine
 *  the frequency of hand rolling movements. After processing the signal, the status of Parkinson's tremors is 
 *  presented to the user via text and color on the LCD.
 * 
 **************************************/

// C++ Libraries
#include <array>
#include <vector>
#include <string>
#include <math.h>
// Mbed Drivers
#include "mbed.h"
#include "LCD_DISCO_F429ZI.h" // Note that BSP_DISCO_F429ZI drivers were modified by me to replace "wait_ms()" with "thread_sleep_for()"
#include "TS_DISCO_F429ZI.h"
// Project Code
#include "Gyroscope.h"
#include "MovingAverage.h"
#include "GUI.h"

// CMSIS DSP Library
#include "arm_math.h"
#include "arm_const_structs.h"

/******************************
 * GLOBALS
*******************************/
// Store velocity data
std::array<float, 3> velocity_xyz; 

// Create moving averages
MovingAverage<float, 3> moving_avg_freq;

// FFT
arm_cfft_instance_f32 fft;

// 256 samples X 30ms intervals = 7.68 seconds of window
#define FFT_SIZE 256
#define SAMPLING_FREQ 30 // in ms. This gives us 33.3 samples / sec
float32_t fft_input[FFT_SIZE * 2] = {0};
float32_t fft_output[FFT_SIZE] = {0};
uint32_t ifftFlag = 0;
uint32_t doBitReverse = 1;
float fft_maxValue;
uint32_t fft_maxIndex;


/* Create and initilialize GUI */
GUI gui("TrmrGlv 1.0");
extern uint8_t boot_image_glove_map[240*256];

/* Define States along with UI */
enum STATES {
    TREMOR_DETECTION,
    FREQ_VIEW,
    INFO
};
/************************************
 * TREMOR DETECTION STATE
 * Identifies parkinsonian tremors via fourier transform on gyroscope data
 * 
 * This state periodically samples gyroscope values in a window, performs complex fourier transform,
 * calculates maximum energy bin, averages the frequency and displays results on an LCD screen.
 *
 * @returns None
 */
Region* TREMOR_BUTTON = new EllipseRegion(120, 100, 64, 32, LCD_COLOR_DARKGREEN, LCD_COLOR_DARKYELLOW, 4, LCD_COLOR_DARKYELLOW, "Tremor");
std::vector<Region*> TREMOR_UI = {
    new RectRegion(0, 40, 240, 280, LCD_COLOR_BLACK, LCD_COLOR_BLACK, 4, LCD_COLOR_BLACK, ""),
};
/* fillFFTWindow(void)
 *  Collects data from the gyroscope at specific frequency to fill the fft input buffer 
 * @returns None
 */
void fillFFTWindow(void) {
    gui.lcd.SetBackColor(LCD_COLOR_BLACK);
    gui.lcd.SetTextColor(LCD_COLOR_GREEN);
    gui.lcd.DisplayStringAt(0, 150, (uint8_t *) "SAMPLING...", CENTER_MODE);
    //Create gyroscope instance
    Gyroscope gyro;
    // Fill sample with values
    for(int i = 0; i < FFT_SIZE; i++) {
        velocity_xyz = gyro.sequential_read();
        fft_input[i*2] = velocity_xyz[0];
        fft_input[i * 2 + 1] = 0;
        // printf(">x:%f\n", velocity_xyz[0]);
        thread_sleep_for(SAMPLING_FREQ);
    }
    gyro.endSPI();
    //memcpy(fft_input, fft_window, FFT_SIZE * 2);
    gui.lcd.DisplayStringAt(0, 150, (uint8_t *) "           ", CENTER_MODE);
}
/* fourierTransform(void)
 *      Performs a fourier transform on the input buffer, calculates maximum energy bin, and returns the frequency
 * @returns float freqeuncy of signal
 */
float fourierTransform(void) {
    /* Process the data through the CFFT/CIFFT module */
    //printf("Processing Data\n");
    arm_cfft_f32(&fft, fft_input, ifftFlag, doBitReverse);

    /* Process the data through the Complex Magnitude Module for
    calculating the magnitude at each bin */
    //printf("Computing Complex Magnitude\n");
    arm_cmplx_mag_f32(fft_input, fft_output, FFT_SIZE);

    /* Calculates maxValue and returns corresponding BIN value */
    //printf("Getting Maximum energy bin\n");
    arm_max_f32(fft_output, FFT_SIZE, &fft_maxValue, &fft_maxIndex);
    //printf("Max Val: %f\n", fft_maxValue);
    printf("Max Index: %lu\n", fft_maxIndex);

    /* Calculate frequency of maximum energy bin -> based on index in sample and sample rate */
    float maxFreqComponent = static_cast<float>(fft_maxIndex) * (static_cast<float>(SAMPLING_FREQ) / FFT_SIZE);
    printf("Frequency:%f\n", maxFreqComponent);
    return maxFreqComponent;
}
/************************************
 * FREQUENCY VIEW STATE
 * Displays raw frequency spectrum from a fourier transform on gyroscope data
 * 
 * This state periodically samples gyroscope values in a window, performs complex fourier transform,
 * calculates maximum energy bin, and displays a graph on an LCD screen.
 *
 * @returns None
 */
Region* FREQ_BUTTON = new EllipseRegion(120, 180, 64, 32, LCD_COLOR_DARKGREEN, LCD_COLOR_DARKYELLOW, 4, LCD_COLOR_DARKYELLOW, " Freq");
std::vector<Region*> FREQ_UI = {
    new RectRegion(0, 40, 240, 280, LCD_COLOR_BLACK, LCD_COLOR_BLACK, 4, LCD_COLOR_BLACK, ""),
};
/************************************
 * INFO STATE
 * Shows device info and useage
 * 
 * @returns None
 */
Region* INFO_BUTTON = new EllipseRegion(120, 260, 64, 32, LCD_COLOR_DARKGREEN, LCD_COLOR_DARKYELLOW, 4, LCD_COLOR_DARKYELLOW, " Info");
std::vector<Region*> INFO_UI = {
    new RectRegion(0, 40, 240, 280, LCD_COLOR_BLACK, LCD_COLOR_BLACK, 4, LCD_COLOR_BLACK, ""),
};

/*****************************************
 * MAIN
******************************************/
/**
 * Main function that sets up and runs the GUI application. Blocks indefinitely.
 *  
 * @returns 0 indicating successful execution.
 */
int main(void) {
    // Setup //
    /* Initialize GUI state machine*/
    gui.addState(TREMOR_DETECTION, TREMOR_BUTTON, TREMOR_UI);
    gui.addState(FREQ_VIEW, FREQ_BUTTON, FREQ_UI);
    gui.addState(INFO, INFO_BUTTON, INFO_UI);

    /* Initialize first FFT Sample with Gyro Data*/
    fillFFTWindow();

    /* Initialize CFFT module */
    printf("Initializing CFFT\n");
    arm_status status;
    status = arm_cfft_init_256_f32(&fft);

    gui.init();
    // Execution //
    while (true) {
        
        if(gui.getTouchEvent())
            gui.update();

        switch(gui.state) {
            case TREMOR_DETECTION: ////////////////////////////////////////////////////////////////////////////////////////////////
            {
                if(gui.getTouchEvent())
                    gui.update();

                // Perform FFT
                float freq = fourierTransform();
                
                // Apply moving average 
                moving_avg_freq.update(freq);

                // Draw text with freq
                char freq_str[8];
                sprintf(freq_str, "%4.2f", moving_avg_freq.getAverage());
                gui.lcd.SetBackColor(gui.background_color);
                gui.lcd.SetTextColor(LCD_COLOR_WHITE);
                gui.lcd.DisplayStringAt(0, 80, (uint8_t *) " Tremor Range:", LEFT_MODE);
                //gui.lcd.DisplayStringAt(0, 110, (uint8_t *) "        ", CENTER_MODE);
                gui.lcd.DisplayStringAt(10, 110, (uint8_t *) "[3.0, 6.0]", LEFT_MODE);
                gui.lcd.DisplayStringAt(200, 110, (uint8_t *) "hz", LEFT_MODE);
                
                // Classify frequency into intensities
                if(moving_avg_freq.getAverage() >= 5.0f && moving_avg_freq.getAverage() <= 6.0f){
                    RectRegion r(40, 200, 160, 100, LCD_COLOR_DARKRED, LCD_COLOR_BLACK, 4, LCD_COLOR_ORANGE, "");
                    r.draw(&(gui.lcd));
                    gui.lcd.SetBackColor(LCD_COLOR_DARKRED);
                    gui.lcd.SetTextColor(LCD_COLOR_WHITE);
                    gui.lcd.DisplayStringAt(56, 260, (uint8_t *) "HIGH", LEFT_MODE);
                } 
                else if(moving_avg_freq.getAverage() >= 4.0f && moving_avg_freq.getAverage() < 5.0f){
                    RectRegion r(40, 200, 160, 100, LCD_COLOR_ORANGE, LCD_COLOR_BLACK, 4, LCD_COLOR_DARKYELLOW, "");
                    r.draw(&(gui.lcd));
                    gui.lcd.SetBackColor(LCD_COLOR_ORANGE);
                    gui.lcd.SetTextColor(LCD_COLOR_WHITE);
                    gui.lcd.DisplayStringAt(56, 260, (uint8_t *) "MID", LEFT_MODE);
                } 
                else if(moving_avg_freq.getAverage() >= 3.0f && moving_avg_freq.getAverage() < 4.0f){
                    RectRegion r(40, 200, 160, 100, LCD_COLOR_YELLOW, LCD_COLOR_BLACK, 4, LCD_COLOR_DARKYELLOW, "");
                    r.draw(&(gui.lcd));
                    gui.lcd.SetBackColor(LCD_COLOR_YELLOW);
                    gui.lcd.SetTextColor(LCD_COLOR_BLACK);
                    gui.lcd.DisplayStringAt(56, 260, (uint8_t *) "LOW", LEFT_MODE);
                } 
                else {
                    RectRegion r(40, 200, 160, 100, LCD_COLOR_DARKGREEN, LCD_COLOR_BLACK, 4, LCD_COLOR_DARKYELLOW, "");
                    r.draw(&(gui.lcd));
                    gui.lcd.SetBackColor(LCD_COLOR_DARKGREEN);
                    gui.lcd.SetTextColor(LCD_COLOR_WHITE);
                    gui.lcd.DisplayStringAt(56, 260, (uint8_t *) "N/A", LEFT_MODE);
                }
                gui.lcd.DisplayStringAt(56, 210, (uint8_t *) "Status: ", LEFT_MODE);
                gui.lcd.DisplayStringAt(0, 234, (uint8_t *) "        ", CENTER_MODE);
                gui.lcd.DisplayStringAt(56, 234, (uint8_t *)freq_str, LEFT_MODE);
                gui.lcd.DisplayStringAt(140, 234, (uint8_t *) "hz", LEFT_MODE);
                gui.lcd.SetTextColor(LCD_COLOR_WHITE);
                
                // Get new gyroscope Sample
                fillFFTWindow();

            } break;
            case FREQ_VIEW: ////////////////////////////////////////////////////////////////////////////////////////////////
            {
                // Do touchscreen input
                if(gui.getTouchEvent())
                    gui.update();
                
                // Perform FFT
                float freq = fourierTransform();

                // Draw text with freq
                char freq_str[8];
                sprintf(freq_str, "%4.2f", freq);
                gui.lcd.SetBackColor(gui.background_color);
                gui.lcd.SetTextColor(LCD_COLOR_WHITE);
                gui.lcd.DisplayStringAt(40, 80, (uint8_t *) "Freq:", LEFT_MODE);
                gui.lcd.DisplayStringAt(0, 110, (uint8_t *) "        ", CENTER_MODE);
                gui.lcd.DisplayStringAt(40, 110, (uint8_t *) freq_str, LEFT_MODE);
                gui.lcd.DisplayStringAt(140, 110, (uint8_t *) "hz", LEFT_MODE);
                
                // Starting point for Graph
                int y_max = 100;
                int y_coord = 300;
                int x_coord = 56;
                // Draw Graph
                for (uint32_t i = 0; i < FFT_SIZE/2; i++) {
                    int magnitude = y_max * fft_output[i] / fft_maxValue;
                    if (magnitude > y_max) {
                        magnitude = y_max;
                    } else if (magnitude < 0) {
                        magnitude = 0;
                    }
                    
                    // Clear previous line
                    gui.lcd.SetTextColor(gui.background_color);
                    gui.lcd.DrawLine(i+x_coord, y_coord, i+x_coord, y_coord - y_max);
                    
                    // Draw new line
                    if (fft_maxIndex == i) {
                        gui.lcd.SetTextColor(LCD_COLOR_CYAN);
                        gui.lcd.DrawLine(i+x_coord, y_coord, i+x_coord, y_coord - magnitude);
                    } else {
                        gui.lcd.SetTextColor(LCD_COLOR_DARKGREEN);
                        gui.lcd.DrawLine(i+x_coord, y_coord, i+x_coord, y_coord - magnitude);
                    }
                }

                // Sample gyroscope data
                fillFFTWindow();
                
            } break;
            case INFO: ////////////////////////////////////////////////////////////////////////////////////////////////
            {
                // Do touchscreen event
                if(gui.getTouchEvent())
                    gui.update();
                // Show device info
                BSP_LCD_SetFont(&Font16);
                gui.lcd.SetBackColor(gui.background_color);
                gui.lcd.DisplayStringAt(10, 60, (uint8_t *) "This device collects", LEFT_MODE);
                gui.lcd.DisplayStringAt(10, 80, (uint8_t *) "gyroscope data and ", LEFT_MODE);
                gui.lcd.DisplayStringAt(10, 100, (uint8_t *) "calculates the freq. ", LEFT_MODE);
                gui.lcd.DisplayStringAt(10, 120, (uint8_t *) "of oscillation to ", LEFT_MODE);
                gui.lcd.DisplayStringAt(10, 140, (uint8_t *) "detect Parkinsonian ", LEFT_MODE);
                gui.lcd.DisplayStringAt(10, 160, (uint8_t *) "Tremors. ", LEFT_MODE);
                gui.lcd.SetBackColor(LCD_COLOR_DARKGRAY);
                gui.lcd.DisplayStringAt(10, 180, (uint8_t *) "Tremor Mode:", LEFT_MODE);
                gui.lcd.SetBackColor(gui.background_color);
                gui.lcd.DisplayStringAt(10, 200, (uint8_t *) "- Processes signal", LEFT_MODE);
                gui.lcd.DisplayStringAt(10, 220, (uint8_t *) "- Identifies resting", LEFT_MODE);
                gui.lcd.DisplayStringAt(10, 240, (uint8_t *) "tremor + intensity", LEFT_MODE);
                gui.lcd.SetBackColor(LCD_COLOR_DARKGRAY);
                gui.lcd.DisplayStringAt(10, 260, (uint8_t *) "Frequency Mode:", LEFT_MODE);
                gui.lcd.SetBackColor(gui.background_color);
                gui.lcd.DisplayStringAt(10, 280, (uint8_t *) "- Outputs raw ", LEFT_MODE);
                gui.lcd.DisplayStringAt(10, 300, (uint8_t *) "frequency spectrum", LEFT_MODE);
                BSP_LCD_SetFont(&Font24);
            } break; 
                
        }
    }
    return 0;
}
