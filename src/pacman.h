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
#include <stdlib.h>

#define SPRITE_SIZE 32                     // Tamanho (largura e altura) de cada frame do sprite
#define PACMAN_SPRITE_COLS 9               // Número de colunas na spritesheet
#define PACMAN_SPRITE_ROWS 4               // Número de linhas na spritesheet (diferentes movimentos)
#define GHOST_SPRITE_COLS 1                // Número de colunas na spritesheet
#define GHOST_SPRITE_ROWS 4                // Número de linhas na spritesheet (diferentes movimentos)
#define PACMAN_V_0 5                       // Velocidade padrão do pacman
#define GHOSTS_V_0 5.5                     // Velocidade padrão dos fantasmas
#define MAPS_N 2                           // Número de mapas
#define FPS 60                             // Taxa de atualização da tela

// Botão
typedef struct Button Button;
struct Button {
	int x_i, x_f, y_i, y_f;  // Coordenadas inicial e final do retângulo correspondente ao botão
	bool hover;              // Indica o mouse em cima
};

// Obsoleto --> Thumbnails serão removidas do maps_menu
typedef struct Image Image;
struct Image {
	ALLEGRO_BITMAP *img;
	int w, h;
};

// Mapa
typedef struct Map Map;
struct Map {
	int **m;                          // Matriz que contém o mapa em si: paredes, pellets, vitaminas e vazio;
	int id;                           // Id do mapa utilizado
	int w, h;                         // Tamanho do mapa (aka dimensões da matriz != dimensões na tela)
	int x_i, x_f, y_i, y_f;           // Coordenadas inicial e final do retângulo em que o mapa será desenhado na tela
	double x_fac, y_fac;              // Fator de conversão mapa --> pixels
	double pellet_rad, vitamin_rad;   // Define o tamanho radial das pellets e da vitamina
	int pellet_n;                     // Conta as pellets restantes
};

// Informações dinâmicas (movimento) comuns ao Pacman e os fantasmas
typedef struct Dynamics Dynamics;
struct Dynamics {
	double start_x, start_y;        // Posição inicial da entidade. Retorna para tal, no caso de morte
	double x, y;                    // Posição atual
	double v;                       // Módulo da velocidade
	int direction_x, direction_y;   // Versor da direção. Ambos só podem receber +/-1, mas nunca ao mesmo tempo, de forma a formar as 4 direções
};

// Pacman
typedef struct Pacman Pacman;
struct Pacman {
	Dynamics dyn;              // Informações relacionadas ao movimento
	int points;                // Pontos
	double size;               // Tamanho "radial" do Pacman em relação ao mapa. Geralmente usado para colisões e desenhá-lo
	bool vitamin;              // True = efeito da vitamina ativado
	int lives;                 // Número de vidas
	int movement;              // Id do movimento, o qual corresponde à linha da spite sheet que deve ser utilizada
	int frame;                 // Id da coluna da sprite sheet que deve ser utilizada
	ALLEGRO_BITMAP *sprite;    // Sprite sheet
};

// Fantasma
typedef struct Ghost Ghost;
struct Ghost {
	Dynamics dyn;              // Informações relacionadas ao movimento
	double size;               // Tamanho "radial" do Pacman em relação ao mapa. Geralmente usado para colisões e desenhá-lo
	bool vulnerable;           // Indica que o Pacman comeu a vitamina e ele pode ser comido
	int movement;              // Id do movimento, o qual corresponde à linha da spite sheet que deve ser utilizada
	int frame;                 // Id da coluna da sprite sheet que deve ser utilizada
	ALLEGRO_BITMAP *sprite;    // Sprite sheet
};

void start (ALLEGRO_DISPLAY **display, ALLEGRO_EVENT_QUEUE **queue, ALLEGRO_TIMER **timer, int *width, int *height);

bool load_media (ALLEGRO_FONT **font, ALLEGRO_FONT **title_font, ALLEGRO_SAMPLE **menu_sample, ALLEGRO_SAMPLE **game_sample, ALLEGRO_BITMAP **background);

int main_menu (ALLEGRO_EVENT *ev, ALLEGRO_EVENT_QUEUE **queue, bool *running, ALLEGRO_FONT *font, ALLEGRO_FONT *title_font, int width, int height, ALLEGRO_BITMAP *background);

void main_menu_show (ALLEGRO_FONT **font, ALLEGRO_FONT **title_font, const Button *b, const int *b_n, const int *select, ALLEGRO_BITMAP **background, int width, int height);

int maps_menu (ALLEGRO_EVENT *ev, ALLEGRO_EVENT_QUEUE **queue, bool *running, ALLEGRO_FONT *font, int width, int height, Map *map);

void maps_menu_show (ALLEGRO_FONT **font, const Button *b, const int *maps_n, const int *map_sel, Map *map, char **names);

Ghost* get_entities (Map *map, Pacman *pacman, int *ghosts_n);

int game (ALLEGRO_EVENT *ev, ALLEGRO_EVENT_QUEUE **queue, bool *running, Map *map, ALLEGRO_FONT *title_font, int width, int height, ALLEGRO_TIMER **timer, double *sprite_timer, double *sprite_delay);

void game_show (Map *map, ALLEGRO_FONT **font, const Button *b, const int *b_n, const int *select, Pacman *pacman, Ghost *ghosts, const int *ghosts_n, int *width, int *height);

bool move_pacman (Map *map, Pacman *pacman);

void move_ghosts (Map *map, Ghost *ghosts, int *ghosts_n);

void change_direction (Ghost *ghost);

void verify_defeat (Pacman *pacman, Ghost *ghosts, int *ghosts_n, int *defeat_active);

void get_map (int map_id, Map *map);

void free_map (Map *map);

void destroy_all (ALLEGRO_SAMPLE_INSTANCE **sample_instance, ALLEGRO_SAMPLE **sample, ALLEGRO_FONT **font, ALLEGRO_TIMER **timer, ALLEGRO_EVENT_QUEUE **queue, ALLEGRO_DISPLAY **display);
