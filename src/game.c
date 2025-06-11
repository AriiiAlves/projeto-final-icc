#include"pacman.h"

int game (ALLEGRO_EVENT *ev, ALLEGRO_EVENT_QUEUE **queue, bool *running, Map *map, ALLEGRO_FONT *font, int width, int height) {
	Pacman pacman;
	int ghosts_n;
	Ghost *ghosts = get_entities(map, &pacman, &ghosts_n);
	if (!ghosts) {
		printf("Failed to load sprites\n");
		*running = false;
	}
	map->x_fac = 1.0*(map->x_f - map->x_i)/map->w;
	map->y_fac = 1.0*(map->y_f - map->y_i)/map->h;

	int b_n = 0;
	Button* b = NULL;
/*
	b = malloc(b_n * sizeof(Button));
	b[0] = (Button){width*(0.5-0.2), width*(0.5+0.2), height*(0.35-0.05), height*(0.35+0.05), false};
	b[1] = (Button){width*(0.5-0.2), width*(0.5+0.2), height*(0.46-0.05), height*(0.46+0.05), false};
	b[2] = (Button){width*(0.5-0.2), width*(0.5+0.2), height*(0.57-0.05), height*(0.57+0.05), false};
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
			if (ev->keyboard.keycode == ALLEGRO_KEY_UP || ev->keyboard.keycode == ALLEGRO_KEY_W)
				if ((map->m[(int)(pacman.dyn.y-1)][(int)(pacman.dyn.x)] % 4)) {
					pacman.dyn.direction_x = 0;
					pacman.dyn.direction_y = -1;
				}
			if (ev->keyboard.keycode == ALLEGRO_KEY_LEFT || ev->keyboard.keycode == ALLEGRO_KEY_A)
				if ((map->m[(int)(pacman.dyn.y)][(int)(pacman.dyn.x-1)] % 4)) {
					pacman.dyn.direction_x = -1;
					pacman.dyn.direction_y = 0;
				}
			if (ev->keyboard.keycode == ALLEGRO_KEY_DOWN || ev->keyboard.keycode == ALLEGRO_KEY_S)
				if ((map->m[(int)(pacman.dyn.y+1)][(int)(pacman.dyn.x)] % 4)) {
					pacman.dyn.direction_x = 0;
					pacman.dyn.direction_y = 1;
				}
			if (ev->keyboard.keycode == ALLEGRO_KEY_RIGHT || ev->keyboard.keycode == ALLEGRO_KEY_D)
				if ((map->m[(int)(pacman.dyn.y)][(int)(pacman.dyn.x+1)] % 4)) {
					pacman.dyn.direction_x = 1;
					pacman.dyn.direction_y = 0;
				}
			if (ev->keyboard.keycode == ALLEGRO_KEY_PAD_PLUS)
				pacman.dyn.v += 1;
			if (ev->keyboard.keycode == ALLEGRO_KEY_PAD_MINUS)
				pacman.dyn.v -= 1;
			if (ev->keyboard.keycode == ALLEGRO_KEY_EQUALS)
				pacman.dyn.v += 0.1;
			if (ev->keyboard.keycode == ALLEGRO_KEY_MINUS)
				pacman.dyn.v -= 0.1;
			if (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
				*running = false;
			}
			break;
		case ALLEGRO_EVENT_TIMER:
			move_pacman(map, &pacman);
			move_ghosts(map, ghosts, &ghosts_n);
			redraw = true; // Marca que a tela precisa ser redesenhada
			break;
		}
		if (redraw && al_is_event_queue_empty(*queue)) {
			game_show(map, &font, b, &b_n, &select, &pacman, ghosts, &ghosts_n);
			redraw = false;
		}
	}
	free(b);
	free(ghosts);
	free_map(map);
	return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------*/

void game_show (Map *map, ALLEGRO_FONT **font, const Button *b, const int *b_n, const int *select, Pacman *pacman, Ghost *ghosts, const int *ghosts_n) {
	al_clear_to_color(al_map_rgb(30, 40, 30));
	ALLEGRO_COLOR wall = al_map_rgb(0, 10, 100),
		      door = al_map_rgb(40, 80, 150),
		      empty = al_map_rgb(10, 10, 12),
		      pellet = al_map_rgb(250, 255, 255),
		      vitamin = al_map_rgb(250, 150, 150);

	al_draw_filled_rectangle(map->x_i-10, map->y_i-10, map->x_f+10, map->y_f+10, al_map_rgb(0, 0, 0));

	for (int i = 0; i < map->h; ++i) {
		for (int j = 0; j < map->w; ++j) {
			switch (map->m[i][j]) {
			default: // Wall
				al_draw_filled_rectangle(map->x_i+map->x_fac*j, map->y_i+map->y_fac*i, map->x_i+map->x_fac*(j+1), map->y_i+map->y_fac*(i+1), wall);
				break;
			case 1: // Pellet
				al_draw_filled_rectangle(map->x_i+map->x_fac*j, map->y_i+map->y_fac*i, map->x_i+map->x_fac*(j+1), map->y_i+map->y_fac*(i+1), empty);
				al_draw_filled_circle(map->x_i+map->x_fac*(j+0.5), map->y_i+map->y_fac*(i+0.5), map->y_fac*map->pellet_rad, pellet);
				break;
			case 2: // Empty
				al_draw_filled_rectangle(map->x_i+map->x_fac*j, map->y_i+map->y_fac*i, map->x_i+map->x_fac*(j+1), map->y_i+map->y_fac*(i+1), empty);
				break;
			case 3: // Vitamin
				al_draw_filled_rectangle(map->x_i+map->x_fac*j, map->y_i+map->y_fac*i, map->x_i+map->x_fac*(j+1), map->y_i+map->y_fac*(i+1), empty);
				al_draw_filled_circle(map->x_i+map->x_fac*(j+0.5), map->y_i+map->y_fac*(i+0.5), map->y_fac*map->vitamin_rad, vitamin);
				break;
			case 4: // Door of the ghosts' beginning
				al_draw_filled_rectangle(map->x_i+map->x_fac*j, map->y_i+map->y_fac*i, map->x_i+map->x_fac*(j+1), map->y_i+map->y_fac*(i+1), empty);
				al_draw_filled_rectangle(map->x_i+map->x_fac*j, map->y_i+map->y_fac*(i+0.4), map->x_i+map->x_fac*(j+1), map->y_i+map->y_fac*(i+0.6), door);
				break;
			}
		}
	}

	ALLEGRO_COLOR color_pacman = al_map_rgb(200, 200, 30),
		      color_ghosts[4] = {al_map_rgb(220, 10, 10), al_map_rgb(20, 170, 230), al_map_rgb(240, 120, 20), al_map_rgb(250, 90, 200)};
	al_draw_filled_rectangle(map->x_i+map->x_fac*(pacman->dyn.x-0.25), map->y_i+map->y_fac*(pacman->dyn.y-0.25), map->x_i+map->x_fac*(pacman->dyn.x+0.25), map->y_i+map->y_fac*(pacman->dyn.y+0.25), color_pacman);
	for (int i = 0; i < *ghosts_n; i++)
		al_draw_filled_rectangle(map->x_i+map->x_fac*(ghosts[i].dyn.x-0.25), map->y_i+map->y_fac*(ghosts[i].dyn.y-0.25), map->x_i+map->x_fac*(ghosts[i].dyn.x+0.25), map->y_i+map->y_fac*(ghosts[i].dyn.y+0.25), color_ghosts[i]);

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

Ghost* get_entities (Map *map, Pacman *pacman, int *ghosts_n) {
/*
	// Controle da animação
	int frame = 0;            // Frame atual da animação (coluna)
	int movement = 0;         // Linha do spritesheet representando o movimento atual
	double sprite_timer = 0.0;// Timer para troca de frames
	double sprite_delay = 0.15;// Tempo entre cada frame (150 ms)
*/
	Ghost* ghosts;
	switch (map->id) {
	default:
		*pacman = (Pacman){(Dynamics){0.0, 0.0, 5.0, 0, 0}, 0, 0.25, false, NULL};
		pacman->sprite = al_load_bitmap("../../sprites/sprite.png");
		pacman->dyn.x = map->w/2.0;
		pacman->dyn.y = 24.5;
		*ghosts_n = 4;
		ghosts = malloc(*ghosts_n * sizeof(Ghost));
		ghosts[0] = (Ghost){(Dynamics){0.0, 0.0, 5.1, 0, 0}, 0.25, false, NULL};
		ghosts[1] = (Ghost){(Dynamics){0.0, 0.0, 5.1, 0, 0}, 0.25, false, NULL};
		ghosts[2] = (Ghost){(Dynamics){0.0, 0.0, 5.1, 0, 0}, 0.25, false, NULL};
		ghosts[3] = (Ghost){(Dynamics){0.0, 0.0, 5.1, 0, 0}, 0.25, false, NULL};
		ghosts[0].sprite = al_load_bitmap("../../sprites/sprite.png");
		ghosts[1].sprite = al_load_bitmap("../../sprites/sprite.png");
		ghosts[2].sprite = al_load_bitmap("../../sprites/sprite.png");
		ghosts[3].sprite = al_load_bitmap("../../sprites/sprite.png");
		for (int i = 0; i < *ghosts_n; i++) {
			al_convert_mask_to_alpha(ghosts[i].sprite, al_map_rgb(255, 0, 255)); // Remove fundo magenta
			if (!ghosts[i].sprite)
				return NULL;
			ghosts[i].dyn.x = (map->w - *ghosts_n + 1.0)/2.0+i;
			ghosts[i].dyn.y = 15;
		}
		break;
	}
	return ghosts;
}

/*-------------------------------------------------------------------------------------------------------------------------*/

void move_pacman (Map *map, Pacman *pacman) {
	int next_square;
	if (pacman->dyn.direction_x) {
		// Movimento
		pacman->dyn.x += pacman->dyn.direction_x * pacman->dyn.v / FPS; // Move
		pacman->dyn.y = (int)(pacman->dyn.y) + 0.5; // Centraliza
		if (pacman->dyn.x < pacman->size) // Faz o túnel, une as paredes esquerda e direita
			pacman->dyn.x = map->w - pacman->size;
		else if (pacman->dyn.x + pacman->size > map->w)
			pacman->dyn.x = pacman->size;
		next_square = map->m[(int)(pacman->dyn.y)][(int)(pacman->dyn.x+pacman->dyn.direction_x*0.5)];
		if (!next_square) // Se parede, não anda
			pacman->dyn.x = (int)(pacman->dyn.x) + 0.5;
		// Come as pellets
		if (next_square == 1 && map->m[(int)(pacman->dyn.y)][(int)(pacman->dyn.x+pacman->dyn.direction_x*(pacman->size+map->pellet_rad-0.5))] == 1) {
			map->m[(int)(pacman->dyn.y)][(int)(pacman->dyn.x+pacman->dyn.direction_x*(pacman->size+map->pellet_rad-0.5))] = 2;
			pacman->points += 10;
		}
		// Come as vitaminas
		if (next_square == 3 && map->m[(int)(pacman->dyn.y)][(int)(pacman->dyn.x+pacman->dyn.direction_x*(pacman->size+map->vitamin_rad-0.5))] == 3) {
			map->m[(int)(pacman->dyn.y)][(int)(pacman->dyn.x+pacman->dyn.direction_x*(pacman->size+map->vitamin_rad-0.5))] = 2;
			pacman->vitamin = true;
		}
	} else if (pacman->dyn.direction_y) {
		// Movimento
		pacman->dyn.x = (int)(pacman->dyn.x) + 0.5; // Centraliza
		pacman->dyn.y += pacman->dyn.direction_y * pacman->dyn.v / FPS; // Move
		if (pacman->dyn.y < pacman->size) // Faz o túnel, une as paredes esquerda e direita
			pacman->dyn.y = map->h - pacman->size;
		else if (pacman->dyn.y + pacman->size > map->h)
			pacman->dyn.y = pacman->size;
		next_square = map->m[(int)(pacman->dyn.y+pacman->dyn.direction_y*0.5)][(int)(pacman->dyn.x)];
		if (!next_square) // Se parede, não anda
			pacman->dyn.y = (int)(pacman->dyn.y) + 0.5;
		// Come as pellets
		if (next_square == 1 && map->m[(int)(pacman->dyn.y+pacman->dyn.direction_y*(pacman->size+map->pellet_rad-0.5))][(int)(pacman->dyn.x)] == 1) {
			map->m[(int)(pacman->dyn.y+pacman->dyn.direction_y*(pacman->size+map->pellet_rad-0.5))][(int)(pacman->dyn.x)] = 2;
			pacman->points += 10;
		}
		// Come as vitaminas
		if (next_square == 3 && map->m[(int)(pacman->dyn.y+pacman->dyn.direction_y*(pacman->size+map->vitamin_rad-0.5))][(int)(pacman->dyn.x)] == 3) {
			map->m[(int)(pacman->dyn.y+pacman->dyn.direction_y*(pacman->size+map->vitamin_rad-0.5))][(int)(pacman->dyn.x)] = 2;
			pacman->vitamin = true;
		}
	}
//	move(map, &(pacman->dyn));
}

/*-------------------------------------------------------------------------------------------------------------------------*/

void move_ghosts (Map *map, Ghost *ghosts, int *ghosts_n) {
	for (int i = 0; i < *ghosts_n; i++)
		if (ghosts[i].dyn.direction_x) {
			// Movimento
			ghosts[i].dyn.x += ghosts[i].dyn.direction_x * ghosts[i].dyn.v / FPS; // Move
			ghosts[i].dyn.y = (int)(ghosts[i].dyn.y) + 0.5; // Centraliza
			if (ghosts[i].dyn.x < ghosts[i].size) // Faz o túnel, une as paredes esquerda e direita
				ghosts[i].dyn.x = map->w - 0.5;
			else if (ghosts[i].dyn.x + ghosts[i].size > map->w)
				ghosts[i].dyn.x = ghosts[i].size;
			if (!map->m[(int)(ghosts[i].dyn.y)][(int)(ghosts[i].dyn.x+ghosts[i].dyn.direction_x*0.5)]) // Se parede, não anda
				ghosts[i].dyn.x = (int)(ghosts[i].dyn.x) + ghosts[i].size;
		} else if (ghosts[i].dyn.direction_y) {
			// Movimento
			ghosts[i].dyn.x = (int)(ghosts[i].dyn.x) + 0.5; // Centraliza
			ghosts[i].dyn.y += ghosts[i].dyn.direction_y * ghosts[i].dyn.v / FPS; // Move
			if (ghosts[i].dyn.y < ghosts[i].size) // Faz o túnel, une as paredes esquerda e direita
				ghosts[i].dyn.y = map->h - 0.5;
			else if (ghosts[i].dyn.y + ghosts[i].size > map->h)
				ghosts[i].dyn.y = ghosts[i].size;
			if (!map->m[(int)(ghosts[i].dyn.y+ghosts[i].dyn.direction_y*0.5)][(int)(ghosts[i].dyn.x)]) // Se parede, não anda
				ghosts[i].dyn.y = (int)(ghosts[i].dyn.y) + ghosts[i].size;
		}
}
