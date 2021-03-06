#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "cbmp.h"
#include <string.h>
#include <time.h>

// Keep odd to maintain symmetry
#define CAPTURING_AREA 13
#define CELL_SIZE 21
#define EROSION_SIZE 3

// Function Prototypes
void grayscalify(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS]);
void apply_threshold(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS]);
void erode(unsigned char erode_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS]);
bool detect_area(unsigned char erode_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS]);
bool is_white_in_exclusion_zone(unsigned char erode_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], int x, int y);
bool is_white_in_detection_area(unsigned char erode_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], int x, int y);
void deleteCell(unsigned char image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], int x, int y);
void copy_image(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS]);
void drawCoordinates(unsigned char image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS]);

//Declaring the arrays to store the images
unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS];
unsigned char image_1[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS];
unsigned char image_2[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS];

// Other variables
int cellCount = 0;

// Coordinates
unsigned int xCoords[500];
unsigned int yCoords[500];

//Main function
int main(int argc, char **argv)
{
  clock_t start, end;
  double cpu_time_used;
  start = clock();

  //Checking that 2 arguments are passed
  if (argc != 3)
  {
    fprintf(stderr, "Usage: %s <output file path> <output file path>\n", argv[0]);
    exit(1);
  }

  //Load input_image from file
  read_bitmap(argv[1], input_image);

  //Run greyscalification on input image
  grayscalify(input_image, image_2);

  //Make pixels on grayscaled image either black or white
  apply_threshold(image_2, image_1);

  // Start detecting and eroding
  char *nameWithoutExt = NULL;
  nameWithoutExt = strdup(argv[2]);
  nameWithoutExt[strlen(nameWithoutExt) - 4] = '\0';
  char nameOfFile[40];
  for (int i = 0; true; i++)
  {
    if (i != 0) // Save image before detecting and eroding
    {
      if (!detect_area(image_1)) // If it has not detected white
      {
        break;
      }
      erode(image_1);
    }
    //Save output_image to file
    // sprintf(nameOfFile, "%s %d.bmp", nameWithoutExt, i);
    // write_bitmap(image_1, nameOfFile);
  }

  drawCoordinates(input_image);
  sprintf(nameOfFile, "%s %s.bmp", nameWithoutExt, "OUTPUT");
  write_bitmap(input_image, nameOfFile);

  end = clock();
  cpu_time_used = end - start;
  printf("Total time: %f ms\n", cpu_time_used * 1000.0 / CLOCKS_PER_SEC);

  // for (int i = 0; i < cellCount; i++)
  // {
  //   printf("(%d,%d)\n", xCoords[i], yCoords[i]);
  // }

  // printf("\n\nDone!\nCells counted: %d", cellCount);
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
void grayscalify(unsigned char input_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], unsigned char output_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS])
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

// // CIRCULAR SHAPE
// void erode(unsigned char erode_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS])
// {
//   // start = clock();
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
//   // end = clock();
//   // cpu_time_used = end - start;
// }

// DIAMOND SHAPED EROSION ////
void erode(unsigned char erode_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS])
{
  // start = clock();
  // Make image_2 equal to erode_image to save the original image
  copy_image(erode_image, image_2);
  for (int x = 0; x < BMP_WIDTH; x++)
  {
    for (int y = 0; y < BMP_HEIGHT; y++)
    {
      if (image_2[x][y][0] > 0)
      {
        for (int i = (int)(-EROSION_SIZE * 0.5); i <= (int)(EROSION_SIZE * 0.5); i++)
        {
          int erosionCheck = (int)((EROSION_SIZE - (abs((int)(EROSION_SIZE * 0.5) - (i + (int)(EROSION_SIZE * 0.5))) * 2) - 1) * 0.5);
          for (int j = -erosionCheck; j <= erosionCheck; j++)
          {
            if (x + i >= 0 && x + i < BMP_WIDTH && y + j >= 0 && y + j < BMP_HEIGHT && image_2[x + i][y + j][0] == 0)
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
  // end = clock();
  // cpu_time_used = end - start;
}

bool detect_area(unsigned char erode_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS])
{
  bool hasDetectedWhite = false;
  for (int x = 0; x < BMP_WIDTH - CAPTURING_AREA + 1; x++)
  {
    for (int y = 0; y < BMP_HEIGHT - CAPTURING_AREA + 1; y++)
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
      xCoords[cellCount] = x + CAPTURING_AREA / 2;
      yCoords[cellCount] = y + CAPTURING_AREA / 2;

      cellCount++;
      deleteCell(erode_image, x, y);
    }
  }
  return hasDetectedWhite;
}

bool is_white_in_exclusion_zone(unsigned char erode_image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], int x, int y)
{
  for (int xOffset = -1; xOffset < CAPTURING_AREA + 1; xOffset++)
  {
    for (int yOffset = -1; yOffset < CAPTURING_AREA + 1; yOffset++)
    {
      // If the pixel is a part of the exclusion frame
      if ((xOffset == -1 || xOffset == CAPTURING_AREA) || (yOffset == -1 || yOffset == CAPTURING_AREA))
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
  for (int xOffset = 0; xOffset < CAPTURING_AREA; xOffset++)
  {
    for (int yOffset = 0; yOffset < CAPTURING_AREA; yOffset++)
    {
      // If the detection area contains a cell
      if (erode_image[x + xOffset][y + yOffset][0] > 1)
      {
        return true;
      }
    }
  }
  return false;
}

// Sets the color of all pixels in detection area to black
void deleteCell(unsigned char image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS], int x, int y)
{
  for (int xOffset = 0; xOffset < CAPTURING_AREA; xOffset++)
  {
    for (int yOffset = 0; yOffset < CAPTURING_AREA; yOffset++)
    {
      // Mark every cell with blue
      // image[x + xOffset][y + yOffset][0] = 0;
      // image[x + xOffset][y + yOffset][1] = 0;
      // image[x + xOffset][y + yOffset][2] = 255;

      // This deletes the cells
      for (int c = 0; c < BMP_CHANNELS; c++)
      {
        image[x + xOffset][y + yOffset][c] = 0;
      }
    }
  }
}

void drawCoordinates(unsigned char image[BMP_WIDTH][BMP_HEIGHT][BMP_CHANNELS])
{
  for (int i = 0; i < cellCount; i++)
  {
    for (int j = -CELL_SIZE / 2; j <= CELL_SIZE / 2; j++)
    {
      for (int k = -1; k <= 1; k++)
      {
        if (xCoords[i] + j >= 0 && xCoords[i] + j < BMP_WIDTH)
        {
          image[xCoords[i] + j][yCoords[i] + k][0] = 255;
          image[xCoords[i] + j][yCoords[i] + k][1] = 0;
          image[xCoords[i] + j][yCoords[i] + k][2] = 0;
        }
        if (yCoords[i] + j >= 0 && yCoords[i] + j < BMP_WIDTH)
        {
          image[xCoords[i] + k][yCoords[i] + j][0] = 255;
          image[xCoords[i] + k][yCoords[i] + j][1] = 0;
          image[xCoords[i] + k][yCoords[i] + j][2] = 0;
        }
      }
    }
  }
}