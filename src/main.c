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

typedef struct button button;
struct button {
	int x_i, x_f, y_i, y_f;
	bool hover;
};

typedef struct image image;
struct image {
	ALLEGRO_BITMAP *img;
	int w, h;
};

typedef struct Map Map;
struct Map {
	int **m;
	int w, h;
	int x_i, x_f, y_i, y_f;
	double x_fac, y_fac;
};

void start (ALLEGRO_DISPLAY **display, ALLEGRO_EVENT_QUEUE **queue, ALLEGRO_TIMER **timer, int *width, int *height);
int menu (ALLEGRO_EVENT *ev, ALLEGRO_EVENT_QUEUE **queue, bool *running, ALLEGRO_FONT *font, int width, int height);
void menu_show (ALLEGRO_FONT **font, const button *b, const int *b_n, const int *select);
int choose_map_menu (ALLEGRO_EVENT *ev, ALLEGRO_EVENT_QUEUE **queue, bool *running, ALLEGRO_FONT *font, int width, int height, Map *map);
void choose_map_menu_show (ALLEGRO_FONT **font, const button *b, const int *b_n, const int *select, image *thumbs, int *thumbs_n);
int game (ALLEGRO_EVENT *ev, ALLEGRO_EVENT_QUEUE **queue, bool *running, Map* map, ALLEGRO_FONT *font, int width, int height);
void game_show (Map* map, ALLEGRO_FONT **font, const button *b, const int *b_n, const int *select);
void get_map (int map_id, Map *map);
void free_map (Map *map);

int main() {
	ALLEGRO_DISPLAY *display;
	ALLEGRO_EVENT_QUEUE *queue;
	ALLEGRO_TIMER *timer;
	ALLEGRO_FONT *font;
	ALLEGRO_SAMPLE *sample;
	ALLEGRO_SAMPLE_INSTANCE *sample_instance;
//	ALLEGRO_BITMAP *sprite_sheet;
	int width = 1920, height = 1080; // Valores padrão caso o Allegro falhe em obter os do sistema

	start(&display, &queue, &timer, &width, &height);

	font = al_load_ttf_font("../../fontes/arial.ttf", 19, 0);
	if (!font) {
		printf("Erro ao carregar fonte.\n");
		return -1;
	}
	al_reserve_samples(1);
	sample = al_load_sample("../../sons/sound.wav");
	if (!sample) {
		printf("Erro ao carregar som.\n");
		return -1;
	}
	sample_instance = al_create_sample_instance(sample);
	al_attach_sample_instance_to_mixer(sample_instance, al_get_default_mixer());
	al_set_sample_instance_playmode(sample_instance, ALLEGRO_PLAYMODE_LOOP);
	al_play_sample_instance(sample_instance);
//	sprite_sheet = al_load_bitmap("../../sprites/sprite.png");
//	al_convert_mask_to_alpha(sprite_sheet, al_map_rgb(255, 0, 255)); // Remove fundo magenta

	// Variáveis de controle da aplicaçãoa
	int menu_id = 0;
//	printf("%dx%d\n", width, height);
	Map map = (Map){NULL, 0, 0, width*(0.5-0.25), width*(0.5+0.25), height*(0.54-0.44), height*(0.54+0.44), 0.0, 0.0};
	bool running = true;      // Indica se a aplicação deve continuar executando
//	bool redraw = true;       // Indica se a tela precisa ser redesenhada
//	bool hover = false;       // Indica se o mouse está sobre o botão
//	bool playing = false;     // Indica se o som está tocando
//	int mouse_x = 0, mouse_y = 0; // Posição atual do mouse
//	double time = 0.0;
	ALLEGRO_EVENT ev;         // Estrutura para eventos

/*
	// Controle da animação
	int frame = 0;            // Frame atual da animação (coluna)
	int movement = 0;         // Linha do spritesheet representando o movimento atual
	double sprite_timer = 0.0;// Timer para troca de frames
	double sprite_delay = 0.15;// Tempo entre cada frame (150 ms)
*/

	// Inicia o temporizador
	al_start_timer(timer);
	while (running) {
		al_wait_for_event(queue, &ev);
		switch (menu_id) {
		case 0:
			menu_id = menu(&ev, &queue, &running, font, width, height);
			break;
		case 1:
			if (map.m == NULL)
				get_map(0, &map);
			menu_id = game(&ev, &queue, &running, &map, font, width, height);
			break;
		case 2:
			menu_id = choose_map_menu(&ev, &queue, &running, font, width, height, &map);
			break;
		default:
			running = false;
			break;
		}
	}

	al_destroy_sample_instance(sample_instance);
	al_destroy_sample(sample);
//	al_destroy_bitmap(sprite_sheet);
	al_destroy_font(font);
	al_destroy_timer(timer);
	al_destroy_event_queue(queue);
	al_destroy_display(display);
	return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------*/

void start (ALLEGRO_DISPLAY **display, ALLEGRO_EVENT_QUEUE **queue, ALLEGRO_TIMER **timer, int *width, int *height ) {
	al_init();
	al_init_font_addon();
	al_init_ttf_addon();
	al_init_primitives_addon();
	al_init_image_addon();
	al_install_mouse();
	al_install_keyboard();
	al_install_audio();
	al_init_acodec_addon();
	// Cria uma janela com as dimensões especificadas
	al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
	ALLEGRO_DISPLAY_MODE mode;
	if (al_get_display_mode(0, &mode)) {
		*width = mode.width;
		*height = mode.height;
	}
	*display = al_create_display(*width, *height);
	// Cria uma fila de eventos para tratar interações do usuário e um temporizador para controle de FPS
	*queue = al_create_event_queue();
	*timer = al_create_timer(1.0 / FPS);
	// Registra as fontes de eventos que a fila vai monitorar
	al_register_event_source(*queue, al_get_display_event_source(*display));
	al_register_event_source(*queue, al_get_timer_event_source(*timer));
	al_register_event_source(*queue, al_get_mouse_event_source());
	al_register_event_source(*queue, al_get_keyboard_event_source());
}

/*-------------------------------------------------------------------------------------------------------------------------*/

int menu (ALLEGRO_EVENT *ev, ALLEGRO_EVENT_QUEUE **queue, bool *running, ALLEGRO_FONT *font, int width, int height) {
	int b_n = 3;
	button* b;
	b = malloc(b_n * sizeof(button));
	b[0] = (button){width*(0.5-0.2), width*(0.5+0.2), height*(0.35-0.05), height*(0.35+0.05), false};
	b[1] = (button){width*(0.5-0.2), width*(0.5+0.2), height*(0.46-0.05), height*(0.46+0.05), false};
	b[2] = (button){width*(0.5-0.2), width*(0.5+0.2), height*(0.57-0.05), height*(0.57+0.05), false};
	int mouse_x, mouse_y;
	bool redraw = false;
	int select = -1;
	while (*running) {
		al_wait_for_event(*queue, ev);
		// Tratamento dos eventos recebidos
		switch (ev->type) {
		case ALLEGRO_EVENT_DISPLAY_CLOSE:
			*running = false;
			break;
		case ALLEGRO_EVENT_MOUSE_AXES:
		case ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY:
			// Atualiza posição do mouse e detecta se está sobre o botão
			mouse_x = ev->mouse.x;
			mouse_y = ev->mouse.y;
			for (int i = 0; i < b_n; i++) {
				b[i].hover = (mouse_x >= b[i].x_i && mouse_x <= b[i].x_f && mouse_y >= b[i].y_i && mouse_y <= b[i].y_f);
				if (b[i].hover)
					select = i;
			}
			break;
		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
			// Ao clicar sobre o botão, vai para o menu correspondente
			for (int i = 0; i < b_n; i++)
				if (b[i].hover)
					return i+1;
			break;
		case ALLEGRO_EVENT_KEY_DOWN:
			if (ev->keyboard.keycode == ALLEGRO_KEY_LEFT || ev->keyboard.keycode == ALLEGRO_KEY_UP) {
				if (select < 0)
					select = 0;
				else if (--select < 0)
					select += b_n;
			}
			if (ev->keyboard.keycode == ALLEGRO_KEY_RIGHT || ev->keyboard.keycode == ALLEGRO_KEY_DOWN) {
				if (select < 0)
					select = 0;
				else if (++select == b_n)
					select -= b_n;
			}
			if (select >= 0 && ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)
				select = -1;
			if (ev->keyboard.keycode == ALLEGRO_KEY_ENTER || ev->keyboard.keycode == ALLEGRO_KEY_PAD_ENTER)
				if (select >= 0)
					return select+1;
			break;
		case ALLEGRO_EVENT_TIMER:
			redraw = true; // Marca que a tela precisa ser redesenhada
			break;
		}
		if (redraw && al_is_event_queue_empty(*queue)) {
			menu_show(&font, b, &b_n, &select);
			redraw = false;
		}
	}
	return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------*/

void menu_show (ALLEGRO_FONT **font, const button *b, const int *b_n, const int *select) {
	al_clear_to_color(al_map_rgb(30, 40, 30));
	ALLEGRO_COLOR b_color = al_map_rgb(0,180,255),
		      b_color_hover = al_map_rgb(0,120,255);

	for (int i = 0; i < *b_n; i++)
		if (b[i].hover || *select == i)
			al_draw_filled_rounded_rectangle(b[i].x_i, b[i].y_i, b[i].x_f, b[i].y_f, 10, 10, b_color_hover);
		else
			al_draw_filled_rounded_rectangle(b[i].x_i, b[i].y_i, b[i].x_f, b[i].y_f, 10, 10, b_color);
	al_draw_text(*font, al_map_rgb(255,255,55), (b[0].x_i+b[0].x_f)/2, (b[0].y_i+b[0].y_f)/2, ALLEGRO_ALIGN_CENTER, "Start");
	al_draw_text(*font, al_map_rgb(255,255,55), (b[1].x_i+b[1].x_f)/2, (b[1].y_i+b[1].y_f)/2, ALLEGRO_ALIGN_CENTER, "Maps");
	al_draw_text(*font, al_map_rgb(255,255,55), (b[2].x_i+b[2].x_f)/2, (b[2].y_i+b[2].y_f)/2, ALLEGRO_ALIGN_CENTER, "Quit");
/*
	int sprite_x = frame * SPRITE_SIZE;
	int sprite_y = movement * SPRITE_SIZE;
	al_draw_bitmap_region(sprite_sheet, sprite_x, sprite_y, SPRITE_SIZE, SPRITE_SIZE, (WIDTH - SPRITE_SIZE)/2, HEIGHT - SPRITE_SIZE - 80, 0);
*/
	al_flip_display();
}

/*-------------------------------------------------------------------------------------------------------------------------*/

int choose_map_menu (ALLEGRO_EVENT *ev, ALLEGRO_EVENT_QUEUE **queue, bool *running, ALLEGRO_FONT *font, int width, int height, Map *map) {
	int thumbs_n = 3;
	image* thumbs = malloc(thumbs_n * sizeof(image));
	thumbs[0].img = al_load_bitmap("../../imagens/thumbs/original.jpg");
	thumbs[1].img = al_load_bitmap("../../imagens/thumbs/original.jpg");
	thumbs[2].img = al_load_bitmap("../../imagens/thumbs/original.jpg");
	for (int i = 0; i < thumbs_n; i++) {
		thumbs[i].w = al_get_bitmap_width(thumbs[i].img);
		thumbs[i].h = al_get_bitmap_width(thumbs[i].img);
	}
	int b_n = 3;
	button* b = malloc(b_n * sizeof(button));
	b[0] = (button){width*(0.23-0.13), width*(0.23+0.13), height*(0.5-0.1), height*(0.5+0.1), false};
	b[1] = (button){width*(0.5-0.13), width*(0.5+0.13), height*(0.5-0.1), height*(0.5+0.1), false};
	b[2] = (button){width*(0.77-0.13), width*(0.77+0.13), height*(0.5-0.1), height*(0.5+0.1), false};
	int mouse_x, mouse_y;
	bool redraw = false;
	int select = -1;
	while (*running) {
		al_wait_for_event(*queue, ev);
		// Tratamento dos eventos recebidos
		switch (ev->type) {
		case ALLEGRO_EVENT_DISPLAY_CLOSE:
			*running = false;
			break;
		case ALLEGRO_EVENT_MOUSE_AXES:
		case ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY:
			// Atualiza posição do mouse e detecta se está sobre o botão
			mouse_x = ev->mouse.x;
			mouse_y = ev->mouse.y;
			for (int i = 0; i < b_n; i++) {
				b[i].hover = (mouse_x >= b[i].x_i && mouse_x <= b[i].x_f && mouse_y >= b[i].y_i && mouse_y <= b[i].y_f);
				if (b[i].hover)
					select = i;
			}
			break;
		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
			// Ao clicar sobre o botão, vai seleciona o mapa correspondente e volta para o menu principal
			for (int i = 0; i < thumbs_n; i++)
				if (b[i].hover) {
					if (map->m != NULL)
						free_map(map);
					get_map(i, map);
					return 0;
				}
			break;
		case ALLEGRO_EVENT_KEY_DOWN:
			if (ev->keyboard.keycode == ALLEGRO_KEY_LEFT) {
				if (select < 0)
					select = 0;
				else if (--select < 0)
					select += b_n;
			}
			if (ev->keyboard.keycode == ALLEGRO_KEY_RIGHT) {
				if (select < 0)
					select = 0;
				else if (++select == b_n)
					select -= b_n;
			}
			if (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
				if (select >= 0)
					select = -1;
				else
					return 0;
			}
			if (ev->keyboard.keycode == ALLEGRO_KEY_ENTER || ev->keyboard.keycode == ALLEGRO_KEY_PAD_ENTER)
				if (select >= 0 && select < thumbs_n) {
					if (map->m != NULL)
						free_map(map);
					get_map(select, map);
					return 0;
				}
			break;
		case ALLEGRO_EVENT_TIMER:
			redraw = true; // Marca que a tela precisa ser redesenhada
			break;
		}
		if (redraw && al_is_event_queue_empty(*queue)) {
			choose_map_menu_show(&font, b, &b_n, &select, thumbs, &thumbs_n);
			redraw = false;
		}
	}
	return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------*/

void choose_map_menu_show (ALLEGRO_FONT **font, const button *b, const int *b_n, const int *select, image *thumbs, int *thumbs_n) {
	al_clear_to_color(al_map_rgb(30, 40, 30));
	ALLEGRO_COLOR thumbs_color = al_map_rgba_f(1, 1, 1, 0.75),
		      thumbs_color_hover = al_map_rgba_f(1, 1, 1, 1);

	for (int i = 0; i < *thumbs_n; i++) {
		if (b[i].hover || *select == i)
			al_draw_tinted_scaled_bitmap(thumbs[i].img, thumbs_color_hover, 0, 0, thumbs[i].w, thumbs[i].h, b[i].x_i, b[i].y_i, (b[i].x_f-b[i].x_i), (b[i].y_f-b[i].y_i), 0);
		else
			al_draw_tinted_scaled_bitmap(thumbs[i].img, thumbs_color, 0, 0, thumbs[i].w, thumbs[i].h, b[i].x_i, b[i].y_i, (b[i].x_f-b[i].x_i), (b[i].y_f-b[i].y_i), 0);
	}
/*
	int sprite_x = frame * SPRITE_SIZE;
	int sprite_y = movement * SPRITE_SIZE;
	al_draw_bitmap_region(sprite_sheet, sprite_x, sprite_y, SPRITE_SIZE, SPRITE_SIZE, (WIDTH - SPRITE_SIZE)/2, HEIGHT - SPRITE_SIZE - 80, 0);
*/
	al_flip_display();
}

/*-------------------------------------------------------------------------------------------------------------------------*/

int game (ALLEGRO_EVENT *ev, ALLEGRO_EVENT_QUEUE **queue, bool *running, Map* map, ALLEGRO_FONT *font, int width, int height) {
	map->x_fac = 1.0*(map->x_f - map->x_i)/map->w;
	map->y_fac = 1.0*(map->y_f - map->y_i)/map->h;
//	printf("x:%lf\ny:%lf\n", map->x_fac, map->y_fac);
//	printf("x_i:%d\nx_f:%d\n", map->x_i, map->x_f);
	/*
	for (int i = 0; i < map->h; ++i) {
		for (int j = 0; j < map->w; ++j)
			printf("%d", map->m[i][j]);
		printf("\n");
	}
	*/
	int b_n = 0;
	button* b = NULL;
/*
	b = malloc(b_n * sizeof(button));
	b[0] = (button){width*(0.5-0.2), width*(0.5+0.2), height*(0.35-0.05), height*(0.35+0.05), false};
	b[1] = (button){width*(0.5-0.2), width*(0.5+0.2), height*(0.46-0.05), height*(0.46+0.05), false};
	b[2] = (button){width*(0.5-0.2), width*(0.5+0.2), height*(0.57-0.05), height*(0.57+0.05), false};
*/
	int mouse_x, mouse_y;
	bool redraw = false;
	int select = -1;
	while (*running) {
		al_wait_for_event(*queue, ev);
		// Tratamento dos eventos recebidos
		switch (ev->type) {
		case ALLEGRO_EVENT_DISPLAY_CLOSE:
			*running = false;
			break;
		case ALLEGRO_EVENT_MOUSE_AXES:
		case ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY:
			// Atualiza posição do mouse e detecta se está sobre o botão
			mouse_x = ev->mouse.x;
			mouse_y = ev->mouse.y;
			for (int i = 0; i < b_n; i++) {
				b[i].hover = (mouse_x >= b[i].x_i && mouse_x <= b[i].x_f && mouse_y >= b[i].y_i && mouse_y <= b[i].y_f);
				if (b[i].hover)
					select = i;
			}
			break;
		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
			// Ao clicar sobre o botão, vai para o menu correspondente
			for (int i = 0; i < b_n; i++)
				if (b[i].hover)
					return i+1;
			break;
		case ALLEGRO_EVENT_KEY_DOWN:
			if (ev->keyboard.keycode == ALLEGRO_KEY_LEFT || ev->keyboard.keycode == ALLEGRO_KEY_UP) {
				if (select < 0)
					select = 0;
				else if (--select < 0)
					select += b_n;
			}
			if (ev->keyboard.keycode == ALLEGRO_KEY_RIGHT || ev->keyboard.keycode == ALLEGRO_KEY_DOWN) {
				if (select < 0)
					select = 0;
				else if (++select == b_n)
					select -= b_n;
			}
			if (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
				if (select >= 0)
					select = -1;
				else
					*running = false;
			}
			if (ev->keyboard.keycode == ALLEGRO_KEY_ENTER || ev->keyboard.keycode == ALLEGRO_KEY_PAD_ENTER)
				if (select >= 0)
					return select+1;
			break;
		case ALLEGRO_EVENT_TIMER:
			redraw = true; // Marca que a tela precisa ser redesenhada
			break;
		}
		if (redraw && al_is_event_queue_empty(*queue)) {
			game_show(map, &font, b, &b_n, &select);
			redraw = false;
		}
	}
	return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------*/

void game_show (Map* map, ALLEGRO_FONT **font, const button *b, const int *b_n, const int *select) {
	al_clear_to_color(al_map_rgb(30, 40, 30));
	ALLEGRO_COLOR wall = al_map_rgb(0, 10, 100),
		      empty = al_map_rgb(0, 0, 0),
		      pellet = al_map_rgb(250, 255, 255),
		      vitamin = al_map_rgb(250, 150, 150);

	al_draw_filled_rectangle(map->x_i-10, map->y_i-10, map->x_f+10, map->y_f+10, al_map_rgb(0, 0, 0));

	for (int i = 0; i < map->h; ++i) {
		for (int j = 0; j < map->w; ++j) {
			switch (map->m[i][j]) {
			default:
				al_draw_filled_rectangle(map->x_i+map->x_fac*j, map->y_i+map->y_fac*i, map->x_i+map->x_fac*(j+1), map->y_i+map->y_fac*(i+1), wall);
				break;
			case 1:
				al_draw_filled_rectangle(map->x_i+map->x_fac*j, map->y_i+map->y_fac*i, map->x_i+map->x_fac*(j+1), map->y_i+map->y_fac*(i+1), empty);
				al_draw_filled_circle(map->x_i+map->x_fac*(j+0.5), map->y_i+map->y_fac*(i+0.5), map->y_fac*0.125, pellet);
				break;
			case 2:
				al_draw_filled_rectangle(map->x_i+map->x_fac*j, map->y_i+map->y_fac*i, map->x_i+map->x_fac*(j+1), map->y_i+map->y_fac*(i+1), empty);
				break;
			case 3:
				al_draw_filled_rectangle(map->x_i+map->x_fac*j, map->y_i+map->y_fac*i, map->x_i+map->x_fac*(j+1), map->y_i+map->y_fac*(i+1), empty);
				al_draw_filled_circle(map->x_i+map->x_fac*(j+0.5), map->y_i+map->y_fac*(i+0.5), map->y_fac*0.375, vitamin);
				break;
			}
		}
	}

/*
	ALLEGRO_COLOR b_color = al_map_rgb(0,180,255),
		      b_color_hover = al_map_rgb(0,120,255);

	for (int i = 0; i < *b_n; i++)
		if (b[i].hover || *select == i)
			al_draw_filled_rounded_rectangle(b[i].x_i, b[i].y_i, b[i].x_f, b[i].y_f, 10, 10, b_color_hover);
		else
			al_draw_filled_rounded_rectangle(b[i].x_i, b[i].y_i, b[i].x_f, b[i].y_f, 10, 10, b_color);
	al_draw_text(*font, al_map_rgb(255,255,55), (b[0].x_i+b[0].x_f)/2, (b[0].y_i+b[0].y_f)/2, ALLEGRO_ALIGN_CENTER, "Start");
	al_draw_text(*font, al_map_rgb(255,255,55), (b[1].x_i+b[1].x_f)/2, (b[1].y_i+b[1].y_f)/2, ALLEGRO_ALIGN_CENTER, "Maps");
	al_draw_text(*font, al_map_rgb(255,255,55), (b[2].x_i+b[2].x_f)/2, (b[2].y_i+b[2].y_f)/2, ALLEGRO_ALIGN_CENTER, "Quit");

	int sprite_x = frame * SPRITE_SIZE;
	int sprite_y = movement * SPRITE_SIZE;
	al_draw_bitmap_region(sprite_sheet, sprite_x, sprite_y, SPRITE_SIZE, SPRITE_SIZE, (WIDTH - SPRITE_SIZE)/2, HEIGHT - SPRITE_SIZE - 80, 0);
*/
	al_flip_display();
}

/*-------------------------------------------------------------------------------------------------------------------------*/

void get_map (int map_id, Map *map) {
	FILE* map_file;
	switch (map_id) {
	default:
		map_file = fopen("../../mapas/original.txt", "r");
		break;
	}
	fscanf(map_file, "%d %d", &map->w, &map->h);
	map->m = malloc(map->h * sizeof(int*));
	for (int i = 0; i < map->h; i++)
		map->m[i] = malloc(map->w * sizeof(int));
	char *line = malloc((map->w+1) * sizeof(char));
	for (int i = 0; i < map->h; i++) {
		fscanf(map_file, "%s ", line);
		for (int j = 0; j < map->w; j++) {
			switch (line[j]) {
			default:// Parede
				map->m[i][j] = 0;
				break;
			case '1':// Bolinha
				map->m[i][j] = 1;
				break;
			case '2':// Vazio
				map->m[i][j] = 2;
				break;
			case '3':// Vitamina
				map->m[i][j] = 3;
				break;

			}
		}
	}
	fclose(map_file);
}

/*-------------------------------------------------------------------------------------------------------------------------*/

void free_map (Map *map) {
	for (int i = 0; i < map->w; i++)
		free(map->m[i]);
	free(map->m);
	map->m = NULL;
}
