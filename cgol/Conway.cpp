#include "Conway.h"
#include <iostream>
#include <stdlib.h>
#include <math.h>

Conway::Conway(std::string name) : isRunning(true), isSimulating(false),
								window(nullptr), renderer(nullptr),
								texture(nullptr), world(nullptr),
								oldWorld(nullptr), fps(20),
								colors(new uint32_t[ALIEN + 1]), alienCounter(0),
								cellSize(8), boardWidth(WINDOW_W / cellSize),
								boardHeight(WINDOW_H / cellSize) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("Error initializing SDL: %s\n", SDL_GetError());
	}
	window = SDL_CreateWindow(name.c_str(),
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		WINDOW_W,
		WINDOW_H,
		SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, 0);
	fps = 20;
	cellSize = 8;
	colors = new uint32_t[ALIEN + 1];
	for (uint8_t i = 0; i < ALIEN + 1; ++i) {
		if (i == 0)
			colors[i] = 0xFF;
		else if (i < ALIVE) {
			colors[i] = ((i * 16) & (0xFF)) << 24 |
				((i * 16) & (0xFF)) << 16 |
				((i * 16) & (0xFF)) << 8 |
				0xFF; // Gray
		}
		else if (i < OLD)
			colors[i] = 0x00C000FF; // Green
		else if (i < ALIEN)
			colors[i] = 0x0020C0FF; // Blue
		else if (i == ALIEN)
			colors[i] = 0xC00000FF; // Red
	}
}

Conway::~Conway() {
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
}

void Conway::initialize() {
	boardWidth = WINDOW_W / cellSize;
	boardHeight = WINDOW_H / cellSize;
	world = new uint8_t[boardWidth * boardHeight];
	oldWorld = new uint8_t[boardWidth * boardHeight];
	for (uint16_t y = 0; y < boardHeight; ++y)
		for (uint16_t x = 0; x < boardWidth; ++x)
			setCell(world, x, y, rand() % 5 ? DEAD : ALIVE);
}

void Conway::run() {
	isRunning = true;
	isSimulating = false;
	SDL_Event event;
	uint16_t count = 0;

	while (isRunning) {
		while (SDL_PollEvent(&event)) {
			if (event.type != SDL_QUIT && event.type != SDL_KEYDOWN && event.type != SDL_MOUSEBUTTONDOWN) {
				continue;
			}
			switch (event.type) {
			case SDL_QUIT:
				isRunning = false;
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
				case SDLK_q:
					isRunning = false;
					break;
				case SDLK_SPACE:
					isSimulating = !isSimulating;
					break;
				case SDLK_DOWN:
					fps = std::max(fps - 1, 1);
					break;
				case SDLK_UP:
					fps = std::min(fps + 1, 100);
					break;
				case SDLK_a:
					alienCounter = 511;
					break;
				case SDLK_c:
					memset(world, DEAD, boardWidth * boardHeight * sizeof(uint8_t));
					break;
				case SDLK_n:
					if (!isSimulating) {
						step();
					}
					break;
				case SDLK_PLUS:
				case SDLK_EQUALS:
					if (cellSize < 128) {
						cellSize *= 2;
						initialize();
					}
					break;
				case SDLK_MINUS:
					if (cellSize > 1) {
						cellSize /= 2;
						initialize();
					}
					break;
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				setCell(world,
					event.button.x / cellSize,
					event.button.y / cellSize,
					getCellBinary(world, event.button.x / cellSize, event.button.y / cellSize) ? DEAD : ALIVE);
				break;
			default:
				break;
			}
		}

		if (isSimulating) {
			step();
		}
		render();
		SDL_Delay(1000 / fps);
	}
}

uint8_t Conway::getCellBinary(uint8_t* board, int16_t x, int16_t y) {
	return getCell(board, x, y) < ALIVE ? 0 : 1;
}

uint8_t Conway::getCell(uint8_t* board, int16_t x, int16_t y) {
	x = (x + boardWidth) % boardWidth;
	y = (y + boardHeight) % boardHeight;
	return board[y * boardWidth + x];
}

void Conway::setCell(uint8_t* board, int16_t x, int16_t y, uint8_t value) {
	x = (x + boardWidth) % boardWidth;
	y = (y + boardHeight) % boardHeight;
	board[y * boardWidth + x] = value;
}

void Conway::step() {
	uint8_t neighbours;
	uint8_t oldCell;
	uint8_t* tmp = oldWorld;
	uint16_t alive = 0;
	oldWorld = world;
	world = tmp;
	for (uint16_t y = 0; y < boardHeight; ++y) {
		for (uint16_t x = 0; x < boardWidth; ++x) {
			oldCell = getCell(oldWorld, x, y);
			neighbours = getCellBinary(oldWorld, x - 1, y - 1) +
				getCellBinary(oldWorld, x - 1, y) +
				getCellBinary(oldWorld, x - 1, y + 1) +
				getCellBinary(oldWorld, x, y - 1) +
				getCellBinary(oldWorld, x, y + 1) +
				getCellBinary(oldWorld, x + 1, y - 1) +
				getCellBinary(oldWorld, x + 1, y) +
				getCellBinary(oldWorld, x + 1, y + 1);

			if (neighbours == 3 && oldCell < ALIVE) {
				// Any dead cell with exactly three live
				// neighbours becomes a live cell
				setCell(world, x, y, ALIVE);
				alive++;
			}
			else if (neighbours < 2 || neighbours > 3 || oldCell < ALIVE) {
				// Any live cell with fewer than two live neighbours dies
				// Any live cell with more than three live neighbours dies
				// Any dead cell stays dead
				if (oldCell == DEAD) {
					// Dead stays dead
					setCell(world, x, y, DEAD);
				}
				else
				{
					// Fading out cell fades out by 1
					// In case it is an OLD cell, it has to
					// start fading out from ALIVE value
					setCell(world, x, y, std::min(ALIVE, (int)oldCell) - 1);
				}
			}
			else if (oldCell >= ALIVE) {
				// Any live cell with two or three live neighbours lives on
				setCell(world, x, y, std::min(OLD, oldCell + 1));
				alive++;
			}
		}
	}
	++alienCounter;
	// Every 512 - random alien injection
	if (alienCounter % 512 == 0) {
		uint16_t aliens = (boardWidth * boardHeight / 30);
		for (uint16_t i = 0; i < aliens; i++) {
			setCell(world, rand() % boardWidth, rand() % boardHeight, ALIEN);
		}
	}
}

void Conway::render() {
	SDL_SetRenderDrawColor(renderer, 32, 32, 32, 255);
	SDL_RenderClear(renderer);

	uint32_t color;
	SDL_Rect rect;
	rect.w = std::max(1, cellSize - 1);
	rect.h = std::max(1, cellSize - 1);
	for (uint16_t y = 0; y < boardHeight; ++y) {
		for (uint16_t x = 0; x < boardWidth; ++x) {
			color = colors[getCell(world, x, y)];
			SDL_SetRenderDrawColor(renderer,
				(color >> 24) & 0xFF,
				(color >> 16) & 0xFF,
				(color >> 8) & 0xFF,
				color & 0xFF);
			rect.x = cellSize * x;
			rect.y = cellSize * y;
			SDL_RenderFillRect(renderer, &rect);
		}
	}
	SDL_RenderPresent(renderer);
}