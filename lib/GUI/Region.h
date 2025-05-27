// C++
#include <string>
// Mbed
#include <mbed.h>
#include "LCD_DISCO_F429ZI.h"
#include "TS_DISCO_F429ZI.h"

#define TEXT_LINE_HEIGHT 20

/**
 * @brief Base class representing a region to be drawn on a graphical user interface.
 * 
 */
class Region {
private:
    uint16_t position_x;
    uint16_t position_y;

    uint32_t color = LCD_COLOR_GRAY;
    uint32_t select_color = LCD_COLOR_DARKGRAY;

    uint16_t border_size = 1;
    uint32_t border_color = LCD_COLOR_DARKGRAY;

    uint32_t text_color = LCD_COLOR_WHITE;
    Text_AlignModeTypdef text_mode = LEFT_MODE;

    string text = "";
public:
    /** CONSTRUCTOR
     * Constructs a Region object with the specified parameters.
     *
     * @param xPos The x-coordinate position of the region.
     * @param yPos The y-coordinate position of the region.
     * @param _color The color of the region.
     * @param _select_color The color of the region when selected.
     * @param _border_size The size of the border of the region.
     * @param _border_color The color of the border of the region.
     * @param _text The text associated with the region.
     *
     * @returns none
     */
    Region(uint16_t xPos, uint16_t yPos, uint32_t _color, uint32_t _select_color, uint16_t _border_size, uint32_t _border_color, string _text) : position_x(xPos), position_y(yPos), color(_color), select_color(_select_color), border_size(_border_size), border_color(_border_color), text(_text) {}
    
    /**
     * Checks if a given point (x, y) is within a specific region. Child must implement!!!
     *
     * @param x The x-coordinate of the point.
     * @param y The y-coordinate of the point.
     *
     * @returns True if the point is within the region, False otherwise.
     */
    virtual bool isWithin(uint16_t x, uint16_t y) = 0;

    /**
     * Draws the object on the specified LCD display. Child must implement!!!
     *
     * @param lcd A pointer to the LCD_DISCO_F429ZI object.
     *
     * @returns None
     */
    virtual void draw(LCD_DISCO_F429ZI *lcd) = 0;

    /**
     * Fills the specified LCD display with content. Child must implement!!!
     *
     * @param lcd A pointer to the LCD_DISCO_F429ZI object representing the LCD display.
     *
     * @returns None
     */
    virtual void fill(LCD_DISCO_F429ZI *lcd) = 0;

    /**
     * Draws text on an LCD display.
     *
     * @param lcd A pointer to the LCD_DISCO_F429ZI object.
     *
     * @returns None
     */
    virtual void drawText(LCD_DISCO_F429ZI *lcd) {
        if (text != "") {
            lcd->SetBackColor(color);
            lcd->SetTextColor(text_color);
            lcd->DisplayStringAt(position_x, position_y, (uint8_t *)text.c_str(), text_mode);
        }
    }
    
    friend class GUI;
    friend class RectRegion;
    friend class EllipseRegion;
};

/**
 * @brief Implements a rectangular drawing region in a GUI, as parent class defines
 * 
 */
class RectRegion : public Region {
private: 
    uint16_t width;
    uint16_t height;
public:
    /** CONSTRUCTOR
     * Constructs a RectRegion object with the specified parameters.
     *
     * @param xPos The x-coordinate of the top-left corner of the rectangle.
     * @param yPos The y-coordinate of the top-left corner of the rectangle.
     * @param _width The width of the rectangle.
     * @param _height The height of the rectangle.
     * @param _color The color of the rectangle.
     * @param _select_color The color of the rectangle when selected.
     * @param _border_size The size of the border of the rectangle.
     * @param _border_color The color of the border of the rectangle.
     * @param _text The text associated with the rectangle.
     *
     * @returns None
     */
    RectRegion(uint16_t xPos, uint16_t yPos, uint16_t _width, uint16_t _height ,uint32_t _color, uint32_t _select_color, uint16_t _border_size, uint32_t _border_color, string _text) : Region(xPos, yPos, _color, _select_color, _border_size, _border_color, _text) {
        width = _width;
        height = _height;
    }
    
    /**
     * Checks if a given point (x, y) is within the boundaries of a rectangle.
     *
     * @param x The x-coordinate of the point.
     * @param y The y-coordinate of the point.
     *
     * @returns True if the point is within the rectangle, False otherwise.
     */
    bool isWithin(uint16_t x, uint16_t y) override {
        return ((x >= position_x && x <= position_x + width) && (y >= position_y && y <= position_y + height));
    }
    
    /**
     * Draws a rectangle region on the LCD display, with border and text.
     *
     * @param lcd A pointer to the LCD_DISCO_F429ZI object representing the LCD display.
     *
     * @returns None
     */
    void draw(LCD_DISCO_F429ZI *lcd) override {
        lcd->SetTextColor(border_color);
        lcd->DrawRect(position_x, position_y, width, height);
        lcd->FillRect(position_x, position_y, width, height);

        lcd->SetTextColor(color);
        lcd->DrawRect(position_x + border_size, position_y + border_size, width - (border_size * 2), height - (border_size * 2));
        lcd->FillRect(position_x + border_size, position_y + border_size, width - (border_size * 2), height - (border_size * 2));

        drawText(lcd);
    }
    
    /**
     * Fills a rectangle on the LCD display with a specified color and draws text on it.
     *
     * @param lcd A pointer to the LCD_DISCO_F429ZI object representing the LCD display.
     *
     * @returns None
     */
    void fill(LCD_DISCO_F429ZI* lcd) override {
        lcd->SetTextColor(select_color);
        lcd->FillRect(position_x + border_size, position_y + border_size, width - (border_size * 2), height - (border_size * 2));

        drawText(lcd);
    }

    /**
     * Draws text on the LCD display using the dimensions of the region.
     *
     * @param lcd A pointer to the LCD_DISCO_F429ZI object representing the LCD display.
     *
     * @returns None
     */
    void drawText(LCD_DISCO_F429ZI *lcd) override {
        if (text != "") {
            lcd->SetBackColor(color);
            lcd->SetTextColor(text_color);
            lcd->DisplayStringAt(position_x + border_size, position_y + (height / 2) - (TEXT_LINE_HEIGHT / 2), (uint8_t *)text.c_str(), text_mode);
        }
    }

    friend class GUI;
};

/**
 * @brief Implements an elliptical drawing region in a GUI, as parent class defines
 * 
 */
class EllipseRegion : public Region {
private: 
    uint16_t radius_x;
    uint16_t radius_y;
public:
    /** CONSTRUCTOR
     * Constructs an EllipseRegion object with the specified parameters.
     *
     * @param xPos The x-coordinate of the center of the ellipse.
     * @param yPos The y-coordinate of the center of the ellipse.
     * @param xRad The radius of the ellipse along the x-axis.
     * @param yRad The radius of the ellipse along the y-axis.
     * @param _color The color of the ellipse.
     * @param _select_color The color of the ellipse when selected.
     * @param _border_size The size of the border of the ellipse.
     * @param _border_color The color of the border of the ellipse.
     * @param _text The text associated with the ellipse.
     *
     * @returns None
     */
    EllipseRegion(uint16_t xPos, uint16_t yPos, uint16_t xRad, uint16_t yRad ,uint32_t _color, uint32_t _select_color, uint16_t _border_size, uint32_t _border_color, string _text) : Region(xPos, yPos, _color, _select_color, _border_size, _border_color, _text) {
        radius_x = xRad;
        radius_y = yRad;
    }

    /**
     * Checks if a given point (x, y) is within the boundaries of an ellipse.
     *
     * @param x The x-coordinate of the point.
     * @param y The y-coordinate of the point.
     *
     * @returns True if the point is within the ellipse, False otherwise.
     */
    bool isWithin(uint16_t x, uint16_t y) override {
        return ((x >= position_x - radius_x && x <= position_x + radius_x) && (y >= position_y - radius_y && y <= position_y + radius_y));
    }
    
    /**
     * Draws an ellipse on the LCD display.
     *
     * @param lcd A pointer to the LCD_DISCO_F429ZI object.
     *
     * @returns None
     */
    void draw(LCD_DISCO_F429ZI *lcd) override {
        lcd->SetTextColor(border_color);
        lcd->DrawEllipse(position_x, position_y, radius_x, radius_y);
        lcd->FillEllipse(position_x, position_y, radius_x, radius_y);

        lcd->SetTextColor(color);
        lcd->DrawEllipse(position_x, position_y, radius_x - border_size, radius_y - border_size);
        lcd->FillEllipse(position_x, position_y, radius_x - border_size, radius_y - border_size);

        drawText(lcd);
    }
    
    /**
     * Fills an ellipse on the LCD display with a specified color, position, and size.
     *
     * @param lcd A pointer to the LCD_DISCO_F429ZI object.
     *
     * @returns None
     */
    void fill(LCD_DISCO_F429ZI *lcd) override {
        lcd->SetTextColor(select_color);
        lcd->FillEllipse(position_x, position_y, radius_x - border_size, radius_y - border_size);

        drawText(lcd);
    }
    
    /**
     * Draws text on the LCD display using the dimensions of the region.
     *
     * @param lcd A pointer to the LCD_DISCO_F429ZI object representing the LCD display.
     *
     * @returns None
     */
    void drawText(LCD_DISCO_F429ZI *lcd) override {
        if (text != "") {
            lcd->SetBackColor(color);
            lcd->SetTextColor(text_color);
            lcd->DisplayStringAt(position_x - radius_x + (radius_x / 4), position_y - (TEXT_LINE_HEIGHT / 2), (uint8_t *)text.c_str(), text_mode);
        }
    }

    friend class GUI;
};
