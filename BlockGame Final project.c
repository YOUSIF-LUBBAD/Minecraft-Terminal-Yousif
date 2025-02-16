// ==================================================> INCLUDES <==================================================

#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <time.h>
#include <math.h>

#include <string.h>
#include <unistd.h> // For sleep()

#define VERTICIES 3000
#define POLYGONS 3000
#define WORLD_SIZE 16
#define NUM_BLOCKS 17

// ==================================================> STRUCTS <==================================================

/* 
	Potential Improvement for readability.
	Would require rewrite
	
typedef struct point {
	int x, y, z;
}Point;

typedef struct polygon {
	int p1, p2, p3;
}Polygon;
*/

// ==================================================> PROTOTYPES <==================================================

void convertScreen(int gameCoords[VERTICIES][3], double screenCoords[VERTICIES][3], double playerPos[3], double playerRot[3]);

void fillPolygon(double polygon[3][3], int color, char draw);

int isInside(double poly[3][2], int pointX, int pointY);

void getGameInputs(double playerMove[], double playerRot[], int* menu, int* grounded, int* destroy, int* blockType);

int getMenuInputs(int* menuX, int* menuY, int* menu);

void drawAll(int allPolygons[POLYGONS][4], double screenCoords[VERTICIES][3], int drawOrder[POLYGONS], int gameCoords[VERTICIES][3]);

void generateTerrain(int blockPositions[WORLD_SIZE][WORLD_SIZE][WORLD_SIZE], int seed);

void loadTerrain(int blockPositions[WORLD_SIZE][WORLD_SIZE][WORLD_SIZE], FILE* level);

void saveWorld(int blockPositions[WORLD_SIZE][WORLD_SIZE][WORLD_SIZE], FILE* level);

void generatePolygons(int blockPositions[WORLD_SIZE][WORLD_SIZE][WORLD_SIZE], int polygons[POLYGONS][4], int gameCoords[VERTICIES][3], int blockColors[][3]);

int addVertex(int x, int y, int z, int vertexList[VERTICIES][3]);

void addPoly(int vert1, int vert2, int vert3, int color, int polyList[POLYGONS][4]);

void cullBack(int polygons[POLYGONS][4], double screenCoords[VERTICIES][3], int drawOrder[POLYGONS]);

void orderPoly(int polygons[POLYGONS][4], double screenCoords[VERTICIES][3], int drawOrder[POLYGONS]);

int checkCollisions(double playerPos[3], double playerMove[3], int blockPositions[WORLD_SIZE][WORLD_SIZE][WORLD_SIZE]);

void drawPaused(int menuX, int menuY);

void playerTouching(double playerPos[], double playerRot[], int blockPositions[WORLD_SIZE][WORLD_SIZE][WORLD_SIZE], int blocksTouching[2][3]);

void editBlock(int blockPositions[WORLD_SIZE][WORLD_SIZE][WORLD_SIZE], int blocksTouching[2][3], int blockType, int destroy);

void drawInventory(int blockColors[][3], int blockType);



void setColor(const char *color);

void resetColor();

void welcomeScreen();

void displayMenu();

int getValidatedChoice();

int playMenu();

void loadMenu(FILE** level);

void quitMenu();

void drawGraphicalMenu(void);

// ==================================================> GLOBAL <==================================================

double deltaTime;

// ==================================================> MAIN <==================================================

int main() {
	//Main menu section
	int choice;
	int seed = -1;
	FILE* level = NULL;
 
    // Welcome Screen
    welcomeScreen();
      
       // Graphical Menu
    drawGraphicalMenu();
      
      // Pause after the graphical menu
    printf("\nPress Enter to continue...");
    getchar(); // Wait for the user to press Enter
 
    while (1) {
        // Display the menu
        displayMenu();
 
        // Get the validated choice
        choice = getValidatedChoice();
 
        // Handle menu options
        switch (choice) {
            case 1:
                seed = playMenu();
                break;
            case 2:
                loadMenu(&level);
                break;
            case 3:
                quitMenu();
                exit(0);
            default:
                printf("Invalid choice. Please try again.\n");
		}
		if (seed != -1) break;
		if (level != NULL) break;
	}
	// ^^Main Menu^^
	
	
	// window / keyboard setup
	// LINES and COLS are values for the screen size
	initscr();
	keypad(stdscr, TRUE);
	nodelay(stdscr, TRUE);
	start_color();
	init_pair(1, COLOR_BLACK, COLOR_CYAN);
	bkgd(COLOR_PAIR(1));
	init_pair(2, COLOR_BLACK, COLOR_GREEN);
	init_pair(3, COLOR_YELLOW, COLOR_BLACK);
	init_pair(4, COLOR_WHITE, COLOR_BLACK);
	init_pair(5, COLOR_BLACK, COLOR_YELLOW);
	init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(7, COLOR_BLACK, COLOR_RED);
	init_pair(8, COLOR_RED, COLOR_YELLOW);
	init_pair(9, COLOR_BLACK, COLOR_BLUE);
	init_pair(10, COLOR_RED, COLOR_WHITE);
	init_pair(13, COLOR_WHITE, COLOR_MAGENTA);
	init_pair(14, COLOR_BLACK, COLOR_MAGENTA);
	init_pair(15, COLOR_BLACK, COLOR_WHITE);
	init_pair(16, COLOR_WHITE, COLOR_BLACK);
	refresh();
	
	// 3D vertex coordinates
	int gameCoords[VERTICIES][3];
	for (int i = 0; i < VERTICIES; i++) gameCoords[i][1] = -1;
	
	// Screen vertex coordinates
	double screenCoords[VERTICIES][3];
	
	// Polygon list
	int polygons[POLYGONS][4];
	for (int i = 0; i < POLYGONS; i++) polygons[i][1] = -1;
	int drawOrder[POLYGONS];
	
	// Stores the type of block at position [x][y][z]
	int blockPositions[WORLD_SIZE][WORLD_SIZE][WORLD_SIZE];
	int blockColors[][3] = {{2,-3,-3},{-3,-3,-3},{-4,-4,-4},{5,-3,5},{-2,-2,-2},{7,7,7},{-8,-8,-8},{8,8,8},{2,2,2},{1,1,1},{9,9,9},{14,14,14},{-10,-10,-10},{15,15,15},{-6,-6,-6},{-14,-14,-14},{3,3,3},{-13,-16,-16}};
	
	// Player position
	double playerPos[3] = {16,32,16};
	double playerRot[3] = {0,0,0};
	double playerMove[3] = {0,0,0};
	int grounded = 1;
	int blocksTouching[2][3] = {0};
	int destroy = 0;
	int blockType = 0;
	
	// Frame system
	struct timespec currentFrameTime, lastFrameTime;
	clock_gettime(CLOCK_REALTIME, &lastFrameTime);
	int frameLength[60];
	double frameAverage;
	
	// Menu variables
	int menuX = 0;
	int menuY = 0;
	int menu = 0;
	int isClicked = 0;
	int running = 1;
	
	// Load world or generate new one from seed
	if (seed != -1) {
		generateTerrain(blockPositions, seed);
	} else {
		loadTerrain(blockPositions, level);
	}
	generatePolygons(blockPositions, polygons, gameCoords, blockColors);
	
	// GAME LOOP
	while (running) {
		
		clock_gettime(CLOCK_REALTIME, &currentFrameTime);
		for (int i = 1; i < 60; i++) {
			frameLength[i-1] = frameLength[i];
		}
		frameLength[59] = currentFrameTime.tv_nsec - lastFrameTime.tv_nsec + (currentFrameTime.tv_sec - lastFrameTime.tv_sec)*1000000000;
		deltaTime = ((double)(currentFrameTime.tv_nsec - lastFrameTime.tv_nsec + (currentFrameTime.tv_sec - lastFrameTime.tv_sec)*1000000000)) / 1000000000 * 60;
		clock_gettime(CLOCK_REALTIME, &lastFrameTime);
		
		if (menu == 0) {
			getGameInputs(playerMove, playerRot, &menu, &grounded, &destroy, &blockType);
			grounded = checkCollisions(playerPos, playerMove, blockPositions);
			if (destroy != 0) {
				editBlock(blockPositions, blocksTouching, blockType, destroy);
				for (int i = 0; i < POLYGONS; i++) {
					polygons[i][1] = -1;
				}
				generatePolygons(blockPositions, polygons, gameCoords, blockColors);
			}
			
			playerTouching(playerPos, playerRot, blockPositions, blocksTouching);
		
			convertScreen(gameCoords, screenCoords, playerPos, playerRot);
		
			cullBack(polygons, screenCoords, drawOrder);
			orderPoly(polygons, screenCoords, drawOrder);
		}
		if (menu == 1) {
			isClicked = getMenuInputs(&menuX, &menuY, &menu);
			if (isClicked && menuX == 2) running = 0;
			if (isClicked && menuX == 1) saveWorld(blockPositions, level);
		}
		
		erase();
		
		drawAll(polygons, screenCoords, drawOrder, gameCoords);
		drawInventory(blockColors, blockType);
		
		if (menu == 1) drawPaused(menuX, menuY);
		
		mvprintw(0, 0, "X,Y,Z: %.2lf, %.2lf, %.2lf", playerPos[0]/2, (playerPos[1]-5.2)/2+1, playerPos[2]/2);
		mvprintw(1, 0, "Mouse: %d, %d, %d", blocksTouching[1][0], blocksTouching[1][1], blocksTouching[1][2]);
		
		frameAverage = 0;
		for (int i = 0; i < 60; i++) {
			frameAverage += frameLength[i];
		}
		frameAverage /= 60;
		if (frameAverage != 0) mvprintw(0,COLS-8,"%4d FPS",(int)(1000000000/((double)frameAverage)));
		else mvprintw(0,COLS-8,"   0 FPS");
		
		refresh();
	}
	
	endwin();
	return 0;
}

// ==================================================> FUNCTIONS <==================================================

// Converts game coordinates into screen coordinates
void convertScreen(int gameCoords[VERTICIES][3], double screenCoords[VERTICIES][3], double playerPos[3], double playerRot[3]) {
	
	double tempScreenCoords[3];
	double matrixConversions[5][6] = {
		{-playerPos[0], 0, -playerPos[1], 0,  -playerPos[2], 0},
		{cos(playerRot[2] / 180.0 * M_PI), -sin(playerRot[2] / 180.0 * M_PI), sin(playerRot[2] / 180.0 * M_PI), cos(playerRot[2] / 180.0 * M_PI), 1, 0},
		{cos(playerRot[1] / 180.0 * M_PI), sin(playerRot[1] / 180.0 * M_PI), 1, 0, -sin(playerRot[1] / 180.0 * M_PI), cos(playerRot[1] / 180.0 * M_PI)},
		{1, 0, cos(playerRot[0] / 180.0 * M_PI), -sin(playerRot[0] / 180.0 * M_PI), sin(playerRot[0] / 180.0 * M_PI), cos(playerRot[0] / 180.0 * M_PI)},
		{atan(M_PI/4), 0, atan(M_PI/1.2), 0, 0, 0}};
	
	for (int i = 0; i < VERTICIES; i++) {
		if (gameCoords[i][1] == -1) {
			continue;
		}
		
		for (int j = 0; j < 3; j++) {
			screenCoords[i][j] = gameCoords[i][j];
		}
		
		//First matrix - Translations
		screenCoords[i][0] += matrixConversions[0][0];
		screenCoords[i][1] += matrixConversions[0][2];
		screenCoords[i][2] += matrixConversions[0][4];
		
		//Second matrix - Rotation Z-Axis
		tempScreenCoords[0] = matrixConversions[1][0] * screenCoords[i][0] + matrixConversions[1][1] * screenCoords[i][1];
		tempScreenCoords[1] = matrixConversions[1][2] * screenCoords[i][0] + matrixConversions[1][3] * screenCoords[i][1];
		tempScreenCoords[2] = screenCoords[i][2];
		
		//Third matrix - Rotation Y-Axis
		screenCoords[i][0] = matrixConversions[2][0] * tempScreenCoords[0] + matrixConversions[2][1] * tempScreenCoords[2];
		screenCoords[i][1] = tempScreenCoords[1];
		screenCoords[i][2] = matrixConversions[2][4] * tempScreenCoords[0] + matrixConversions[2][5] * tempScreenCoords[2];
		
		//Fourth matrix - Rotation X-Axis
		tempScreenCoords[0] = screenCoords[i][0];
		tempScreenCoords[1] = matrixConversions[3][2] * screenCoords[i][1] + matrixConversions[3][3] * screenCoords[i][2];
		tempScreenCoords[2] = matrixConversions[3][4] * screenCoords[i][1] + matrixConversions[3][5] * screenCoords[i][2];
		
		//Fifth matrix - Scaling to screen
		screenCoords[i][0] = matrixConversions[4][0] * tempScreenCoords[0];
		screenCoords[i][1] = matrixConversions[4][2] * tempScreenCoords[1];
		screenCoords[i][2] = tempScreenCoords[2];
		
		//Sixth matrix - Perspective
		if (screenCoords[i][2] > 0.01) {
			screenCoords[i][0] /= screenCoords[i][2];
			screenCoords[i][1] /= screenCoords[i][2];
		} else {
			screenCoords[i][0] *= 100;
			screenCoords[i][1] *= 100;
		}
	}
		
		return;
}

// Fills the interior of the polygon
void fillPolygon(double polygon[3][3], int color, char draw) {
	
	//is polygon on screen
	int isVisible = 0;
	for (int i = 0; i < 3; i++) {
		if (polygon[i][2] > 0) isVisible = 1;
	}
	
	if (!isVisible) return;
	
	//finding min and max x/y coords
	double minX = polygon[0][0] * (COLS / 2);
	double maxX = polygon[0][0] * (COLS / 2);
	double minY = polygon[0][1] * (LINES / 2);
	double maxY = polygon[0][1] * (LINES / 2);
	for (int i = 1; i < 3; i++) {
		if (minX > polygon[i][0] * (COLS / 2)) minX = polygon[i][0] * (COLS / 2);
		else if (maxX < polygon[i][0] * (COLS / 2)) maxX = polygon[i][0] * (COLS / 2);
		if (minY > polygon[i][1] * (LINES / 2)) minY = polygon[i][1] * (LINES / 2);
		else if (maxY < polygon[i][1] * (LINES / 2)) maxY = polygon[i][1] * (LINES / 2);
	}
	if (maxY > LINES / 2 - 1) maxY = LINES / 2 - 1;
	if (minY < -LINES / 2) minY = -LINES / 2;
	if (maxX > COLS / 2) maxX = COLS / 2;
	if (minX < -COLS / 2 + 1) minX = -COLS / 2 + 1;
	
	//checking positions for being inside the triangle
	double poly[3][2];
	for (int i = 0; i < 3; i++) {
		poly[i][0] = polygon[i][0] * (COLS / 2);
		poly[i][1] = polygon[i][1] * (LINES / 2);
	}
	
	// if the polygon has no characters on it
	if (color > 0) {
		attron(COLOR_PAIR(color));
		for (int i = (int)minX; i <= (int)maxX; i++) {
			for (int j = (int)minY; j <= (int)maxY; j++) {
				if (isInside(poly, i, j) == 1) {
					move(-j + LINES / 2 - 1, i + COLS / 2 - 1);
					addch(' ');
				}
			}
		}
		attroff(COLOR_PAIR(color));
		
	// if the polygon should have characters
	} else {
		attron(COLOR_PAIR(-color));
		for (int i = (int)minX; i <= (int)maxX; i++) {
			for (int j = (int)minY; j <= (int)maxY; j++) {
				if (isInside(poly, i, j) == 1) {
					move(-j + LINES / 2 - 1, i + COLS / 2 - 1);
					addch(draw);
				}
			}
		}
		attroff(COLOR_PAIR(-color));
	}
	return;
}

// Returns if point x y is inside the triangle
int isInside(double poly[3][2], int pointX, int pointY) {
	// Calculate area of the triangle
	double area = abs((poly[0][0] * (poly[1][1] - poly[2][1]) + poly[1][0] * (poly[2][1] - poly[0][1]) + poly[2][0] * (poly[0][1] - poly[1][1]))/2.0);
	// Calculate areas of three triangles using the point that we are checking for
	double a1 = abs((pointX * (poly[1][1] - poly[2][1]) + poly[1][0] * (poly[2][1] - pointY) + poly[2][0] * (pointY - poly[1][1]))/2.0);
	double a2 = abs((poly[0][0] * (pointY - poly[2][1]) + pointX * (poly[2][1] - poly[0][1]) + poly[2][0] * (poly[0][1] - pointY))/2.0);
	double a3 = abs((poly[0][0] * (poly[1][1] - pointY) + poly[1][0] * (pointY - poly[0][1]) + pointX * (poly[0][1] - poly[1][1]))/2.0);
	
	// compare the area of the whole triangle to the area of the three triangle parts
	if (area - (a1 + a2 + a3) >= 0) return 1;
	
	return 0;
}

// Gets input from player while in the game
void getGameInputs(double playerMove[], double playerRot[], int* menu, int* grounded, int* destroy, int* blockType) {
	int ch = getch();
	*destroy = 0;
	
	// Checks which key the player is pressing and then changes some values based on what the key should do
	switch (ch) {
		case KEY_LEFT:
		playerMove[0] -= cos(playerRot[1]/180*M_PI) * 2;
		playerMove[2] -= sin(playerRot[1]/180*M_PI) * 2;
		break;
		
		case KEY_RIGHT:
		playerMove[0] += cos(playerRot[1]/180*M_PI) * 2;
		playerMove[2] += sin(playerRot[1]/180*M_PI) * 2;
		break;
		
		case KEY_DOWN:
		playerMove[2] -= cos(playerRot[1]/180*M_PI) * 2;
		playerMove[0] += sin(playerRot[1]/180*M_PI) * 2;
		break;
		
		case KEY_UP:
		playerMove[2] += cos(playerRot[1]/180*M_PI) * 2;
		playerMove[0] -= sin(playerRot[1]/180*M_PI) * 2;
		break;
		
		case 's':
		if (playerRot[0] > -90.0) playerRot[0] -= 40 * deltaTime;
		else playerRot[0] = -90.0;
		break;
		
		case 'w':
		if (playerRot[0] < 90.0) playerRot[0] += 40 * deltaTime;
		else playerRot[0] = 90.0;
		break;
		
		case 'a':
		if (playerRot[1] < 180.0) playerRot[1] += 40 * deltaTime;
		else playerRot[1] = -180.0;
		break;
		
		case 'd':
		if (playerRot[1] > -180.0) playerRot[1] -= 40 * deltaTime;
		else playerRot[1] = 180.0;
		break;
		
		case 'z':
		*destroy = -1;
		break;
		
		case 'x':
		*destroy = 1;
		break;
		
		case 'q':
		if (*blockType > 0)
		*blockType -= 1;
		break;
		
		case 'e':
		if (*blockType < NUM_BLOCKS)
		*blockType += 1;
		break;
		
		case ' ':
		if (*grounded) {
			playerMove[1] = 0.3;
		}
		break;
		
		case 27:
		*menu = 1;
		break;
	}
}

// Gets input from player while in the menus
int getMenuInputs(int* menuX, int* menuY, int* menu) {
	int ch = getch();
	
	// Does the same thing as game inputs just with the menu
	switch (ch) {
		case KEY_LEFT:
		if (*menuX > 0) *menuX -= 1;
		break;
		
		case KEY_RIGHT:
		if (*menuX < 2) *menuX += 1;
		break;
		
		case KEY_DOWN:
		if (*menuY < 2) *menuY += 1;
		break;
		
		case KEY_UP:
		if (*menuY > 0) *menuY -= 1;
		break;
		
		case '\n':
		return 1;
		break;
		
		case 27:
		*menu = 0;
		break;
	}
	return 0;
}

// Generates terrain from the seed provided
void generateTerrain(int blockPositions[WORLD_SIZE][WORLD_SIZE][WORLD_SIZE], int seed) {
	// Clear array
	for (int x = 0; x < WORLD_SIZE; x++) {
		for (int y = 0; y < WORLD_SIZE; y++) {
			for (int z = 0; z < WORLD_SIZE; z++) {
				blockPositions[x][y][z] = -1;
			}
		}
	}
	// Test case for a flat map with a tree
	//fill array
	if (seed == 0) {
		for (int x = 0; x < WORLD_SIZE; x++) {
			for (int y = 0; y < 5; y++) {
				for (int z = 0; z < WORLD_SIZE; z++) {
					blockPositions[x][y][z] = 2;
				}
			}
		}
		for (int x = 0; x < WORLD_SIZE; x++) {
			for (int y = 5; y < 7; y++) {
				for (int z = 0; z < WORLD_SIZE; z++) {
					blockPositions[x][y][z] = 1;
				}
			}
		}
		for (int x = 0; x < WORLD_SIZE; x++) {
			for (int z = 0; z < WORLD_SIZE; z++) {
				blockPositions[x][7][z] = 0;
			}
		}
		for (int x = 2; x < 7; x++) {
			for (int z = 6; z < 11; z++) {
				blockPositions[x][10][z] = 4;
				blockPositions[x][11][z] = 4;
			}
		}
		blockPositions[4][12][8] = 4;
		blockPositions[4][13][8] = 4;
		blockPositions[3][12][8] = 4;
		blockPositions[3][13][8] = 4;
		blockPositions[5][12][8] = 4;
		blockPositions[5][13][8] = 4;
		blockPositions[4][12][7] = 4;
		blockPositions[4][13][7] = 4;
		blockPositions[4][12][9] = 4;
		blockPositions[4][13][9] = 4;
		blockPositions[2][10][6] = -1;
		blockPositions[6][11][10] = -1;
		blockPositions[4][8][8] = 3;
		blockPositions[4][9][8] = 3;
		blockPositions[4][10][8] = 3;
		blockPositions[4][11][8] = 3;
		
	// random world generator for other seeds
	} else {
		srand(seed);
		for (int x = 0; x < WORLD_SIZE; x++) {
			for (int y = 0; y < 8; y++) {
				for (int z = 0; z < WORLD_SIZE; z++) {
					blockPositions[x][y][z] = rand()%NUM_BLOCKS+1;
				}
			}
		}
	}
	blockPositions[0][0][0] = 3;
	blockPositions[1][0][0] = 4;
}

// Loads terrain from a file character by character converting them to numbers
void loadTerrain(int blockPositions[WORLD_SIZE][WORLD_SIZE][WORLD_SIZE], FILE* level) {
	for (int x = 0; x < WORLD_SIZE; x++) {
		for (int y = 0; y < WORLD_SIZE; y++) {
			for (int z = 0; z < WORLD_SIZE; z++) {
				blockPositions[x][y][z] = ((int)fgetc(level))-33;
			}
		}
	}
}

// Saves the current world to a text file block by block converting them to characters
void saveWorld(int blockPositions[WORLD_SIZE][WORLD_SIZE][WORLD_SIZE], FILE* level) {
	level = fopen("world.txt", "w");
	for (int x = 0; x < WORLD_SIZE; x++) {
		for (int y = 0; y < WORLD_SIZE; y++) {
			for (int z = 0; z < WORLD_SIZE; z++) {
				fputc((char)(blockPositions[x][y][z]+33), level);
			}
		}
	}
}

// Assigns the terrain mesh to a group of polygons
void generatePolygons(int blockPositions[WORLD_SIZE][WORLD_SIZE][WORLD_SIZE], int polygons[POLYGONS][4], int gameCoords[VERTICIES][3], int blockColors[][3]) {
	for (int x = 0; x < WORLD_SIZE; x++) {
		for (int y = 0; y < WORLD_SIZE; y++) {
			for (int z = 0; z < WORLD_SIZE; z++) {
				if (blockPositions[x][y][z] > -1) {
					// Generating polygons on the left side of blocks
					if (x == 0 || blockPositions[x-1][y][z]  == -1) {
						addPoly(addVertex(2*x, 2*y, 2*z, gameCoords), addVertex(2*x, 2*y+2, 2*z+2, gameCoords), addVertex(2*x, 2*y+2, 2*z, gameCoords), blockColors[blockPositions[x][y][z]][1], polygons);
						addPoly(addVertex(2*x, 2*y, 2*z, gameCoords), addVertex(2*x, 2*y, 2*z+2, gameCoords), addVertex(2*x, 2*y+2, 2*z+2, gameCoords), blockColors[blockPositions[x][y][z]][1], polygons);
					}
					// Generating polygons on the right side of blocks
					if (x == 15 || blockPositions[x+1][y][z]  == -1) {
						addPoly(addVertex(2*x+2, 2*y, 2*z, gameCoords), addVertex(2*x+2, 2*y+2, 2*z, gameCoords), addVertex(2*x+2, 2*y+2, 2*z+2, gameCoords), blockColors[blockPositions[x][y][z]][1], polygons);
						addPoly(addVertex(2*x+2, 2*y, 2*z, gameCoords), addVertex(2*x+2, 2*y+2, 2*z+2, gameCoords), addVertex(2*x+2, 2*y, 2*z+2, gameCoords), blockColors[blockPositions[x][y][z]][1], polygons);
					}
					// Generating polygons on the bottom side of blocks
					if (y == 0 || blockPositions[x][y-1][z]  == -1) {
						addPoly(addVertex(2*x, 2*y, 2*z, gameCoords), addVertex(2*x+2, 2*y, 2*z+2, gameCoords), addVertex(2*x, 2*y, 2*z+2, gameCoords), blockColors[blockPositions[x][y][z]][2], polygons);
						addPoly(addVertex(2*x, 2*y, 2*z, gameCoords), addVertex(2*x+2, 2*y, 2*z, gameCoords), addVertex(2*x+2, 2*y, 2*z+2, gameCoords), blockColors[blockPositions[x][y][z]][2], polygons);
					}
					// Generating polygons on the top side of blocks
					if (y == 15 || blockPositions[x][y+1][z]  == -1) {
						addPoly(addVertex(2*x, 2*y+2, 2*z, gameCoords), addVertex(2*x, 2*y+2, 2*z+2, gameCoords), addVertex(2*x+2, 2*y+2, 2*z+2, gameCoords), blockColors[blockPositions[x][y][z]][0], polygons);
						addPoly(addVertex(2*x, 2*y+2, 2*z, gameCoords), addVertex(2*x+2, 2*y+2, 2*z+2, gameCoords), addVertex(2*x+2, 2*y+2, 2*z, gameCoords), blockColors[blockPositions[x][y][z]][0], polygons);
					}
					// Generating polygons on the front side of blocks
					if (z == 0 || blockPositions[x][y][z-1]  == -1) {
						addPoly(addVertex(2*x, 2*y, 2*z, gameCoords), addVertex(2*x, 2*y+2, 2*z, gameCoords), addVertex(2*x+2, 2*y+2, 2*z, gameCoords), blockColors[blockPositions[x][y][z]][1], polygons);
						addPoly(addVertex(2*x, 2*y, 2*z, gameCoords), addVertex(2*x+2, 2*y+2, 2*z, gameCoords), addVertex(2*x+2, 2*y, 2*z, gameCoords), blockColors[blockPositions[x][y][z]][1], polygons);
					}
					// Generating polygons on the back side of blocks
					if (z == 15 || blockPositions[x][y][z+1]  == -1) {
						addPoly(addVertex(2*x, 2*y, 2*z+2, gameCoords), addVertex(2*x+2, 2*y+2, 2*z+2, gameCoords), addVertex(2*x, 2*y+2, 2*z+2, gameCoords), blockColors[blockPositions[x][y][z]][1], polygons);
						addPoly(addVertex(2*x, 2*y, 2*z+2, gameCoords), addVertex(2*x+2, 2*y, 2*z+2, gameCoords), addVertex(2*x+2, 2*y+2, 2*z+2, gameCoords), blockColors[blockPositions[x][y][z]][1], polygons);
					}
				}
			}
		}
	}
}

// Adds a vertex to the vertex list and returns the position of that vertex in the list
int addVertex(int x, int y, int z, int vertexList[VERTICIES][3]) {
	// checks if a vertex is in the list already
	for (int i = 0; i < VERTICIES; i++) {
		if (vertexList[i][0] == x && vertexList[i][1] == y && vertexList[i][2] == z) return i;
	}
	
	// if not find the first open position in the list and fill it with the coordinates
	for (int i = 0; i < VERTICIES; i++) {
		if (vertexList[i][1] == -1) {
			vertexList[i][0] = x;
			vertexList[i][1] = y;
			vertexList[i][2] = z;
			return i;
		}
	}
	return -1;
}

// Adds a polygon to the list
void addPoly(int vert1, int vert2, int vert3, int color, int polyList[POLYGONS][4]) {
	// checks if the polygon already exists in the array
	for (int i = 0; i < POLYGONS; i++) {
		if (polyList[i][0] == vert1 && polyList[i][1] == vert2 && polyList[i][2] == vert3) return;
	}
	
	// if not add it in the first open space
	for (int i = 0; i < POLYGONS; i++) {
		if (polyList[i][1] == -1) {
			polyList[i][0] = vert1;
			polyList[i][1] = vert2;
			polyList[i][2] = vert3;
			polyList[i][3] = color;
			return;
		}
	}
}

// Draws all of the polygons to the screen
void drawAll(int allPolygons[POLYGONS][4], double screenCoords[VERTICIES][3], int drawOrder[POLYGONS], int gameCoords[VERTICIES][3]) {
	double polygon[3][3];
	int num = 0;
	for (int i = 0; i < POLYGONS; i++) {
		if (drawOrder[i] == -1) break;
		if (allPolygons[drawOrder[i]][1] != -1) {
			// use polygon
			for (int j = 0; j < 3; j++) {
				for (int k = 0; k < 3; k++) {
					polygon[j][k] = screenCoords[allPolygons[drawOrder[i]][j]][k];
				}
			}
			// detect polygon facing and change the text type
			if (gameCoords[allPolygons[drawOrder[i]][0]][0] == gameCoords[allPolygons[drawOrder[i]][1]][0] && gameCoords[allPolygons[drawOrder[i]][1]][0] == gameCoords[allPolygons[drawOrder[i]][2]][0]) {
				fillPolygon(polygon, allPolygons[drawOrder[i]][3], '@');
				num++;
			} else if (gameCoords[allPolygons[drawOrder[i]][0]][1] == gameCoords[allPolygons[drawOrder[i]][1]][1] && gameCoords[allPolygons[drawOrder[i]][1]][1] == gameCoords[allPolygons[drawOrder[i]][2]][1]) {
				fillPolygon(polygon, allPolygons[drawOrder[i]][3], '#');
				num++;
			} else {
				fillPolygon(polygon, allPolygons[drawOrder[i]][3], '$');
				num++;
			}
		}
	}
	attron(COLOR_PAIR(15));
	move(LINES / 2, COLS / 2);
	addch('+');
	attroff(COLOR_PAIR(15));
}

// Removes polygons that are facing away from the screen
void cullBack(int polygons[POLYGONS][4], double screenCoords[VERTICIES][3], int drawOrder[POLYGONS]) {
	for (int i = 0; i < POLYGONS; i++) {
		drawOrder[i] = -1;
	}
	int numFacing = 0;
	for (int i = 0; i < POLYGONS; i++) {
		// if polygon is facing towards the screen then add it to the list to be drawn
		if (polygons[i][1] != -1)
		if (((screenCoords[polygons[i][1]][0] - screenCoords[polygons[i][0]][0]) * (screenCoords[polygons[i][2]][1] - screenCoords[polygons[i][0]][1])) - ((screenCoords[polygons[i][1]][1] - screenCoords[polygons[i][0]][1]) * (screenCoords[polygons[i][2]][0] - screenCoords[polygons[i][0]][0])) < 0) {
			drawOrder[numFacing] = i;
			numFacing++;
		}
	}
}

// Orders the polygons so they are drawn back to front
void orderPoly(int polygons[POLYGONS][4], double screenCoords[VERTICIES][3], int drawOrder[POLYGONS]) {
	double zDistance[POLYGONS];
	for (int i = 0; i < POLYGONS; i++) {
		if (drawOrder[i] == -1) break;
		zDistance[i] = (screenCoords[polygons[drawOrder[i]][0]][2] + screenCoords[polygons[drawOrder[i]][1]][2] + screenCoords[polygons[drawOrder[i]][2]][2]) / 3;
	}
	
	// bubble sort to order the polygons from back to front
	int tempPoly;
	double tempDistance;
	for (int i = 1; i < POLYGONS; i++) {
		if (drawOrder[i] == -1) break;
		for (int j = i; j > 0; j--) {
			if (zDistance[j] > zDistance[j-1]) {
				tempDistance = zDistance[j];
				zDistance[j] = zDistance[j-1];
				zDistance[j-1] = tempDistance;
				
				tempPoly = drawOrder[j];
				drawOrder[j] = drawOrder[j-1];
				drawOrder[j-1] = tempPoly;
			} else {
				break;
			}
		}
	}
}

// Checks if the player is inside a block and pushes them out of it. returns if the player is grounded
int checkCollisions(double playerPos[3], double playerMove[3], int blockPositions[WORLD_SIZE][WORLD_SIZE][WORLD_SIZE]) {
	int onGround = 1;
	int collided = 0;
	double collisionPoints[12][3];
	// twelve points on the player that are checked for collisions
	double collisionOffset[12][3] = {
		{0.299, -1.599, 0.299}, 
		{-0.299, -1.599, 0.299}, 
		{0.299, -1.599, -0.299}, 
		{-0.299, -1.599, -0.299}, 
		{0.299, -0.7, 0.299}, 
		{-0.299, -0.7, 0.299}, 
		{0.299, -0.7, -0.299}, 
		{-0.299, -0.7, -0.299}, 
		{0.299, 0.199, 0.299}, 
		{-0.299, 0.199, 0.299}, 
		{0.299, 0.199, -0.299}, 
		{-0.299, 0.199, -0.299}
	};
	
	// moving those twelve points to be relative to the player
	for (int i = 0; i < 12; i++) {
		collisionPoints[i][0] = playerPos[0] / 2 + collisionOffset[i][0];
		collisionPoints[i][1] = playerPos[1] / 2 + collisionOffset[i][1];
		collisionPoints[i][2] = playerPos[2] / 2 + collisionOffset[i][2];
	}
	
	// checks player collision in the y direction
	// if player is moving down
	if (playerMove[1] < 0) {
		for (int i = 0; i < 4; i++) {
			if (blockPositions[(int)(collisionPoints[i][0])][(int)(collisionPoints[i][1]+playerMove[1]/2*deltaTime)][(int)(collisionPoints[i][2])] != -1) {
				collided = 1;
			}
		}
		if (collided) {
			playerPos[1] = (int)(collisionPoints[0][1]+playerMove[1]/2*deltaTime)*2+5.2;
		} else {
			playerPos[1] += playerMove[1] * deltaTime;
			onGround = 0;
		}
	// if player is moving up
	} else if (playerMove[1] > 0) {
		for (int i = 8; i < 12; i++) {
			if (blockPositions[(int)(collisionPoints[i][0])][(int)(collisionPoints[i][1]+playerMove[1]/2*deltaTime)][(int)(collisionPoints[i][2])] != -1) {
				collided = 1;
			}
		}
		if (collided) {
			playerPos[1] = (int)(collisionPoints[8][1]+playerMove[1]/2*deltaTime)*2-0.4;
			playerMove[1] = 0;
		} else playerPos[1] += playerMove[1] * deltaTime;
		onGround = 0;
	} else {
		for (int i = 0; i < 4; i++) {
			if (blockPositions[(int)(collisionPoints[i][0])][(int)(collisionPoints[i][1]-0.5)][(int)(collisionPoints[i][2])] != -1) {
				collided = 1;
			}
		}
		if (!collided) onGround = 0;
	}
	
	collided = 0;
	// checks player collision in the x direction
	if (playerMove[0] > 0) {
		for (int i = 0; i < 12; i++) {
			if (blockPositions[(int)(collisionPoints[i][0]+playerMove[0]/2*deltaTime)][(int)(collisionPoints[i][1])][(int)(collisionPoints[i][2])] != -1) {
				collided = 1;
			}
		}
		if (collided) {
			playerPos[0] = (int)(collisionPoints[0][0]+playerMove[0]/2*deltaTime)*2-0.6;
		} else {
			playerPos[0] += playerMove[0] * deltaTime;
		}
	} else if (playerMove[0] < 0) {
		for (int i = 0; i < 12; i++) {
			if (blockPositions[(int)(collisionPoints[i][0]+playerMove[0]/2*deltaTime)][(int)(collisionPoints[i][1])][(int)(collisionPoints[i][2])] != -1) {
				collided = 1;
			}
		}
		if (collided) {
			playerPos[0] = (int)(collisionPoints[0][0]+playerMove[0]/2*deltaTime)*2+0.6;
		} else {
			playerPos[0] += playerMove[0] * deltaTime;
		}
	}
	
	collided = 0;
	// checks player collision in the z direction
	if (playerMove[2] > 0) {
		for (int i = 0; i < 12; i++) {
			if (blockPositions[(int)(collisionPoints[i][0])][(int)(collisionPoints[i][1])][(int)(collisionPoints[i][2]+playerMove[2]/2*deltaTime)] != -1) {
				collided = 1;
			}
		}
		if (collided) {
			playerPos[2] = (int)(collisionPoints[0][2]+playerMove[2]/2*deltaTime)*2-0.6;
		} else {
			playerPos[2] += playerMove[2] * deltaTime;
		}
	} else if (playerMove[2] < 0) {
		for (int i = 0; i < 12; i++) {
			if (blockPositions[(int)(collisionPoints[i][0])][(int)(collisionPoints[i][1])][(int)(collisionPoints[i][2]+playerMove[2]/2*deltaTime)] != -1) {
				collided = 1;
			}
		}
		if (collided) {
			playerPos[2] = (int)(collisionPoints[0][2]+playerMove[2]/2*deltaTime)*2+0.6;
		} else {
			playerPos[2] += playerMove[2] * deltaTime;
		}
	}
	
	// resets the player movement and makes the player fall if needed
	playerMove[0] = 0;
	if (!onGround && playerMove[1] > -0.9) {
		playerMove[1] -= 0.02 * deltaTime;
	} else if (onGround) {
		playerMove[1] = 0;
	}
	playerMove[2] = 0;
	
	return onGround;
}

// Draws the pause menu
void drawPaused(int menuX, int menuY) {
	attron(COLOR_PAIR(16));
	for (int x = 30; x < COLS-30; x++) {
		for (int y = 10; y < LINES-10; y++) {
			move(y, x);
			addch(' ');
		}
	}
	// draws game paused text and game controls
	move(13, COLS/2-5);
	printw("GAME PAUSED");
	mvaddstr(15, COLS/2-18, "MOVE: ARROW KEYS    CAMERA: WASD");
	mvaddstr(17, COLS/2-18, "PLACE/BREAK: Z/X    CHANGE BLOCK: Q/E");
	
	// draws the menu icons and inverts the colors on the one that is selected
	if (menuX == 0) attron(COLOR_PAIR(15));
	move(LINES/2+2, COLS/2-30);
	printw("+--------------+");
	move(LINES/2+3, COLS/2-30);
	printw("|     MENU     |");
	move(LINES/2+4, COLS/2-30);
	printw("+--------------+");
	attron(COLOR_PAIR(16));
	
	if (menuX == 1) attron(COLOR_PAIR(15));
	move(LINES/2+2, COLS/2-8);
	printw("+--------------+");
	move(LINES/2+3, COLS/2-8);
	printw("|     SAVE     |");
	move(LINES/2+4, COLS/2-8);
	printw("+--------------+");
	attron(COLOR_PAIR(16));
	
	if (menuX == 2) attron(COLOR_PAIR(15));
	move(LINES/2+2, COLS/2+14);
	printw("+--------------+");
	move(LINES/2+3, COLS/2+14);
	printw("|     QUIT     |");
	move(LINES/2+4, COLS/2+14);
	printw("+--------------+");
	attron(COLOR_PAIR(16));
	
	attroff(COLOR_PAIR(16));
}

// Casts a ray and detects the coordinates of the first block it reaches and the space in front of it
void playerTouching(double playerPos[], double playerRot[], int blockPositions[WORLD_SIZE][WORLD_SIZE][WORLD_SIZE], int blocksTouching[2][3]) {
	double rayPos[] = {playerPos[0], playerPos[1], playerPos[2]};
	double rayIncrement[] = {-sin(playerRot[1]/180*M_PI)*cos(playerRot[0]/180*M_PI)/10, sin(playerRot[0]/180*M_PI)/10, cos(playerRot[1]/180*M_PI)*cos(playerRot[0]/180*M_PI)/10};
	
	// resets the blocksTouching variable
	for (int i = 0; i < 3; i++) {
		blocksTouching[0][i] = -1;
		blocksTouching[1][i] = -1;
	}
	
	// increments the ray checking if it hits a block each time
	for (int i = 0; i < 100; i++) {
		for (int j = 0; j < 3; j++) {
			rayPos[j] += rayIncrement[j];
			if (rayPos[j] < 0 || rayPos[j] > WORLD_SIZE*2+2) return;
		}
		if (blockPositions[(int)(rayPos[0]/2)][(int)(rayPos[1]/2)][(int)(rayPos[2]/2)] != -1) {
			blocksTouching[1][0] = (int)(rayPos[0]/2);
			blocksTouching[1][1] = (int)(rayPos[1]/2);
			blocksTouching[1][2] = (int)(rayPos[2]/2);
			return;
		} else {
			blocksTouching[0][0] = (int)(rayPos[0]/2);
			blocksTouching[0][1] = (int)(rayPos[1]/2);
			blocksTouching[0][2] = (int)(rayPos[2]/2);
		}
	}
}

// Places or breaks the block the player is currently looking at
void editBlock(int blockPositions[WORLD_SIZE][WORLD_SIZE][WORLD_SIZE], int blocksTouching[2][3], int blockType, int destroy) {
	if (blocksTouching[1][0] != -1) {
		if (destroy == 1) {
			blockPositions[blocksTouching[1][0]][blocksTouching[1][1]][blocksTouching[1][2]] = -1;
		} else {
			blockPositions[blocksTouching[0][0]][blocksTouching[0][1]][blocksTouching[0][2]] = blockType;
		}
		
	}
}

// Draws the block the player is currently holding
void drawInventory(int blockColors[][3], int blockType) {
	// draws the box around the block you are currently holding
	attron(COLOR_PAIR(15));
	mvaddstr(LINES-1, COLS/2-4, "+------+");
	for (int y = 0; y < 3; y++) {
		mvaddstr(LINES-y-2, COLS/2-4, "|      |");
	}
	mvaddstr(LINES-5, COLS/2-4, "+------+");
	attroff(COLOR_PAIR(15));
	
	// draws the block with the correct colors as a 4x3 pixel grid
	if (blockColors[blockType][0] > 0) {
		attron(COLOR_PAIR(blockColors[blockType][0]));
		mvaddstr(LINES-4, COLS/2-2, "    ");
		attroff(COLOR_PAIR(blockColors[blockType][0]));
	} else {
		attron(COLOR_PAIR(-blockColors[blockType][0]));
		mvaddstr(LINES-4, COLS/2-2, "####");
		attroff(COLOR_PAIR(blockColors[blockType][0]));
	}
	if (blockColors[blockType][1] > 0) {
		attron(COLOR_PAIR(blockColors[blockType][1]));
		mvaddstr(LINES-3, COLS/2-2, "    ");
		mvaddstr(LINES-2, COLS/2-2, "    ");
		attroff(COLOR_PAIR(blockColors[blockType][1]));
	} else {
		attron(COLOR_PAIR(-blockColors[blockType][1]));
		mvaddstr(LINES-3, COLS/2-2, "$$@@");
		mvaddstr(LINES-2, COLS/2-2, "$$@@");
		attroff(COLOR_PAIR(blockColors[blockType][1]));
	}
}


//============


// Function to set terminal text color
void setColor(const char *color) {
    printf("%s", color);
}
 
// Function to reset terminal color
void resetColor() {
    printf("\033[0m"); // Reset to default
}
 
// Function for the welcome screen
void welcomeScreen() {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
 
    setColor("\033[1;32m"); // Green color
    printf("\n====================================\n");
    printf("      WELCOME TO TERMINALCRAFT      \n");
    printf("====================================\n");
    resetColor();
 
    // Display the current date and time
    printf("Current Date: %d/%d/%d\n", t->tm_mday, t->tm_mon + 1, t->tm_year + 1900);
    printf("Current Time: %d:%d:%d\n\n", t->tm_hour, t->tm_min, t->tm_sec);
 
    printf("Loading...\n");
    for (int i = 0; i < 5; i++) {
        printf(".");
        fflush(stdout); // Force the output
        sleep(1);
    }
    printf("\n");
}

// Function to display the menu
void displayMenu() {
      system("clear");
    setColor("\033[1;34m"); // Blue color for the menu title
    printf("\n====================================\n");
    printf("          TERMINALCRAFT MENU         \n");
    printf("====================================\n");
    resetColor();
    setColor("\033[1;33m"); // Yellow for the options
    printf("[1] Play\n");
    printf("[2] Load\n");
    printf("[3] Quit\n");
    resetColor();
    printf("====================================\n");
    printf("Enter your choice: ");
}
 
// Function to get a validated menu choice
int getValidatedChoice() {
    int choice;
    while (1) {
        if (scanf("%d", &choice) == 1 && (choice >= 1 && choice <= 3)) {
            while (getchar() != '\n'); // Clear input buffer
            return choice;            // Valid choice
        } else {
            printf("Invalid input. Please enter a number between 1 and 3: ");
            while (getchar() != '\n'); // Clear invalid input
        }
    }
}
 
// Function for the Play menu
int playMenu() {
    int seed;
    setColor("\033[1;32m"); // Green color for play menu
    printf("\n===== Play Menu =====\n");
    resetColor();
    printf("Enter a seed number to generate the world: ");
   
    // Read the seed number
    if (scanf("%d", &seed) == 1) {
        // Clear the input buffer
        while (getchar() != '\n');
        printf("World generated with seed: %d\n", seed);
		return seed;
    } else {
        // Handle invalid input
        printf("Invalid input. Returning to main menu.\n");
        while (getchar() != '\n'); // Clear invalid input
    }
	return -1;
}

// Function for the Load menu
void loadMenu(FILE** level) {
    char fileName[100];
    setColor("\033[1;36m"); // Cyan color for load menu
    printf("\n===== Load Menu =====\n");
    resetColor();
    printf("Enter the file name to load: ");
    fgets(fileName, sizeof(fileName), stdin);
    fileName[strcspn(fileName, "\n")] = '\0'; // Remove trailing newline
    printf("Loading world from file: %s\n", fileName);
    // Add file validation or loading logic if required
	*level = fopen(fileName, "r");
}
 
// Function for quitting the program
void quitMenu() {
    setColor("\033[1;31m"); // Red color for the goodbye message
    printf("\n====================================\n");
    printf("   THANK YOU FOR PLAYING TERMINALCRAFT\n");
    printf("====================================\n");
    resetColor();
    printf("\nExiting...\n");
    sleep(2); // Pause for effect
}
 
 
void drawGraphicalMenu() {
      printf("DEBUG: drawGraphicalMenu() is running...\n"); // Debugging message
 
    // Clear the screen
    system("clear");
 
    // Draw the sky with animated stars
    setColor("\033[1;34m"); // Sky blue
    for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 40; j++) {
			if (rand() % 10 < 2) { // Randomly place stars
				setColor("\033[1;37m"); // White for stars
				printf("*");
			} else {
				setColor("\033[1;34m"); // Blue for the sky
				printf("#");
			}
		}
		printf("\n");
	}
	resetColor();
 
 
      // Draw trees with trunks
      setColor("\033[1;32m"); // Green leaves
      printf("      /\\              /\\              /\\\n");
      printf("     /  \\            /  \\            /  \\\n");
      printf("    /____\\          /____\\          /____\\\n");
      resetColor();
      
      setColor("\033[0;33m"); // Brown trunks
      printf("      ||              ||              ||\n");
      printf("      ||              ||              ||\n");
      resetColor();
      
 
    // Draw ground
    setColor("\033[0;33m"); // Brown for ground
    printf("###########################################\n");
    resetColor();
 
    // Draw the title
    setColor("\033[1;37m"); // White
    printf("\n");
    printf("           TERMINAL CRAFT\n");
    printf("===========================================\n");
    resetColor();
 
    // Draw the menu options
    setColor("\033[1;30m"); // Black border
    printf("          +-----------------+\n");
    printf("          |      PLAY       |\n");
    printf("          +-----------------+\n");
    printf("          +-----------------+\n");
    printf("          |      LOAD       |\n");
    printf("          +-----------------+\n");
    printf("          +-----------------+\n");
    printf("          |      QUIT       |\n");
    printf("          +-----------------+\n");
    resetColor();
 
    // Prompt for input
    printf("\nEnter your choice: ");
}