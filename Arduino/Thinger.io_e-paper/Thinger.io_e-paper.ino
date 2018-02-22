#define _DEBUG_
#define _DISABLE_TLS_

/*
  INCLUDES
*/

// WiFi 101
#include <WiFi101.h>

// Thinger.io
#include <ThingerWifi101.h>

// Waveshare e-paper module
#include <SPI.h>
#include <epd1in54.h>
#include <epdpaint.h>

// Image data for "We are open", "Sorry, we are closed" and "Be right back" ...
#include "imagedata_open.h"
#include "imagedata_closed.h"
#include "imagedata_away.h"

/*
  DEFINES
*/

// WiFi parameters
#define WIFI_SSID "REPLACE_ME__WIFI_SSID"
#define WIFI_SSID_PASSWORD "REPLACE_ME__WIFI_PASSWORD"

// Thinger.io parameters
#define THINGERIO_USERNAME "REPLACE_ME__THINGER_IO_USER"
#define THINGERIO_DEVICE_ID "REPLACE_ME__THINGER_IO_DEVICE_ID"
#define THINGERIO_DEVICE_CREDENTIAL "Iq7OxG4hREPLACE_ME__THINGER_IO_DEVICE_CREDENTIAL"

// E-ink colours
#define COLORED 0
#define UNCOLORED 1

#define LED_PIN 6

/*
  VARIABLES
*/

// Thinger.io device handle
ThingerWifi101 thing(THINGERIO_USERNAME, THINGERIO_DEVICE_ID, THINGERIO_DEVICE_CREDENTIAL);

// E-paper image buffer and handle
unsigned char image[5000];
Paint paint(image, 0, 0);
Epd epd;

/*
  SETUP
*/

void setup()
{
  // Configure serial
  Serial.begin(115200);

  // Initialize e-paper
  if (epd.Init(lut_full_update) != 0)
  {
    Serial.println("e-Paper init failed!");
    return;
  }

  // Configure wifi network
  thing.add_wifi(WIFI_SSID, WIFI_SSID_PASSWORD);

  // Configure ressource(s) ...

  // Handle for "text" resource:
  thing["text"] << [](pson & in) {
    boolean clean = in["clean"];
    int posX = in["posX"];
    int posY = in["posY"];
    int fontSize = in["fontSize"];
    String content = in["content"];
#ifdef _DEBUG_
    Serial.println("Received 'text' command ...");
    Serial.print("text.clean:");
    Serial.println(clean);
    Serial.print("text.posX:");
    Serial.println(posX);
    Serial.print("text.posY:");
    Serial.println(posY);
    Serial.print("text.fontSize:");
    Serial.println(fontSize);
    Serial.print("text.content:");
    Serial.println(content);
#endif
    displayText(clean, posX, posY, fontSize, content);
  };


  // Handle for "image" resource:
  thing["image"] << [](pson & in) {
    boolean clean = in["clean"];
    int number = in["number"];
#ifdef _DEBUG_
    Serial.println("Received 'image' command ...");
    Serial.print("image.clean:");
    Serial.println(clean);
    Serial.print("image.number:");
    Serial.println(number);
#endif
    displayImage(clean, number);
  };
}

/*
  LOOP
*/

void loop()
{
  // Just call Thinger.io device handler ...
  thing.handle();
}

/*
  FUNCTIONS
*/

// Handler called function for displaying text:
void displayText(boolean clean, int posX, int posY, int fontSize, String text)
{
  if (clean)
  {
    ePaperClear();
  }
  ePaperDisplayText(clean, posX, posY, fontSize, text);
}

// Handler called function for displaying images:
void displayImage(boolean clean, int number)
{
  if (clean)
  {
    ePaperClear();
  }

  const unsigned char *image_data;
  switch (number)
  {
    case 0:
      image_data = IMAGE_DATA_OPEN;
      break;
    case 1:
      image_data = IMAGE_DATA_AWAY;
      break;
    case 2:
      image_data = IMAGE_DATA_CLOSED;
      break;
  }

  ePaperShowImage(image_data);
}

// E-paper functions ...

// Clear e-ink display:
void ePaperClear()
{
  /**
     From epd1in54-demo.ino - Waveshare 1.54inch e-paper display demo:
     There are 2 memory areas embedded in the e-paper display
     and once the display is refreshed, the memory area will be auto-toggled,
     i.e. the next action of SetFrameMemory will set the other memory area
     therefore you have to clear the frame memory twice.
  */
  epd.ClearFrameMemory(0xFF);   // bit set = white, bit reset = black
  epd.DisplayFrame();
  epd.ClearFrameMemory(0xFF);   // bit set = white, bit reset = black
  epd.DisplayFrame();

  if (epd.Init(lut_partial_update) != 0)
  {
    Serial.print("e-Paper init failed!");
    return;
  }
}

// Display text on e-ink display:
void ePaperDisplayText(boolean clean, int posX, int posY, int fontSize, String text)
{
  paint.SetRotate(ROTATE_0);
  paint.SetWidth(200);
  paint.SetHeight(200);

  if (clean)
  {
    paint.Clear(UNCOLORED);
  }

  sFONT *font;
  switch (fontSize)
  {
    case 8:
      font = &Font8;
      break;
    case 12:
      font = &Font12;
      break;
    case 16:
      font = &Font16;
      break;
    case 20:
      font = &Font20;
      break;
    case 24:
      font = &Font24;
      break;
    default:
      font = &Font24;
      break;
  }

  paint.DrawStringAt(posX, posY, text.c_str(), font, COLORED);
  epd.SetFrameMemory(paint.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());
  epd.DisplayFrame();

  if (epd.Init(lut_partial_update) != 0)
  {
    Serial.print("e-Paper init failed");
    return;
  }
}


// Display image on e-ink display:
void ePaperShowImage(const unsigned char image_data[])
{
  /**
      From epd1in54-demo.ino - Waveshare 1.54inch e-paper display demo:
      There are 2 memory areas embedded in the e-paper display
      and once the display is refreshed, the memory area will be auto-toggled,
      i.e. the next action of SetFrameMemory will set the other memory area
      therefore you have to clear the frame memory twice.
  */
  epd.SetFrameMemory(image_data);
  epd.DisplayFrame();
  epd.SetFrameMemory(image_data);
  epd.DisplayFrame();

  if (epd.Init(lut_partial_update) != 0)
  {
    Serial.print("e-Paper init failed!");
    return;
  }
}
