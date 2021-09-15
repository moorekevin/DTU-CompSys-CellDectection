#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "cbmp.h"
#include <string.h>

#define CAPTURING_AREA 12

// Function Prototypes
void greyscalify(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS]);
void apply_threshold(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS]);
void erode(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS]);
void detect_area(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS]);
bool is_white_in_exclusion_zone(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], int x, int y);
bool is_white_in_detection_area(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], int x, int y);
void deleteCell(unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], int x, int y);
void copy_image(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS]);

//Declaring the array to store the image (unsigned char = unsigned 8 bit)
unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS];
unsigned char outputscalify_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS];
unsigned char thresholded_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS];
unsigned char eroded_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS];
unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS];

// Other variables
int cellCount = 0;

//Main function
int main(int argc, char **argv)
{
  //argc counts how may arguments are passed
  //argv[0] is a string with the name of the program
  //argv[1] is the first command line argument (input image)
  //argv[2] is the second command line argument (output image)

  //Checking that 2 arguments are passed
  if (argc != 3)
  {
    fprintf(stderr, "Usage: %s <output file path> <output file path>\n", argv[0]);
    exit(1);
  }

  printf("Example program - 02132 - A1\n");

  //Load image from file
  read_bitmap(argv[1], input_image);

  //Run greyscalification
  greyscalify(input_image, outputscalify_image);

  //Make pixels either black or white
  apply_threshold(outputscalify_image, thresholded_image);

  // unsigned char *imagepointer = thresholded_image;

  // //Erode image
  // for (int i = 0; i < 10; i++)
  // {
  //   if (i != 0)
  //   { //Save image to file before erosion
  //     erode(imagepointer, eroded_image);
  //     imagepointer = eroded_image;
  //   }
  //   //Save eroded image to file
  //   char nameOfFile[40];
  //   sprintf(nameOfFile, "%s %d.bmp", argv[2], i);
  //   write_bitmap(output_image, nameOfFile);
  // }

  erode(thresholded_image, eroded_image);

  detect_area(eroded_image, output_image);

  write_bitmap(output_image, argv[2]);

  //printf("Done!\n %d", cellCount);
  printf("Done!");
  return 0;
}

//Function to convert pixels of an image into greyscale
void greyscalify(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS])
{
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGHT; y++)
    {
      int colorsum = 0;
      for (int c = 0; c < BMP_CHANNELS; c++)
      {
        colorsum += input_image[x][y][c];
      }
      colorsum /= 3;
      for (int c = 0; c < BMP_CHANNELS; c++)
      {
        output_image[x][y][c] = colorsum;
      }
    }
  }
}

void apply_threshold(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS])
{
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGHT; y++)
    {
      for (int c = 0; c < BMP_CHANNELS; c++)
      {
        output_image[x][y][c] = input_image[x][y][c] > 90 ? 255 : 0;
      }
    }
  }
}

void erode(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS])
{
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGHT; y++)
    {
      if (input_image[x][y][0] == 0)
      {
        output_image[x][y][0] = 0;
        continue;
      }
      if ((x > 0 && input_image[x - 1][y][0] == 0) ||             // Checking left side
          (x < BMP_WIDTH - 1 && input_image[x + 1][y][0] == 0) || // Checking right side
          (y > 0 && input_image[x][y - 1][0] == 0) ||             // Checking top side
          (y < BMP_HEIGHT - 1 && input_image[x][y + 1][0] == 0)   // Checking bottom side
      )
      {
        // Sets the color to black
        for (int c = 0; c < BMP_CHANNELS; c++)
        {
          output_image[x][y][c] = 0;
        }
      }
      else
      {
        for (int c = 0; c < BMP_CHANNELS; c++)
        {
          output_image[x][y][c] = input_image[x][y][c];
        }
      }
    }
  }
}

void detect_area(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS])
{
  copy_image(input_image, output_image);
  for (int x = 0; x < BMP_WIDTH - CAPTURING_AREA; x++)
  {
    for (int y = 0; y < BMP_HEIGHT - CAPTURING_AREA; y++)
    {

      // Continue only if detection area is surrounded by black pixels
      if (is_white_in_exclusion_zone(input_image, output_image, x, y))
      {
        continue;
      }

      // Continue only if there is white in the detection area (a cell)
      if (!is_white_in_detection_area(input_image, output_image, x, y))
      {
        continue;
      }

      cellCount++;
      deleteCell(output_image, x, y);
    }
  }
}

void copy_image(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS])
{
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGHT; y++)
    {
      for (int c = 0; c < BMP_CHANNELS; c++)
      {
        output_image[x][y][c] = input_image[x][y][c];
      }
    }
  }
}

bool is_white_in_exclusion_zone(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], int x, int y)
{
  for (int xOffset = -1; xOffset < CAPTURING_AREA + 1; xOffset++)
  {
    for (int yOffset = -1; yOffset < CAPTURING_AREA + 1; yOffset++)
    {
      // If the pixel is a part of the exclusion frame
      if ((xOffset == -1 || xOffset == CAPTURING_AREA) && (yOffset == -1 || yOffset == CAPTURING_AREA))
      {
        // If there is a white pixel on the exclusion frame
        if (x + xOffset >= 0 && x + xOffset < BMP_WIDTH && y + yOffset >= 0 && y + yOffset < BMP_HEIGHT && input_image[x + xOffset][y + yOffset][0] > 0)
        {
          return true;
        }
      }
    }
  }
  return false;
}

bool is_white_in_detection_area(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], int x, int y)
{
  for (int xOffset = 0; xOffset < CAPTURING_AREA; xOffset++)
  {
    for (int yOffset = 0; yOffset < CAPTURING_AREA; yOffset++)
    {
      // If the detection area contains a cell
      if (input_image[x + xOffset][y + yOffset][0] > 1)
      {
        return true;
      }
    }
  }
  return false;
}

// Sets the color of all pixels in detection area to black
void deleteCell(unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], int x, int y)
{
  for (int xOffset = 0; xOffset < CAPTURING_AREA; xOffset++)
  {
    for (int yOffset = 0; yOffset < CAPTURING_AREA; yOffset++)
    {
      for (int c = 0; c < BMP_CHANNELS; c++)
      {
        output_image[x + xOffset][y + yOffset][c] = 0;
      }
    }
  }
}

/////////////////////// OG CONTENT ////////////////////////////
//Function to invert pixels of an image (negative)
// void invert(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS]){
//   for (int x = 0; x < BMP_WIDTH; x++)
//   {
//     for (int y = 0; y < BMP_HEIGHT; y++)
//     {
//       for (int c = 0; c < BMP_CHANNELS; c++)
//       {
//       output_image[x][y][c] = 255 - input_image[x][y][c];
//       }
//     }
//   }
// }
