#include"pacman.h"

int game (ALLEGRO_EVENT *ev, ALLEGRO_EVENT_QUEUE **queue, bool *running, Map *map, ALLEGRO_FONT *font, int width, int height, ALLEGRO_TIMER **timer, double *sprite_timer, double *sprite_delay, int *menu_id) {
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
	bool game_running = true; // Indica se continua rodando o jogo
	int defeat_active = 0; // Indica se há derrota para se relacionar com a tecla enter

	while (*running && game_running) {
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
					pacman.movement = 2; // 2 é cima
				}
			if (ev->keyboard.keycode == ALLEGRO_KEY_LEFT || ev->keyboard.keycode == ALLEGRO_KEY_A)
				if ((map->m[(int)(pacman.dyn.y)][(int)(pacman.dyn.x-1)] % 4)) {
					pacman.dyn.direction_x = -1;
					pacman.dyn.direction_y = 0;
					pacman.movement = 1; // 1 é esquerda
				}
			if (ev->keyboard.keycode == ALLEGRO_KEY_DOWN || ev->keyboard.keycode == ALLEGRO_KEY_S)
				if ((map->m[(int)(pacman.dyn.y+1)][(int)(pacman.dyn.x)] % 4)) {
					pacman.dyn.direction_x = 0;
					pacman.dyn.direction_y = 1;
					pacman.movement = 3; // 3 é baixo
				}
			if (ev->keyboard.keycode == ALLEGRO_KEY_RIGHT || ev->keyboard.keycode == ALLEGRO_KEY_D)
				if ((map->m[(int)(pacman.dyn.y)][(int)(pacman.dyn.x+1)] % 4)) {
					pacman.dyn.direction_x = 1;
					pacman.dyn.direction_y = 0;
					pacman.movement = 0; // 0 é direita
				}
			if (ev->keyboard.keycode == ALLEGRO_KEY_PAD_PLUS){
				pacman.dyn.v += 1;
				for(int i = 0; i < ghosts_n; i++){
					ghosts[i].dyn.v += 1;
				}
			}
			if (ev->keyboard.keycode == ALLEGRO_KEY_PAD_MINUS){
				pacman.dyn.v -= 1;
				for(int i = 0; i < ghosts_n; i++){
					ghosts[i].dyn.v -= 1;
				}
			}
			if (ev->keyboard.keycode == ALLEGRO_KEY_EQUALS)
				pacman.dyn.v += 0.1;
			if (ev->keyboard.keycode == ALLEGRO_KEY_MINUS)
				pacman.dyn.v -= 0.1;
			if (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
				//*running = false;
				// Volta ao menu principal
				*menu_id = 0;
				game_running = false;
			}
			// NOVO (Após colidir com um fantasma)
			if (ev->keyboard.keycode == ALLEGRO_KEY_ENTER){
				if (defeat_active && pacman.lives >= 1){
					// Desativa indicador de derrota
					defeat_active = 0;
					// Retorna pacman e fantasmas à posição inicial
					pacman.dyn.x = map->w/2.0;
					pacman.dyn.y = 24.5;

					for (int i = 0; i < ghosts_n; i++) {
						ghosts[i].dyn.x = (map->w - ghosts_n + 1.0)/2.0+i;
						ghosts[i].dyn.y = 15;
					}

					// Devolve velocidades (qual a velocidade inicial mesmo?)
					pacman.dyn.v=5;
					for (int i = 0; i < ghosts_n; i++){
						ghosts[i].dyn.v=5;
					}
				}
				else if (defeat_active){
					// Volta ao menu principal
					defeat_active = 0; 
					// Interrompe loop do jogo
					game_running = false;
					// Desativa indicador de derrota
					menu_id = 0;
				}
			}
			break;
		case ALLEGRO_EVENT_TIMER:
			// Atualiza a animação quando o tempo entre frames é atingido
			*sprite_timer += 1.0 / 60.0;
			if (*sprite_timer >= *sprite_delay) {
				pacman.frame = (pacman.frame + 1) % PACMAN_SPRITE_COLS; // Avança para o próximo frame (Pac man)
				for (int i = 0; i < ghosts_n; i++){
					ghosts[i].frame = (ghosts[i].frame + 1) % GHOST_SPRITE_COLS; // Avança para o próximo frame (Ghosts)
				}
			*sprite_timer = 0.0;
			}
			move_pacman(map, &pacman);
			move_ghosts(map, ghosts, &ghosts_n); // Move fantasmas

			verify_defeat(&pacman, ghosts, &ghosts_n, &defeat_active);

			redraw = true; // Marca que a tela precisa ser redesenhada
			break;
		}
		if (redraw && al_is_event_queue_empty(*queue)) {
			game_show(map, &font, b, &b_n, &select, &pacman, ghosts, &ghosts_n, &width, &height);
			redraw = false;
		}
	}
	free(b);
	free(ghosts);
	free_map(map);
	return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------*/

// Design (by Ariel)
void game_show (Map *map, ALLEGRO_FONT **font, const Button *b, const int *b_n, const int *select, Pacman *pacman, Ghost *ghosts, const int *ghosts_n, int *width, int *height) {
	al_clear_to_color(al_map_rgb(30, 40, 30));
	ALLEGRO_COLOR wall = al_map_rgb(0, 10, 100),
		      door = al_map_rgb(40, 80, 150),
		      empty = al_map_rgb(10, 10, 12),
		      pellet = al_map_rgb(250, 255, 255),
		      vitamin = al_map_rgb(250, 150, 150);
	
	al_draw_filled_rectangle(map->x_i-10, map->y_i-10, map->x_f+10, map->y_f+10, al_map_rgb(0, 0, 0));

	// Desenhando itens do mapa
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

	// Desenhando Pac Man e fantasmas na tela
	
	al_draw_bitmap_region(pacman->sprite, pacman->frame * SPRITE_SIZE, pacman->movement * SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE, map->x_i+map->x_fac*(pacman->dyn.x-0.4), map->y_i+map->y_fac*(pacman->dyn.y-0.4), 0);
	for (int i = 0; i < *ghosts_n; i++)
		al_draw_bitmap_region(ghosts[i].sprite, ghosts[i].frame * SPRITE_SIZE, ghosts[i].movement * SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE, map->x_i+map->x_fac*(ghosts[i].dyn.x-0.4), map->y_i+map->y_fac*(ghosts[i].dyn.y-0.4), 0);

	char show_points[50]; // Espaço para armazenar o texto formatado
	char show_lives[10];
	sprintf(show_points, "Points: %d", pacman->points);
	sprintf(show_lives, "Lifes: %d", pacman->lives);

	al_draw_text(*font, al_map_rgb(255, 255, 255), *width-275, *height-100, ALLEGRO_ALIGN_LEFT, show_points);
	al_draw_text(*font, al_map_rgb(255, 255, 255), *width-275, *height-150, ALLEGRO_ALIGN_LEFT, show_lives);

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
		*pacman = (Pacman){(Dynamics){0.0, 0.0, 5.0, 0, 0}, 0, 0.25, false, 3, 0, 0, NULL};
		pacman->sprite = al_load_bitmap("../../sprites/Pac Man.png");
		pacman->dyn.x = map->w/2.0;
		pacman->dyn.y = 24.5;
		*ghosts_n = 4; // Número de fantasmas
		ghosts = malloc(*ghosts_n * sizeof(Ghost));
		ghosts[0] = (Ghost){(Dynamics){0.0, 0.0, 5.1, 0, 0}, 0.25, false, 0, 0, NULL};
		ghosts[1] = (Ghost){(Dynamics){0.0, 0.0, 5.1, 0, 0}, 0.25, false, 0, 0, NULL}; // Apenas um fantasma (DEBUG)
		ghosts[2] = (Ghost){(Dynamics){0.0, 0.0, 5.1, 0, 0}, 0.25, false, 0, 0, NULL};
		ghosts[3] = (Ghost){(Dynamics){0.0, 0.0, 5.1, 0, 0}, 0.25, false, 0, 0, NULL};
		ghosts[0].sprite = al_load_bitmap("../../sprites/Ghost Blue.png");
		ghosts[1].sprite = al_load_bitmap("../../sprites/Ghost Green.png");
		ghosts[2].sprite = al_load_bitmap("../../sprites/Ghost Purple.png");
		ghosts[3].sprite = al_load_bitmap("../../sprites/Ghost Red.png");
		for (int i = 0; i < *ghosts_n; i++) {
			//al_convert_mask_to_alpha(ghosts[i].sprite, al_map_rgb(255, 0, 255)); // Remove fundo magenta
			if (!ghosts[i].sprite){
				return NULL;
			}
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
		if (pacman->dyn.x < 0.5) // Faz o túnel, une as paredes esquerda e direita
			pacman->dyn.x = map->w - 0.5;
		else if (pacman->dyn.x + 0.5 > map->w)
			pacman->dyn.x = 0.5;
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
		if (pacman->dyn.y < 0.5) // Faz o túnel, une as paredes esquerda e direita
			pacman->dyn.y = map->h - 0.5;
		else if (pacman->dyn.y + 0.5 > map->h)
			pacman->dyn.y = 0.5;
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

/* Essa função é executada a todo momento. 
 * Se ghosts[i].dyn.direction_x = 1, se move para a direita. Se é -1, se move para a esquerda.
 * Se ghosts[i].dyn.direction_y= 1, se move para cima. Se é -1, se move para baixo.
*/

void move_ghosts (Map *map, Ghost *ghosts, int *ghosts_n) {

	for (int i = 0; i < *ghosts_n; i++){
		int random = rand() % 100; // Gera número entre 0 e 100

		// Movimento inicial
		if(!ghosts[i].dyn.direction_x && !ghosts[i].dyn.direction_y){
			if(random < 25){
				ghosts[i].dyn.direction_x = 1;
			} else if(random < 50){
				ghosts[i].dyn.direction_x = -1;
			} else if(random < 75){
				ghosts[i].dyn.direction_y = 1; 
			} else{
				ghosts[i].dyn.direction_y = -1;
			}
		}

		if (ghosts[i].dyn.direction_x) {
			// Movimento
			ghosts[i].dyn.x += ghosts[i].dyn.direction_x * ghosts[i].dyn.v / FPS; // Move
			ghosts[i].dyn.y = (int)(ghosts[i].dyn.y) + 0.5; // Centraliza
			if (ghosts[i].dyn.x < 0.5) // Faz o túnel, une as paredes esquerda e direita
				ghosts[i].dyn.x = map->w - 0.5;
			else if (ghosts[i].dyn.x + 0.5 > map->w)
				ghosts[i].dyn.x = 0.5;
			if (!map->m[(int)(ghosts[i].dyn.y)][(int)(ghosts[i].dyn.x+ghosts[i].dyn.direction_x*0.5)]){ // Se parede, não anda
				ghosts[i].dyn.x = (int)(ghosts[i].dyn.x) + ghosts[i].size;
				change_direction(&ghosts[i]); // Muda de direção ao bater
			}
				
		} else if (ghosts[i].dyn.direction_y) {
			// Movimento
			ghosts[i].dyn.x = (int)(ghosts[i].dyn.x) + 0.5; // Centraliza
			ghosts[i].dyn.y += ghosts[i].dyn.direction_y * ghosts[i].dyn.v / FPS; // Move
			if (ghosts[i].dyn.y < 0.5) // Faz o túnel, une as paredes esquerda e direita
				ghosts[i].dyn.y = map->h - 0.5;
			else if (ghosts[i].dyn.y + 0.5 > map->h)
				ghosts[i].dyn.y = 0.5;
			if (!map->m[(int)(ghosts[i].dyn.y+ghosts[i].dyn.direction_y*0.5)][(int)(ghosts[i].dyn.x)]){ // Se parede, não anda
				ghosts[i].dyn.y = (int)(ghosts[i].dyn.y) + ghosts[i].size;
				change_direction(&ghosts[i]); // Muda de direção ao bater
			}
		}
	}
}

/*-------------------------------------------------------------------------------------------------------------------------*/

// Funções novas

void change_direction (Ghost *ghost) {
	int random = rand() % 100; // Gera número entre 0 e 100

	if(ghost->dyn.direction_x){
		ghost->dyn.direction_x = 0; // Reseta movimento

		if(random < 50){
			ghost->dyn.direction_y = 1;
			ghost->movement = 1; // 3 é cima
		} else {
			ghost->dyn.direction_y = -1;
			ghost->movement = 3; // 1 é baixo
		}
	}
	else if(ghost->dyn.direction_y){
		ghost->dyn.direction_y = 0; // Reseta movimento

		if(random < 50){
			ghost->dyn.direction_x = 1;
			ghost->movement = 0; // 0 é direita
		} else {
			ghost->dyn.direction_x = -1;
			ghost->movement = 2; // 2 é esquerda
		}
	}
}

/*-------------------------------------------------------------------------------------------------------------------------*/

// Verifica se há colisão. Se há, para todos os personagens e o jogador tem que apertar enter para iniciar de novo com outra vida.

void verify_defeat (Pacman *pacman, Ghost *ghosts, int *ghosts_n, int *defeat_active) {
	// Verifica se o jogador perdeu
	for(int i = 0; i < *ghosts_n; i++){
		if((int)pacman->dyn.x == (int)ghosts[i].dyn.x  && (int)pacman->dyn.y == (int)ghosts[i].dyn.y && !*defeat_active){
			// Pausa personagens
			pacman->dyn.v=0;
			for(int i = 0; i < *ghosts_n; i++){
				ghosts[i].dyn.v=0;
			}

			// Perde vida
			pacman->lives -= 1;

			// Derrota ativa
			*defeat_active = 1;
//			printf("\nDerrota\n");
		}
	}
}
