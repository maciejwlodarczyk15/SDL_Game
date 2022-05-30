// Included libraries
#include <iostream>
#include <SDL_image.h>
#include <SDL.h>
#include <fstream>
#include <string>

/*
		Declarations
*/

SDL_Texture* loadTexture(std::string path);

// Displayed window size
const int scr_w = 1280;
const int scr_h = 720;

// Size of level
const int lvl_w = 2000;
const int lvl_h = 2000;

// Tile properties
const int tile_w = 80;
const int tile_h = 80;
const int tile_count = 626;
const int tile_sprites = 10;

// Declaration of objects needed to initialize the game
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

// Textures for players
SDL_Texture* p1Texture = NULL;
SDL_Texture* p2Texture = NULL;

// Frees media and shuts down SDL
void close();

/*
		Initialization
*/

// Function to initialize SDL and create a window
bool initialize() {
	// Success flag
	bool success = true;

	// SDL initialization
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cout << "SDL couldn't initialize! Error code:\n" << SDL_GetError() << std::endl;
		success = false;
	}
	else {
		std::cout << "SDL has been initialized." << std::endl;

		// Creating window
		window = SDL_CreateWindow("5 - Kamera dla dwoch graczy", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, scr_w, scr_h, SDL_WINDOW_SHOWN);
		if (window == NULL) {
			std::cout << "Window couldn't have been created! Error code:\n" << SDL_GetError() << std::endl;
			success = false;
		}
		else {
			std::cout << "Window created." << std::endl;

			// Creating renderer
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (renderer == NULL) {
				std::cout << "Renderer couldn't have been created! Error code:\n" << SDL_GetError() << std::endl;
				success = false;
			}
			else {
				std::cout << "Renderer created" << std::endl;
			}
		}
	}
	return success;
}

// Load textures for player
bool loadMedia() {
	bool success = true;

	// Load textures
	std::cout << "Loading texture for 1st player\n";
	p1Texture = loadTexture("./p1.png");
	if (p1Texture == NULL) {
		std::cout << "Texture couln't be loaded!\n";
		success = false;
	}
	std::cout << "Loading texture for 2nd player\n";
	p2Texture = loadTexture("./p2.png");
	if (p2Texture == NULL) {
		std::cout << "Texture couln't be loaded!\n";
		success = false;
	}

	// Making texture half-transparent
	// Alpha strength (127/256 = 50%)
	Uint8 a = 127;
	SDL_SetTextureAlphaMod(p2Texture, a);

	return success;
}

/*
		Textures
*/

// Load texture from a specified path
SDL_Texture* loadTexture(std::string path) {
	// The texture
	SDL_Texture* texture = NULL;

	// Load image from specified path
	SDL_Surface* surface = IMG_Load(path.c_str());
	if (surface == NULL) {
		std::cout << "Image couldn't be loaded. Error code:\n" << SDL_GetError() << std::endl;
	}
	else {
		texture = SDL_CreateTextureFromSurface(renderer, surface);
		if (texture == NULL) {
			std::cout << "Texture couldn't be created. Error code:\n" << SDL_GetError() << std::endl;
		}
		// Free the memory from using surface
		SDL_FreeSurface(surface);
	}
	return texture;
}

// Main texture class
class Texture {
public:
	// Constructor
	Texture(char name, std::string path);

	// Getters
	char getName();
	std::string getPath();
	SDL_Texture* getTexture();

private:
	char textureName;
	std::string path;
	SDL_Texture* cTexture;
};

Texture::Texture(char name, std::string givenPath) {
	textureName = name;
	path = givenPath;
	cTexture = loadTexture(givenPath);
}

char Texture::getName() {
	return textureName;
}

std::string Texture::getPath() {
	return path;
}

SDL_Texture* Texture::getTexture() {
	return cTexture;
}

/*
	Tiles
	I'm using tiles to divide level to parts and then I set up objects on it like on a grid.
*/

// Main tile class
class Tile {
public:
	// Constructor
	Tile(int x, int y, char tileType);
	int x;
	int y;

	char getType();
	// Rectangle object that is used as a tile
	SDL_Rect box;

private:
	// Type of tile that defines texture
	char type;
};

Tile::Tile(int ex, int ey, char tileType) {
	x = ex;
	y = ey;

	// Location of the tile
	box.x = ex;
	box.y = ey;

	// Width and height
	box.w = tile_w;
	box.h = tile_h;

	// Type of tile (texture)
	type = tileType;
}

char Tile::getType() {
	return type;
}

// Function responsible for texturing tiles
void setTilTex(Tile* tileMap[], Texture* textureList[]) {
	std::ifstream map("./Map.txt");
	char check = ' ';
	std::string path;
	int i = 0;
	int y = 0;
	int x = 0;
	map >> check;
	while (check != '!') {
		if (check == '.') {
			x = 0;
			y += tile_h;
		}
		else {
			tileMap[i] = new Tile(x, y, check);
			x += tile_w;
			i++;
		}
		map >> check;
	}
	for (int i = 0; i < tile_sprites; i++) {
		map >> check;
		std::getline(map, path);
		path.insert(0, "./");
		textureList[i] = new Texture(check, path);
	}
	map.close();
}

void close() {
	//Destroy window	
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	window = NULL;
	renderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

int lerp(int now, int then, float percentage) {
	return (now + (then - now) * percentage);
}


/*
		Main function
*/

int main(int argc, char* args[]) {
	if (!initialize()) {
		std::cout << "Failed to initialize.\n";
	}
	else {
		Tile* tilesMap[tile_count] = { NULL };
		Texture* textureList[tile_sprites] = { NULL };
		setTilTex(tilesMap, textureList);
		if (!loadMedia()) {
			std::cout << "Failed to load media.\n";
		}
		else {
			//Camera
			SDL_Rect camera;
			int camVelX = 0;
			int camVelY = 0;
			camera.x = 0;
			camera.y = 0;
			camera.w = scr_w;
			camera.h = scr_h;

			// Size of player (it's square, so it's width and height)
			int playerSize = 100;

			// Rectangle player 1
			SDL_Rect rect;
			int rectX = 0;
			int rectY = 0;
			rect.x = 50;
			rect.y = 50;
			rect.w = playerSize;
			rect.h = playerSize;

			// Circle player 2
			SDL_Rect circle;
			circle.x = 50;
			circle.y = 50;
			circle.w = playerSize;
			circle.h = playerSize;

			//Main loop flag
			bool gameFlag = false;

			//Event handler
			SDL_Event e;

			//Mouse state
			int cur_posx = 50;
			int cur_posy = 50;

			// Distance from object to cursor
			int dist_posx;
			int dist_posy;

			// Flag
			bool p1Moving = 0;
			bool p2Moving = 0;

			// If game is on
			while (gameFlag != true) {
				// Handle events on queue (while queue not empty)
				while (SDL_PollEvent(&e) != 0) {
					// When user wants to quit game
					if (e.type == SDL_QUIT) {
						std::cout << "The application has been closed by the user." << std::endl;
						gameFlag = true;
					}
					// Pressing left mouse button reads cursor position
					else if (e.button.button == SDL_BUTTON_LEFT) {
						SDL_GetMouseState(&cur_posx, &cur_posy);
						cur_posx += camera.x - circle.w/2;
						cur_posy += camera.y - circle.h/2;
					}
				}
				p1Moving = 0;
				p2Moving = 0;
				// Chenge x pos player 2
				// Move right
				if (circle.x < cur_posx && circle.x < lvl_w - circle.w) {
					dist_posx = cur_posx - circle.x;
					if (dist_posx >= 5) {
						circle.x += 5;
						if (circle.x >= camera.x + camera.w - circle.w) {
							std::cout << "Player 2 moved camera" << '\n';
							rect.x += 5;
						}	
					}
					else {
						circle.x += 1;
						if (circle.x >= camera.x + camera.w - circle.w) {
							std::cout << "Player 2 moved camera" << '\n';
							rect.x += 1;
						}
					}
					p2Moving = 1;
					std::cout << "Player 2 made a move." << std::endl;
				}
				// Move left
				if (circle.x > cur_posx && circle.x > 0) {
					dist_posx = circle.x - cur_posx;
					if (dist_posx >= 5) {
						circle.x -= 5;
						if (circle.x <= camera.x) {
							std::cout << "Player 2 moved camera" << '\n';
							rect.x -= 5;
						}
					}
					else {
						circle.x -= 1;
						if (circle.x <= camera.x) {
							std::cout << "Player 2 moved camera" << '\n';
							rect.x -= 1;
						}
					}
					p2Moving = 1;
					std::cout << "Player 2 made a move." << std::endl;
				}
				// Change y pos player 2
				if (circle.y != cur_posy) {
					// Move down
					if (circle.y < cur_posy && circle.y < lvl_h - circle.h) {
						dist_posy = cur_posy - circle.y;
						if (dist_posy >= 5) {
							circle.y += 5;
							if (circle.y > camera.y + camera.h - circle.h) {
								std::cout << "Player 2 moved camera" << '\n';
								rect.y += 5;
							}
						}
						else {
							circle.y += 1;
							if (circle.y > camera.y + camera.h - circle.h) {
								std::cout << "Player 2 moved camera" << '\n';
								rect.y += 5;
							}
						}
						std::cout << "Player 2 made a move." << std::endl;
					}
					// Move up
					else if (circle.y > cur_posy && circle.y > 0) {
						dist_posy = circle.y - cur_posy;
						if (dist_posy >= 5) {
							circle.y -= 5;
							if (circle.y < camera.y) {
								std::cout << "Player 2 moved camera" << '\n';
								rect.y -= 5;
							}
						}
						else {
							circle.y -= 1;
							if (circle.y < camera.y) {
								std::cout << "Player 2 moved camera" << '\n';
								rect.y -= 5;
							}
						}
						std::cout << "Player 2 made a move." << std::endl;
					}
				}

				// If pressing WSAD, change position of player 1
				const Uint8* currentKeyState = SDL_GetKeyboardState(NULL);
				if (currentKeyState[SDL_SCANCODE_D] && rect.x < lvl_w - rect.w) {
					rect.x += 5;
					p1Moving = 1;
					std::cout << "Player 1 made a move." << std::endl;
					// Push second player
					if (rect.x > camera.x + camera.w - rect.w) {
						std::cout << "Player 1 moved camera" << '\n';
						cur_posx += 5;
					}
				}
				else if (currentKeyState[SDL_SCANCODE_A] && rect.x > 0) {
					rect.x -= 5;
					p1Moving = 1;
					std::cout << "Player 1 made a move." << std::endl;

					if (rect.x < camera.x) {
						std::cout << "Player 1 moved camera" << '\n';
						cur_posx -= 5;
					}
				}
				if (currentKeyState[SDL_SCANCODE_S] && rect.y < lvl_h - rect.h) {
					rect.y += 5;
					std::cout << "Player 1 made a move." << std::endl;

					if (rect.y > camera.y + camera.h - rect.h) {
						std::cout << "Player 1 moved camera" << '\n';
						cur_posy += 5;
					}
				}
				else if (currentKeyState[SDL_SCANCODE_W] && rect.y > 0) {
					rect.y -= 5;
					std::cout << "Player 1 made a move." << std::endl;

					if (rect.y < camera.y) {
						std::cout << "Player 1 moved camera" << '\n';
						cur_posy -= 5;
					}
				}

				// If two players move camera at the same time
				// Horizontal
				if (rect.x <= camera.x && circle.x >= camera.x + camera.w - circle.w && p1Moving && p2Moving) {
					std::cout << "Two players tried to move camera at the same time." << '\n';
					rect.x = camera.x;
					circle.x = camera.x + camera.w - circle.w;
				}
				if (circle.x <= camera.x && rect.x >= camera.x + camera.w - rect.w && p1Moving && p2Moving) {
					std::cout << "Two players tried to move camera at the same time." << '\n';
					circle.x = camera.x;
					rect.x = camera.x + camera.w - rect.w;
				}
				// Diagonal
				if (rect.y <= camera.y && circle.y >= camera.y + camera.h - circle.h) {
					std::cout << "Two players tried to move camera at the same time." << '\n';
					rect.y = camera.y;
					circle.y = camera.y + camera.h - circle.h;
				}
				if (circle.y <= camera.y && rect.y >= camera.y + camera.h - rect.h) {
					std::cout << "Two players tried to move camera at the same time." << '\n';
					circle.y = camera.y;
					rect.y = camera.y + camera.h - rect.h;
				}

				// Camera position
				camera.x = (circle.x + rect.x) / 2 - scr_w / 2 + playerSize/2;
				// Don't go over left
				if (camera.x < 0) {
					camera.x = 0;
				}
				// Don't go over right
				else if (camera.x > lvl_w - camera.w) {
					camera.x = lvl_w - camera.w;
				}
				camera.y = (circle.y + rect.y) / 2 - scr_h / 2 + playerSize / 2;
				// Don't go over top
				if (camera.y < 0) {
					camera.y = 0;
				}
				// Don't go over bottom
				else if (camera.y + camera.h > lvl_h) {
					camera.y = lvl_h - camera.h;
				}

				SDL_SetRenderDrawColor(renderer, 34, 34, 128, SDL_ALPHA_OPAQUE);
				SDL_RenderClear(renderer);
				SDL_SetRenderDrawColor(renderer, 34, 34, 128, SDL_ALPHA_OPAQUE);
				int iterator = 0;
				while (tilesMap[iterator] != NULL) {
					for (int i = 0; i < tile_sprites; i++) {
						if (tilesMap[iterator]->getType() == textureList[i]->getName()) {
							tilesMap[iterator]->box.x = tilesMap[iterator]->x - camera.x; 
							tilesMap[iterator]->box.y = tilesMap[iterator]->y - camera.y;
							SDL_RenderCopy(renderer, textureList[i]->getTexture(), NULL, &tilesMap[iterator]->box);
							break;
						}
						else if (i == tile_sprites - 1) {
							tilesMap[iterator]->box.x = tilesMap[iterator]->x - camera.x;
							tilesMap[iterator]->box.y = tilesMap[iterator]->y - camera.y;
							SDL_RenderCopy(renderer, textureList[tile_sprites - 1]->getTexture(), NULL, &tilesMap[iterator]->box);
						}
					}
					iterator++;
				}

				SDL_Rect player1;
				player1.x = rect.x - camera.x;
				player1.y = rect.y - camera.y;
				player1.w = rect.w;
				player1.h = rect.h;
				SDL_RenderCopy(renderer, p1Texture, NULL, &player1);

				SDL_Rect player2;
				player2.x = circle.x - camera.x;
				player2.y = circle.y - camera.y;
				player2.w = circle.w;
				player2.h = circle.h;
				SDL_RenderCopy(renderer, p2Texture, NULL, &player2);

				SDL_RenderPresent(renderer);
			}
		}
	}

	// Free resources and close SDL
	close();

	return 0;
}