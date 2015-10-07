//////////////////////////////////////////////////////////////////////////////
// particle-weather-widget
//////////////////////////////////////////////////////////////////////////////
// Uses the Sparkfun Photon Micro OLED Shield as a simple weather widget.
// https://www.sparkfun.com/products/13628

// Simulate a typical webhook respone:
// curl -H "Authorization: Bearer asdfasdfasdfasdf" \
//     https://api.spark.io/v1/devices/1234123412341234/weather \
//     -d params=63.61~69.27~60.62~clear-night~1444052817~1444095007

#include "oled.h"
#include "icons.h"
#include "math.h"

///////////////
// Variables //
///////////////

#define TIME_ZONE -7

typedef void (*callback)(int);
void doTransition(int iframe, callback from, callback toTitle, callback toFrame);

// this array keeps function pointers to all frames
// frames are the single views that slide from right to left
#define FRAME_COUNT 3
callback frameCallbacks[FRAME_COUNT] = {drawWeatherNow, drawSunrise, drawSunset};
callback titleCallbacks[FRAME_COUNT] = {drawWeatherTitle, drawSunriseTitle, drawSunsetTitle};

//////////////////
// Main Program //
//////////////////

callback last = NULL;
int currentFrame = 0;
long weatherLoadedEpoch = 0;
long weatherAttemptedEpoch = 0;

void setup() {
    Particle.subscribe("hook-response/get_weather", gotWeatherData, MY_DEVICES);
    Particle.function("weather", weather);

    oled.begin();    // Initialize the OLED
    oled.clear(ALL); // Clear the display's internal memory
    oled.display();  // Display what's in the buffer (splashscreen)

    Time.zone(TIME_ZONE);
}

void loop() {
    if (Time.now() > weatherAttemptedEpoch + 600) {
        Particle.publish("get_weather");
        weatherAttemptedEpoch = Time.now();
    }
    if (weatherLoadedEpoch > 0) {
        doTransition(currentFrame, last, titleCallbacks[currentFrame], frameCallbacks[currentFrame]);
        last = frameCallbacks[currentFrame];
        currentFrame = (currentFrame + 1) % FRAME_COUNT;
    }
    delay(3000);
}

////////////
// Frames //
////////////

void doTransition(int iframe, callback from, callback toTitle, callback toFrame) {
    float p;
    int x;

    for (float i = 0.0f; i <= 1.0f; i += 0.005f) {
        oled.clear(PAGE);
        p = interpolate(i);
        if (p <= 0.5f) {
            x = (p * 2.0f) * SCREEN_WIDTH;
            if (from != NULL) {
                (*from)(-x);
            }
            (*toTitle)(SCREEN_WIDTH - x);
        }
        else {
            x = (2.0f * p - 1.0f) * SCREEN_WIDTH;
            (*toTitle)(-x);
            (*toFrame)(SCREEN_WIDTH - x);
        }
        drawBar(iframe - 1 + p, FRAME_COUNT);
        if (iframe == 0 && from != NULL) {
            drawBar(FRAME_COUNT - 1 + p, FRAME_COUNT);
        }
        oled.display();
    }

    clearBar();
    oled.display();
}

float interpolate(float input) {
    return (float)(cos((input + 1) * M_PI) / 2.0f) + 0.5f;
}

/////////////
// Tab Bar //
/////////////

// total: number of screens
// current: current screen [0,total). fractional if transitioning between screens
void drawBar(float current, int total) {
    int padding = 2;
    int y = SCREEN_HEIGHT - 1;
    int w = SCREEN_WIDTH / total;
    int x1 = w * current + padding;
    int x2 = w * current + w - padding;
    if (x1 < 0) x1 = 0;
    if (x1 >= SCREEN_WIDTH) return;
    if (x2 >= SCREEN_WIDTH) x2 = SCREEN_WIDTH - 1;
    if (x2 < 0) return;
    oled.line(x1, y, x2, y);
}

void clearBar() {
    int y = SCREEN_HEIGHT - 1;
    oled.line(0, y, SCREEN_WIDTH - 1, y, BLACK, NORM);
}

//////////////////
// Info Screens //
//////////////////

String weather_high;
String weather_low;
String weather_curr;
String weather_icon;
String weather_sunrise;
String weather_sunset;

void drawWeatherTitle(int x) {
    printTitle(x, "Weather", 0);
}

void drawWeatherNow(int x) {
    drawXbm(x, 0, ICON_WIDTH, ICON_HEIGHT, getIconFromString(weather_icon));
    
    oled.setFontType(0);
    int y = SCREEN_HEIGHT - oled.getFontHeight() - 1;
    setCursor(x, y);
    writeNoWrap(24);
    printNoWrap(weather_high);
    writeNoWrap(247);

    setCursor(x + SCREEN_WIDTH - oled.getFontWidth() * (3 + strlen(weather_low)), y);
    writeNoWrap(25);
    printNoWrap(weather_low);
    writeNoWrap(247);

    if (strlen(weather_curr) > 2) {
        setCursor(x + SCREEN_WIDTH - (oled.getFontWidth() + 1) * (strlen(weather_curr) + 1), 12);
        printNoWrap(weather_curr);
        writeNoWrap(247);
    }
    else {
        oled.setFontType(1);
        setCursor(x + SCREEN_WIDTH - (oled.getFontWidth() + 1) * strlen(weather_curr) - 3, 10);
        printNoWrap(weather_curr);
        oled.circle(x + SCREEN_WIDTH - 3, 12, 2);
    }
}

void drawSunriseTitle(int x) {
    printTitle(x, "Sunrise", 0);
}

void drawSunrise(int x) {
    drawXbm(16 + x, 0, ICON_WIDTH, ICON_HEIGHT, sunrise_bits);

    oled.setFontType(0);
    int y = SCREEN_HEIGHT - oled.getFontHeight() - 7;
    setCursor(x + SCREEN_WIDTH/2 - (oled.getFontWidth()+1) * 2, y);
    printNoWrap(weather_sunrise);
}

void drawSunsetTitle(int x) {
    printTitle(x, "Sunset", 0);
}

void drawSunset(int x) {
    drawXbm(16 + x, 0, ICON_WIDTH, ICON_HEIGHT, sunset_bits);

    oled.setFontType(0);
    int y = SCREEN_HEIGHT - oled.getFontHeight() - 7;
    setCursor(x + SCREEN_WIDTH/2 - (oled.getFontWidth()+1) * 2.5f, y);
    printNoWrap(weather_sunset);
}

///////////////////
// Weather Utils //
///////////////////

void gotWeatherData(const char *name, const char *data) {
    weather(data);
}

// Data should be formatted thusly:
// "current~high~low~iconstring~sunriseepoch~sunsetepoch"
int weather(const char *data) {
    String s = data;
    int l = strlen(data);
    int i = 0;
    
    weather_curr = "";
    bool dot = false;
    while (i < l && data[i] != '~') {
        if (data[i] == '.' || dot) {
            dot = true;
        }
        else {
            weather_curr += data[i];
        }
        i++;
    }
    i++;

    weather_high = "";
    dot = false;
    while (i < l && data[i] != '~') {
        if (data[i] == '.' || dot) {
            dot = true;
        }
        else {
            weather_high += data[i];
        }
        i++;
    }
    i++;

    weather_low = "";
    dot = false;
    while (i < l && data[i] != '~') {
        if (data[i] == '.' || dot) {
            dot = true;
        }
        else {
            weather_low += data[i];
        }
        i++;
    }
    i++;
    
    weather_icon = "";
    while (i < l && data[i] != '~') {
        weather_icon += data[i];
        i++;
    }
    i++;
    
    long epoch = 0;
    while (i < l && data[i] != '~') {
        epoch = epoch * 10 + (data[i] - '0');
        i++;
    }
    weather_sunrise = getTimeOfDayStr(epoch);
    i++;

    epoch = 0;
    while (i < l && data[i] != '~') {
        epoch = epoch * 10 + (data[i] - '0');
        i++;
    }
    weather_sunset = getTimeOfDayStr(epoch);

    weatherLoadedEpoch = Time.now();
    return 1;
}

////////////////////
// OLED Utilities //
////////////////////

int cursorx = 0;
int cursory = 0;

void drawXbm(int x, int y, int width, int height, const unsigned char *xbm) {
    if (width % 8 != 0) {
        width =  ((width / 8) + 1) * 8;
    }
    for (int i = 0; i < width * height / 8; i++ ) {
        unsigned char charColumn = pgm_read_byte(xbm + i);
        for (int j = 0; j < 8; j++) {
            int targetX = (i * 8 + j) % width + x;
            int targetY = (8 * i / (width)) + y;
            if (bitRead(charColumn, j)) {
                oled.pixel(targetX, targetY);
            }
        }
    }
}

void setCursor(int x, int y) {
    cursorx = x;
    cursory = y;
}

// Center and print a small title
// This function is quick and dirty. Only works for titles one
// line long.
void printTitle(int x, String title, int font) {
    int middleX = SCREEN_WIDTH / 2;
    int middleY = SCREEN_HEIGHT / 2;

    oled.setFontType(font);

    setCursor(x + middleX - ((oled.getFontWidth()+1) * title.length() / 2),
                   middleY - (oled.getFontHeight() / 2));
                   
    // Print the title:
    printNoWrap(title);
}

void writeNoWrap(char c) {
     if (cursorx >= SCREEN_WIDTH) {
        return;
    }
    oled.drawChar(cursorx, cursory, c);
    cursorx += oled.getFontWidth() + 1;
}

void printNoWrap(String s) {
    for (int i = 0; i < strlen(s); i++) {
        char c = s.charAt(i);
        writeNoWrap(c);
        if (cursorx >= SCREEN_WIDTH) {
            return;
        }
    } 
}

void printNoWrap(int d) {
    char str[16];
    sprintf(str, "%d", d);
    printNoWrap(str);
}

///////////////
// Utilities //
///////////////

String getTimeOfDayStr(long epoch) {
    return String(Time.hour(epoch)) 
        + ":" 
        + (Time.minute(epoch) < 10 ? "0" : "")
        + String(Time.minute(epoch));
}


