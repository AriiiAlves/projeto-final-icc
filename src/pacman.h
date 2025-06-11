#pragma once
#include <allegro5/allegro.h>              // Biblioteca principal do Allegro
#include <allegro5/allegro_font.h>         // Suporte para fontes bitmap
#include <allegro5/allegro_ttf.h>          // Suporte para fontes TrueType
#include <allegro5/allegro_primitives.h>   // Desenho de primitivas (formas geométricas)
#include <allegro5/allegro_audio.h>        // Sistema de áudio
#include <allegro5/allegro_acodec.h>       // Suporte a codecs de áudio
#include <allegro5/allegro_image.h>        // Carregamento de imagens
#include <allegro5/mouse.h>                // Suporte ao mouse
#include <allegro5/keyboard.h>             // Suporte ao teclado
#include <stdio.h>

#define SPRITE_SIZE 32                     // Tamanho (largura e altura) de cada frame do sprite
#define SPRITE_COLS 5                      // Número de colunas na spritesheet
#define SPRITE_ROWS 8                      // Número de linhas na spritesheet (diferentes movimentos)
#define FPS 60

typedef struct Button Button;
struct Button {
	int x_i, x_f, y_i, y_f;
	bool hover;
};

typedef struct Image Image;
struct Image {
	ALLEGRO_BITMAP *img;
	int w, h;
};

typedef struct Map Map;
struct Map {
	int **m;
	int id;
	int w, h;
	int x_i, x_f, y_i, y_f;
	double x_fac, y_fac;
	double pellet_rad, vitamin_rad;
};

typedef struct Dynamics Dynamics;
struct Dynamics {
	double x, y;
	double v;
	int direction_x, direction_y;
};

typedef struct Pacman Pacman;
struct Pacman {
	Dynamics dyn;
	int points;
	double size;
	bool vitamin;
	ALLEGRO_BITMAP *sprite;
};

typedef struct Ghost Ghost;
struct Ghost {
	Dynamics dyn;
	double size;
	bool vulnerable;
	ALLEGRO_BITMAP *sprite;
};

void start (ALLEGRO_DISPLAY **display, ALLEGRO_EVENT_QUEUE **queue, ALLEGRO_TIMER **timer, int *width, int *height);

bool load_media (ALLEGRO_FONT **font, ALLEGRO_SAMPLE **sample);

int main_menu (ALLEGRO_EVENT *ev, ALLEGRO_EVENT_QUEUE **queue, bool *running, ALLEGRO_FONT *font, int width, int height);

void main_menu_show (ALLEGRO_FONT **font, const Button *b, const int *b_n, const int *select);

int maps_menu (ALLEGRO_EVENT *ev, ALLEGRO_EVENT_QUEUE **queue, bool *running, ALLEGRO_FONT *font, int width, int height, Map *map);

void maps_menu_show (ALLEGRO_FONT **font, const Button *b, const int *b_n, const int *select, Image *thumbs, int *thumbs_n);

Ghost* get_entities (Map *map, Pacman *pacman, int *ghosts_n);

int game (ALLEGRO_EVENT *ev, ALLEGRO_EVENT_QUEUE **queue, bool *running, Map *map, ALLEGRO_FONT *font, int width, int height);

void game_show (Map *map, ALLEGRO_FONT **font, const Button *b, const int *b_n, const int *select, Pacman *pacman, Ghost *ghosts, const int *ghosts_n);

void move_pacman (Map *map, Pacman *pacman);

void move_ghosts (Map *map, Ghost *ghosts, int *ghosts_n);

void get_map (int map_id, Map *map);

void free_map (Map *map);

void destroy_all (ALLEGRO_SAMPLE_INSTANCE **sample_instance, ALLEGRO_SAMPLE **sample, ALLEGRO_FONT **font, ALLEGRO_TIMER **timer, ALLEGRO_EVENT_QUEUE **queue, ALLEGRO_DISPLAY **display);
