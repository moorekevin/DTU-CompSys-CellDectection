#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "cbmp.h"
#include <string.h>

// Keep odd to maintain symmetry
#define CAPTURING_AREA 13
#define CELL_SIZE 21
#define EROSION_SIZE 3

// Function Prototypes
void greyscalify(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS]);
void apply_threshold(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS]);
void erode(unsigned char erode_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS]);
bool detect_area(unsigned char erode_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS]);
bool is_white_in_exclusion_zone(unsigned char erode_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], int x, int y);
bool is_white_in_detection_area(unsigned char erode_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], int x, int y);
void deleteCell(unsigned char image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], int x, int y);
void copy_image(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS]);
void drawCoordinates(unsigned char image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS]);

//Declaring the array to store the erode_image (unsigned char = unsigned 8 bit)
unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS];
unsigned char outputscalify_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS];
unsigned char thresholded_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS];
unsigned char eroded_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS];
unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS];
unsigned char original_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS];

// Other variables
int cellCount = 0;

// Coordinates
unsigned int xCoords[500];
unsigned int yCoords[500];

//Main function
int main(int argc, char **argv)
{

  //argc counts how may arguments are passed
  //argv[0] is a string with the name of the program
  //argv[1] is the first command line argument (input erode_image)
  //argv[2] is the second command line argument (output erode_image)

  //Checking that 2 arguments are passed
  if (argc != 3)
  {
    fprintf(stderr, "Usage: %s <output file path> <output file path>\n", argv[0]);
    exit(1);
  }

  printf("Example program - 02132 - A1\n");

  //Load erode_image from file
  read_bitmap(argv[1], input_image);

  //Run greyscalification
  greyscalify(input_image, outputscalify_image);

  //Make pixels either black or white
  apply_threshold(outputscalify_image, thresholded_image);
  // Copies the erode_image to the eroded imega
  copy_image(thresholded_image, output_image);

  // Start detecting and eroding
  char *nameWithoutExt = NULL;
  nameWithoutExt = strdup(argv[2]);
  nameWithoutExt[strlen(nameWithoutExt) - 4] = '\0';
  char nameOfFile[40];
  for (int i = 0; true; i++)
  {
    if (i != 0) // Save image before detecting and eroding
    {
      if (!detect_area(output_image)) // If it has not detected white
      {
        break;
      }
      erode(output_image);
    }
    //Save output_image to file
    sprintf(nameOfFile, "%s %d.bmp", nameWithoutExt, i);
    write_bitmap(output_image, nameOfFile);
  }

  drawCoordinates(input_image);
  sprintf(nameOfFile, "%s %s.bmp", nameWithoutExt, "OUTPUT");
  write_bitmap(input_image, nameOfFile);

  printf("Done!\nCells counted: %d", cellCount);
  return 0;
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

//Function to convert pixels of an erode_image into greyscale
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
      colorsum *= 0.33;
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

// void erode(unsigned char erode_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS])
// {
//   copy_image(erode_image, original_image);
//   for (int x = 0; x < BMP_WIDTH; x++)
//   {
//     for (int y = 0; y < BMP_HEIGHT; y++)
//     {
//       if (original_image[x][y][0] > 0)
//       {
//         for (int xOffset = -EROSION_SIZE / 2; xOffset <= EROSION_SIZE / 2; xOffset++)
//         {
//           for (int yOffset = -EROSION_SIZE / 2; yOffset <= EROSION_SIZE / 2; yOffset++)
//           {
//             if (xOffset * xOffset + yOffset * yOffset <= (CAPTURING_AREA / 2) * (CAPTURING_AREA / 2) + CAPTURING_AREA * 0.2)
//             {
//               if (x + xOffset >= 0 && x + xOffset < BMP_WIDTH && y + yOffset >= 0 && y + yOffset < BMP_HEIGHT && original_image[x + xOffset][y + yOffset][0] == 0)
//               {
//                 // Sets the color to black
//                 for (int c = 0; c < BMP_CHANNELS; c++)
//                 {
//                   erode_image[x][y][c] = 0;
//                 }
//                 goto finish;
//               }
//             }
//           }
//         }
//       finish:;
//       }
//     }
//   }
// }

//// DIAMOND SHAPED EROSION ////
void erode(unsigned char erode_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS])
{
  copy_image(erode_image, original_image);
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGHT; y++)
    {
      if (original_image[x][y][0] > 0)
      {
        for (int i = (int)(-EROSION_SIZE * 0.5); i <= (int)(EROSION_SIZE * 0.5); i++)
        {
          int erosionCheck = (int)((EROSION_SIZE - (abs((int)(EROSION_SIZE * 0.5) - (i + (int)(EROSION_SIZE * 0.5))) * 2) - 1) * 0.5);
          for (int j = -erosionCheck; j <= erosionCheck; j++)
          {
            if (x + i >= 0 && x + i < BMP_WIDTH && y + j >= 0 && y + j < BMP_HEIGHT && original_image[x + i][y + j][0] == 0)
            {
              // Sets the color to black
              for (int c = 0; c < BMP_CHANNELS; c++)
              {
                erode_image[x][y][c] = 0;
              }
              goto finish; // If a single black pixel was found just skip checking the others
            }
          }
        }
      }
    finish:;
    }
  }
}

bool detect_area(unsigned char erode_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS])
{
  bool hasDetectedWhite = false;
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGHT; y++)
    {
      // Continue only if detection area is surrounded by black pixels
      if (is_white_in_exclusion_zone(erode_image, x, y))
      {
        hasDetectedWhite = true;
        continue;
      }

      // Continue only if there is white in the detection area (a cell)
      if (!is_white_in_detection_area(erode_image, x, y))
      {
        continue;
      }
      hasDetectedWhite = true;

      // Saving coordinates
      xCoords[cellCount] = x;
      yCoords[cellCount] = y;

      cellCount++;
      deleteCell(erode_image, x, y);
    }
  }
  return hasDetectedWhite;
}

bool is_white_in_exclusion_zone(unsigned char erode_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], int x, int y)
{
  for (int xOffset = (int)(-CAPTURING_AREA * 0.5); xOffset <= (int)(CAPTURING_AREA * 0.5); xOffset++)
  {
    for (int yOffset = (int)(-CAPTURING_AREA * 0.5); yOffset <= (int)(CAPTURING_AREA * 0.5); yOffset++)
    {
      // If the pixel is a part of the exclusion frame (a circle)
      if (xOffset * xOffset + yOffset * yOffset >= (int)(CAPTURING_AREA * 0.5) * (int)(CAPTURING_AREA * 0.5) && xOffset * xOffset + yOffset * yOffset <= (int)(CAPTURING_AREA * 0.5) * (int)(CAPTURING_AREA * 0.5) + CAPTURING_AREA * 1.35)
      {
        // If there is a white pixel on the exclusion frame
        if (x + xOffset >= 0 && x + xOffset < BMP_WIDTH && y + yOffset >= 0 && y + yOffset < BMP_HEIGHT && erode_image[x + xOffset][y + yOffset][0] > 0)
        {
          return true;
        }
      }
    }
  }
  return false;
}

bool is_white_in_detection_area(unsigned char erode_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], int x, int y)
{
  for (int xOffset = (int)(-CAPTURING_AREA * 0.5); xOffset <= (int)(CAPTURING_AREA * 0.5); xOffset++)
  {
    for (int yOffset = (int)(-CAPTURING_AREA * 0.5); yOffset <= (int)(CAPTURING_AREA * 0.5); yOffset++)
    {
      // If the pixel is a part of the detection area (inside the circle)
      if (xOffset * xOffset + yOffset * yOffset < (int)(CAPTURING_AREA * 0.5) * (int)(CAPTURING_AREA * 0.5))
      {
        // If there is a white pixel in the detection area
        if (x + xOffset >= 0 && x + xOffset < BMP_WIDTH && y + yOffset >= 0 && y + yOffset < BMP_HEIGHT && erode_image[x + xOffset][y + yOffset][0] > 0)
        {
          return true;
        }
      }
    }
  }
  return false;
}

// Sets the color of all pixels in detection area to black
void deleteCell(unsigned char image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], int x, int y)
{
  for (int xOffset = (int)(-CAPTURING_AREA * 0.5); xOffset <= (int)(CAPTURING_AREA * 0.5); xOffset++)
  {
    for (int yOffset = (int)(-CAPTURING_AREA * 0.5); yOffset <= (int)(CAPTURING_AREA * 0.5); yOffset++)
    {
      // If the pixel is a part of the detection area (inside the circle)
      if (xOffset * xOffset + yOffset * yOffset < (int)(CAPTURING_AREA * 0.5) * (int)(CAPTURING_AREA * 0.5))
      {
        // If there is a white pixel in the detection area
        if (x + xOffset >= 0 && x + xOffset < BMP_WIDTH && y + yOffset >= 0 && y + yOffset < BMP_HEIGHT)
        {
          // Mark every cell with blue
          image[x + xOffset][y + yOffset][0] = 0;
          // image[x + xOffset][y + yOffset][1] = 0;
          image[x + xOffset][y + yOffset][2] = 255;

          // This deletes the cells
          // for (int c = 0; c < BMP_CHANNELS; c++)
          // {
          //   image[x + xOffset][y + yOffset][c] = 0;
          // }
        }
      }
    }
  }
}

void drawCoordinates(unsigned char image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS])
{
  for (int i = 0; i < cellCount; i++)
  {
    for (int j = (int)(-CELL_SIZE * 0.5); j <= (int)(CELL_SIZE * 0.5); j++)
    {
      for (int k = -1; k <= 1; k++)
      {
        if (xCoords[i] + j >= 0 && xCoords[i] + j < BMP_WIDTH && yCoords[i] + k >= 0 && yCoords[i] + k < BMP_HEIGHT)
        {
          image[xCoords[i] + j][yCoords[i] + k][0] = 255;
          image[xCoords[i] + j][yCoords[i] + k][1] = 0;
          image[xCoords[i] + j][yCoords[i] + k][2] = 0;
        }
        if (yCoords[i] + j >= 0 && yCoords[i] + j < BMP_WIDTH && xCoords[i] + k >= 0 && xCoords[i] + k < BMP_HEIGHT)
        {
          image[xCoords[i] + k][yCoords[i] + j][0] = 255;
          image[xCoords[i] + k][yCoords[i] + j][1] = 0;
          image[xCoords[i] + k][yCoords[i] + j][2] = 0;
        }
      }
    }
  }
}
