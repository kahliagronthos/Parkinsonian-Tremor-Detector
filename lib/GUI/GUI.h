// C++
#include <functional>
#include <vector>
#include <string>
// Mbed
#include "mbed.h"
#include "LCD_DISCO_F429ZI.h"
#include "TS_DISCO_F429ZI.h"
// Project
#include "Region.h"

#define SCREEN_XSIZE 240
#define SCREEN_YSIZE 320

/** 
 * @brief Represents an action that can be performed in a GUI, along with UI
 * 
 */
class State {
private:
    int state_id = -1;
    Region* button;
    std::vector<Region*> regions;

public:
    State(int _state_id, Region* _button, std::vector<Region*> _regions) : state_id(_state_id), button(_button), regions(_regions) {}
    friend class GUI;

    // Overloading < operator 
    bool operator<(const State& obj) const
    { 
        return state_id < obj.state_id; 
    } 
};

/**
 * @brief Represents a graphical user interface (GUI): bundling display drawing, touch events, and code execution
 * 
 */
class GUI {
public:
    LCD_DISCO_F429ZI lcd;
    TS_DISCO_F429ZI ts; 

    TS_StateTypeDef TS_State;
    uint16_t touch_x = 0;
    uint16_t touch_y = 0;

    uint32_t background_color = LCD_COLOR_BLACK;
    
    int state = -1;
    std::vector<State> states;
    
    string title;
    EllipseRegion backButton;

    /**
     * Constructs a GUI object with a specified title.
     *
     * @param _title The title of the GUI.
     *
     * @returns None
     */
    GUI(string _title) :  title(_title), backButton(54, 30, 44, 24, LCD_COLOR_DARKGREEN, LCD_COLOR_DARKYELLOW, 4, LCD_COLOR_DARKYELLOW, "BACK") {
        // Initialize LCD
        lcd.Init();
        lcd.Clear(background_color);
        BSP_LCD_SetFont(&Font24);
        // Initialize Touchscreen
        printf("%d\n", ts.Init(SCREEN_XSIZE, SCREEN_YSIZE));
    }

    /**
     * Draws the initial UI on the LCD screen.
     *
     * @returns None
     */
    void init() {
        update();
    }

    /**
     * Displays a bitmap image on an LCD screen at the specified coordinates.
     *
     * @param X The x-coordinate of the top-left corner of the bitmap.
     * @param Y The y-coordinate of the top-left corner of the bitmap.
     * @param pBmp A pointer to the bitmap data.
     * @param wait_time The time to wait after displaying the bitmap.
     *
     * @returns None
     */
    void boot(uint32_t X, uint32_t Y, uint8_t *pBmp, uint16_t wait_time) {
        lcd.DrawBitmap(X, Y, pBmp);
        thread_sleep_for(wait_time);
    }

    /**
     * Draws the title on the LCD screen.
     *
     * @returns None
     */
    void drawTitle() {
        lcd.SetBackColor(background_color);
        lcd.SetTextColor(LCD_COLOR_WHITE);
        lcd.DisplayStringAt(0, 20, (uint8_t *)title.c_str(), CENTER_MODE);
    }

    /**
     * Adds a menu button and its corresponding UI regions to the menu.
     *
     * @param button A pointer to the region representing the menu option.
     * @param action The action to be performed when the menu option is selected.
     *
     * @returns None
     */
    void addState(int state_id, Region* button, std::vector<Region*> ui) {
        states.push_back(State(state_id, button, ui));
        sort(states.begin(), states.end());
    }

    /**
     * Polls the touchscreen for the last touch coordinates and determines
     * if this is a new touch event.
     *
     * @returns bool
     */
    bool getTouchEvent(void) {
        // Get touchscreen state
        ts.GetState(&TS_State);

        // If coordinates are different, this is a new touch event
        if (TS_State.X != touch_x && (SCREEN_YSIZE - TS_State.Y) != touch_y) {
            touch_x = TS_State.X;
            touch_y = SCREEN_YSIZE - TS_State.Y;
            return true;
        }
        else
            return false;
    }

    /**
     * Runs the main loop of the GUI application.
     *
     * @returns None
     */
    void update(void) {
        // On MAIN menu screen
        printf("state: %d\n", state);
        if (state == -1) {
            // Draw Main Title
            drawTitle();

            // Draw all menu buttons
            for(auto it = states.begin(); it != states.end(); it++) {
                it->button->draw(&lcd);

                // If touching a menu option, start the corresponding action
                if (TS_State.TouchDetected && it->button->isWithin(touch_x, touch_y)) {
                    // Show Touch
                    it->button->fill(&lcd);
                    thread_sleep_for(500);
                    lcd.Clear(background_color);
                    // Perform action setup callback function
                    lcd.SetTextColor(background_color);
                    lcd.DrawRect(0,0,SCREEN_XSIZE, SCREEN_YSIZE);
                    touch_x = 0;
                    touch_y = 0;
                    state = it->state_id;
                }
            }
        }
        else { // Inside a STATE
            // Draw regions for this state
            
            for(auto it = states[state].regions.begin(); it != states[state].regions.end(); it++) {
                (*it)->draw(&lcd);
                // // If touching a region, highlight it
                // if (TS_State.TouchDetected && (*it)->isWithin(touch_x, touch_y)) {
                //     (*it)->fill(&lcd);
                // }
            }
            // Draw back button 
            backButton.draw(&lcd);

            // Check if we can go back to menu state
            if (TS_State.TouchDetected && backButton.isWithin(touch_x, touch_y)) {
                // Show touch
                backButton.fill(&lcd);
                thread_sleep_for(500);
                lcd.Clear(background_color);
                // Set State back to Main Menu
                lcd.SetTextColor(background_color);
                lcd.DrawRect(0,0,SCREEN_XSIZE, SCREEN_YSIZE);
                touch_x = 0;
                touch_y = 0;
                state = -1;
            }
        }
    }
    
};