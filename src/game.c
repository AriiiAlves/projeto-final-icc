#include"pacman.h"

/*
 * Quando arescentar o sprite vulnerável,
 * basta:
 * - Apagar linhas 329 e 358
 * - Descomentar linhas 330 e 359
 * - Apagar este próprio comentário
 */

// Loop principal do jogo
int game (ALLEGRO_EVENT *ev, ALLEGRO_EVENT_QUEUE **queue, bool *running, Map *map, NodeMap *nodemap, ALLEGRO_FONT *font, int width, int height, ALLEGRO_TIMER **timer, double *sprite_timer, double *sprite_delay, int *menu_id) {
	// Inicializa entidades
	Pacman pacman;
	int ghosts_n;
	Ghost *ghosts = get_entities(map, &pacman, &ghosts_n);
	int g = ghosts_n;
	if (!ghosts) {// Encerra o programa se não achar os sprites
		printf("Failed to load sprites\n");
		*running = false;
		free_map(map);
		return 0;
	}
	// Essencialmente, número de pixels por quadrado do mapa
	if (0.88 * height * map->w / map->h <= 0.96 * width) {
		map->y_i = height*(0.54-0.44);
		map->y_f = height*(0.54+0.44);
		map->y_fac = 1.0*(map->y_f - map->y_i)/map->h;
		map->x_i = (int)(0.5 * width - map->y_fac * map->w / 2);
		map->x_f = (int)(0.5 * width + map->y_fac * map->w / 2);
		map->x_fac = 1.0*(map->x_f - map->x_i)/map->w;
	} else {
		map->x_i = width*(0.5-0.485);
		map->x_f = width*(0.5+0.485);
		map->x_fac = 1.0*(map->x_f - map->x_i)/map->w;
		map->y_i = (int)(0.54 * height - map->x_fac * map->h / 2);
		map->y_f = (int)(0.54 * height + map->x_fac * map->h / 2);
		map->y_fac = 1.0*(map->y_f - map->y_i)/map->h;
	}

	// Botões
	int b_n = 0;
	Button* b = NULL;
/*
	b = malloc(b_n * sizeof(Button));
	b[0] = (Button){width*(0.5-0.2), width*(0.5+0.2), height*(0.35-0.05), height*(0.35+0.05), false};
	b[1] = (Button){width*(0.5-0.2), width*(0.5+0.2), height*(0.46-0.05), height*(0.46+0.05), false};
	b[2] = (Button){width*(0.5-0.2), width*(0.5+0.2), height*(0.57-0.05), height*(0.57+0.05), false};
*/
	// Variáveis de controle e modos/estados do jogo
	int mouse_x, mouse_y;
	bool redraw = false;
	int select = -1;
	bool game_running = true; // Indica se continua rodando o jogo
	bool lost_life = false; // Indica que o jogador perdeu uma vida e interrompe o jogo (espera por enter)
	bool win = false;
	double vitamin_time = -0.1;
	bool *g1 = calloc(g, sizeof(bool));
	bool *g2 = calloc(g, sizeof(bool));

	// Loop principal = eventos + desenho da tela
	while (game_running) {
		al_wait_for_event(*queue, ev);
		// Tratamento dos eventos recebidos
		switch (ev->type) {
		case ALLEGRO_EVENT_DISPLAY_CLOSE:
			*running = false;
			break;
		case ALLEGRO_EVENT_MOUSE_AXES:
		case ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY:
			// Atualiza posição do mouse e detecta se está sobre algum botão
			mouse_x = ev->mouse.x;
			mouse_y = ev->mouse.y;
			for (int i = 0; i < g; i++)
				g1[i] = (mouse_x >= (map->x_i + map->x_fac*(ghosts[i].dyn.x-ghosts[i].size)) && mouse_x <= (map->x_i + map->x_fac*(ghosts[i].dyn.x+ghosts[i].size)) && mouse_y >= (map->y_i + map->y_fac*(ghosts[i].dyn.y-ghosts[i].size)) && mouse_y <= (map->y_i + map->y_fac*(ghosts[i].dyn.y+ghosts[i].size)));
			for (int i = 0; i < b_n; i++) {
				b[i].hover = (mouse_x >= b[i].x_i && mouse_x <= b[i].x_f && mouse_y >= b[i].y_i && mouse_y <= b[i].y_f);
				if (b[i].hover)
					select = i;
			}
			break;
		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
			for (int i = 0; i < g; i++)
				if (g1[i])
					g2[i] = 10;
			// Ao clicar sobre o botão, vai para o menu correspondente
			for (int i = 0; i < b_n; i++)
				if (b[i].hover)
					return i+1;
			break;
		case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
			for (int i = 0; i < ghosts_n; i++)
				if (g2[i]) {
					g2[i] = 0;
					mouse_x = (int)((mouse_x-map->x_i)/map->x_fac);
					mouse_y = (int)((mouse_y-map->y_i)/map->y_fac);
					if (map->m[mouse_y][mouse_x]) {
						ghosts[i].dyn.x = mouse_x + 0.5;
						ghosts[i].dyn.y = mouse_y + 0.5;
					}
				}
			break;
		case ALLEGRO_EVENT_KEY_DOWN:
			// Muda a direção do Pacman, mas não se for para cima de uma parede
			if (ev->keyboard.keycode == ALLEGRO_KEY_UP || ev->keyboard.keycode == ALLEGRO_KEY_W)
				if ((map->m[((int)(pacman.dyn.y-1)+map->w)%map->w][(int)(pacman.dyn.x)] % 4)) {
					pacman.dyn.direction_x = 0;
					pacman.dyn.direction_y = -1;
					pacman.movement = 2; // 2 é cima
				}
			if (ev->keyboard.keycode == ALLEGRO_KEY_LEFT || ev->keyboard.keycode == ALLEGRO_KEY_A)
				if ((map->m[(int)(pacman.dyn.y)][((int)(pacman.dyn.x-1)+map->h)%map->h] % 4)) {
					pacman.dyn.direction_x = -1;
					pacman.dyn.direction_y = 0;
					pacman.movement = 1; // 1 é esquerda
				}
			if (ev->keyboard.keycode == ALLEGRO_KEY_DOWN || ev->keyboard.keycode == ALLEGRO_KEY_S)
				if ((map->m[(int)(pacman.dyn.y+1)%map->h][(int)(pacman.dyn.x)] % 4)) {
					pacman.dyn.direction_x = 0;
					pacman.dyn.direction_y = 1;
					pacman.movement = 3; // 3 é cima
				}
			if (ev->keyboard.keycode == ALLEGRO_KEY_RIGHT || ev->keyboard.keycode == ALLEGRO_KEY_D)
				if ((map->m[(int)(pacman.dyn.y)][(int)(pacman.dyn.x+1)%map->w] % 4)) {
					pacman.dyn.direction_x = 1;
					pacman.dyn.direction_y = 0;
					pacman.movement = 0; // 0 é direita
				}
			// DEBUG, aka impaciência
			if (ev->keyboard.keycode == ALLEGRO_KEY_PAD_PLUS) {
				pacman.dyn.v += 1;
				for(int i = 0; i < ghosts_n; i++) {
					ghosts[i].dyn.v += 1;
				}
			}
			if (ev->keyboard.keycode == ALLEGRO_KEY_PAD_MINUS) {
				pacman.dyn.v -= 1;
				for(int i = 0; i < ghosts_n; i++) {
					ghosts[i].dyn.v -= 1;
				}
			}
			if (ev->keyboard.keycode == ALLEGRO_KEY_EQUALS)
				pacman.dyn.v += 1;
			if (ev->keyboard.keycode == ALLEGRO_KEY_MINUS)
				pacman.dyn.v -= 1;
			// Pausa o jogo --> Todo
			if (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
				//*running = false;
				// Volta ao menu principal
				game_running = false;
			}
			// Após colidir com um fantasma
			if (ev->keyboard.keycode == ALLEGRO_KEY_ENTER) {
				if (lost_life) {
					if (pacman.lives >= 1) {
						// Desativa indicador de derrota
						lost_life = false;
						vitamin_time = apply_vitamin(false, false, &pacman, ghosts, &ghosts_n, map, nodemap);
						// Retorna pacman e fantasmas à posição inicial e velocidades iniciais
						pacman.dyn.x = pacman.dyn.start_x;
						pacman.dyn.y = pacman.dyn.start_y;
						pacman.dyn.v = pacman.dyn.start_v;
						for (int i = 0; i < ghosts_n; i++) {
							ghosts[i].dyn.x = ghosts[i].dyn.start_x;
							ghosts[i].dyn.y = ghosts[i].dyn.start_y;
							ghosts[i].dyn.v = ghosts[i].dyn.start_v;
						}
					} else
						game_running = false; // Interrompe loop do jogo e volta ao menu principal
				}

				if (win) {
					win = false;
					pacman.dyn.x = pacman.dyn.start_x;
					pacman.dyn.y = pacman.dyn.start_y;
					pacman.dyn.v = pacman.dyn.start_v;
					pacman.dyn.direction_x = 0;
					pacman.dyn.direction_y = 0;
					pacman.vitamin = false;
					for (int i = 0; i < ghosts_n; i++) {
						ghosts[i].dyn.x = ghosts[i].dyn.start_x;
						ghosts[i].dyn.y = ghosts[i].dyn.start_y;
						ghosts[i].dyn.v = ++ghosts[i].dyn.start_v;
						ghosts[i].dyn.direction_x = 0;
						ghosts[i].dyn.direction_y = 0;
						ghosts[i].vulnerable = false;
					}
					int map_id = map->id;
					free_map(map);
					get_map(map_id, map);
				}
			}
			break;
		case ALLEGRO_EVENT_TIMER:
			// Atualiza a animação quando o tempo entre frames é atingido
			*sprite_timer += 0.5 / FPS;
			// Muda os sprites
			if (*sprite_timer >= *sprite_delay) {
				pacman.frame = (pacman.frame + 1) % PACMAN_SPRITE_COLS; // Avança para o próximo frame (Pac man)
				for (int i = 0; i < ghosts_n; i++) {
					if (ghosts[i].vulnerable && vitamin_time <= 3.0)
						ghosts[i].frame = (ghosts[i].frame + 1) % GHOST_VULNERABLE_SPRITE_COLS; // Avança para o próximo frame (Ghosts)
					else
						ghosts[i].frame = (ghosts[i].frame + 1) % GHOST_SPRITE_COLS; // Avança para o próximo frame (Ghosts)
				}
			*sprite_timer = 0.0;
			}
			// Timer do efeito da vitamina
			if (vitamin_time > 0.0) {
				vitamin_time -= 1.0 / FPS;
				if (vitamin_time <= 0.0)
					vitamin_time = apply_vitamin(false, false, &pacman, ghosts, &ghosts_n, map, nodemap);
			}
			// Movimento e detecção da vitamina
			if (move_pacman(map, &pacman, &win)) {
				if (vitamin_time > 0)
					vitamin_time = apply_vitamin(false, true, &pacman, ghosts, &ghosts_n, map, nodemap);
				vitamin_time = apply_vitamin(true, false, &pacman, ghosts, &ghosts_n, map, nodemap);
			}
			if (win)
				freeze(&pacman, ghosts, &ghosts_n);
			move_ghosts(map, nodemap, ghosts, &ghosts_n, &pacman); // Move fantasmas

			if (!lost_life)
				lost_life = check_death(&pacman, ghosts, &ghosts_n);

			// Indica que a tela deve ser atualizada
			redraw = true;
			break;
		}
		// Atualiza a tela
		if (redraw && al_is_event_queue_empty(*queue)) {
			game_show(map, &font, b, &b_n, &select, &pacman, ghosts, &ghosts_n, &width, &height, &lost_life, &win);
			redraw = false;
		}
	}
	// Libera os espaços alocados e volta para o menu principal
	free(g1);
	free(g2);
	free(b);
	free(ghosts);
	free_map(map);
	free_node_map(nodemap);
	return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------*/

// Design (by Ariel)
void game_show (Map *map, ALLEGRO_FONT **font, const Button *b, const int *b_n, const int *select, Pacman *pacman, Ghost *ghosts, const int *ghosts_n, int *width, int *height, bool *lost_life, bool *win) {
	al_clear_to_color(al_map_rgb(30, 40, 30));
	// Cores referentes ao mapa
	ALLEGRO_COLOR wall = al_map_rgb(0, 10, 100),
		      door = al_map_rgb(40, 80, 150),
		      empty = al_map_rgb(10, 10, 12),
		      pellet = al_map_rgb(250, 255, 255),
		      vitamin = al_map_rgb(250, 150, 150);
	// Borda do mapa (10px)
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
			case 5: // Empty (go forward)
			case 6: // Empty (special case)
			case 7: // Empty (special case)
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
	al_draw_scaled_bitmap(pacman->sprite, pacman->frame * SPRITE_SIZE, pacman->movement * SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE, map->x_i+map->x_fac*(pacman->dyn.x-pacman->size), map->y_i+map->y_fac*(pacman->dyn.y-pacman->size), map->x_fac*(2*pacman->size), map->y_fac*(2*pacman->size), 0);
	for (int i = 0; i < *ghosts_n; i++)
		if (ghosts[i].vulnerable)
			al_draw_scaled_bitmap(ghosts[i].sprite_vulnerable, ghosts[i].frame * SPRITE_SIZE, 0 * SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE, map->x_i+map->x_fac*(ghosts[i].dyn.x-ghosts[i].size), map->y_i+map->y_fac*(ghosts[i].dyn.y-ghosts[i].size), map->x_fac*(2*ghosts[i].size), map->y_fac*(2*ghosts[i].size), 0);
		else
			al_draw_scaled_bitmap(ghosts[i].sprite, ghosts[i].frame * SPRITE_SIZE, ghosts[i].movement * SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE, map->x_i+map->x_fac*(ghosts[i].dyn.x-ghosts[i].size), map->y_i+map->y_fac*(ghosts[i].dyn.y-ghosts[i].size), map->x_fac*(2*ghosts[i].size), map->y_fac*(2*ghosts[i].size), 0);

	char show_points[50]; // Espaço para armazenar o texto formatado
	char show_lives[10];
	sprintf(show_points, "Points: %d", pacman->points);
	sprintf(show_lives, "Lives: %d", pacman->lives);

	al_draw_text(*font, al_map_rgb(255, 255, 255), map->x_i, map->y_i - 50, ALLEGRO_ALIGN_LEFT, show_points);
	al_draw_text(*font, al_map_rgb(255, 255, 255), map->x_f, map->y_i - 50, ALLEGRO_ALIGN_RIGHT, show_lives);

	if (*lost_life)
		al_draw_text(*font, al_map_rgb(255, 255, 255), *width/2, *height/2, ALLEGRO_ALIGN_CENTER, "Oops. Press enter to continue.");
	if (*win)
		al_draw_text(*font, al_map_rgb(255, 255, 255), *width/2, *height/2, ALLEGRO_ALIGN_CENTER, "Nicely done!! Press enter to continue.");
	al_flip_display();
}

/*-------------------------------------------------------------------------------------------------------------------------*/

Ghost* get_entities (Map *map, Pacman *pacman, int *ghosts_n) {
	Ghost* ghosts;
	char *ghosts_path[4] = {"../../sprites/ghost_blue.png", "../../sprites/ghost_green.png", "../../sprites/ghost_purple.png", "../../sprites/ghost_red.png"};
	// Inicializa pacman e fantasmas, de acordo com o mapa
	switch (map->id) {
	default:
		// Pacman
		*pacman = (Pacman){(Dynamics){0.0, 0.0, PACMAN_V_0, 0.0, 0.0, PACMAN_V_0, 0, 0}, 0.51, 0, false, 3, 0, 0, NULL};
		pacman->sprite = al_load_bitmap("../../sprites/Pac_Man.png");
		pacman->dyn.start_x = map->w/2.0;
		pacman->dyn.start_y = 24.5;
		pacman->dyn.x = pacman->dyn.start_x;
		pacman->dyn.y = pacman->dyn.start_y;

		// Fantasmas
		*ghosts_n = 4; // Número de fantasmas
		ghosts = malloc(*ghosts_n * sizeof(Ghost));
		for (int i = 0; i < *ghosts_n; i++) {
			ghosts[i] = (Ghost){(Dynamics){0.0, 0.0, GHOSTS_V_0, 0.0, 0.0, GHOSTS_V_0, 0, 0}, 0.51, false, 0, 0, (NodeCoord){0, 0}, NULL, NULL};
			ghosts[i].sprite = al_load_bitmap(ghosts_path[i%4]);
			ghosts[i].sprite_vulnerable = al_load_bitmap("../../sprites/Pac_Man.png");
			ghosts[i].sprite_vulnerable = al_load_bitmap("../../sprites/ghost_vulnerable.png");
		}
		// Centraliza os fantasmas
		for (int i = 0; i < *ghosts_n; i++) {
			if (!ghosts[i].sprite) {// Retorna erro se não achar os sprites
				return NULL;
			}
			ghosts[i].dyn.start_x = (map->w - *ghosts_n + 1.0)/2.0+i;
			ghosts[i].dyn.start_y = 15;
			ghosts[i].dyn.x = ghosts[i].dyn.start_x;
			ghosts[i].dyn.y = ghosts[i].dyn.start_y;
		}
		break;
	case 1:
		// Pacman
		*pacman = (Pacman){(Dynamics){0.0, 0.0, PACMAN_V_0, 0.0, 0.0, PACMAN_V_0, 0, 0}, 0.51, 0, false, 3, 0, 0, NULL};
		pacman->sprite = al_load_bitmap("../../sprites/Pac_Man.png");
		pacman->dyn.start_x = 0.5;
		pacman->dyn.start_y = 0.5;
		pacman->dyn.x = pacman->dyn.start_x;
		pacman->dyn.y = pacman->dyn.start_y;

		// Fantasmas
		*ghosts_n = 30; // Número de fantasmas
		ghosts = malloc(*ghosts_n * sizeof(Ghost));
		for (int i = 0; i < *ghosts_n; i++) {
			ghosts[i] = (Ghost){(Dynamics){0.0, 0.0, GHOSTS_V_0, 0.0, 0.0, GHOSTS_V_0, 0, 0}, 0.51, false, 0, 0, (NodeCoord){0, 0}, NULL, NULL};
			ghosts[i].sprite = al_load_bitmap(ghosts_path[i%4]);
			ghosts[i].sprite_vulnerable = al_load_bitmap("../../sprites/Pac_Man.png");
			ghosts[i].sprite_vulnerable = al_load_bitmap("../../sprites/ghost_vulnerable.png");
		}
		// Inicializa a posição dos fantasmas
		{
			int j = 0;
			for (int i = 0; i < 7; i++, j++) {
				ghosts[j].dyn.start_x = 1.5 + 3*i;
				ghosts[j].dyn.start_y = 3.5 + i;
			}
			for (int i = 0; i < 7; i++, j++) {
				ghosts[j].dyn.start_x = 1.5 + 3*i;
				ghosts[j].dyn.start_y = map->h - (3.5 + i);
			}
			for (int i = 0; i < 7; i++, j++) {
				ghosts[j].dyn.start_x = map->w - (1.5 + 3*i);
				ghosts[j].dyn.start_y = map->h - (3.5 + i);
			}
			for (int i = 0; i < 7; i++, j++) {
				ghosts[j].dyn.start_x = map->w - (1.5 + 3*i);
				ghosts[j].dyn.start_y = 3.5 + i;
			}
			ghosts[j].dyn.start_x = map->w/2.0;
			ghosts[j++].dyn.start_y = 4.5;
			ghosts[j].dyn.start_x = map->w/2.0;
			ghosts[j++].dyn.start_y = map->h - 4.5;
		}
		for (int i = 0; i < *ghosts_n; i++) {
			if (!ghosts[i].sprite) {// Retorna erro se não achar os sprites
				return NULL;
			}
			ghosts[i].dyn.x = ghosts[i].dyn.start_x;
			ghosts[i].dyn.y = ghosts[i].dyn.start_y;
		}
		break;
	}
	return ghosts;
}

/*-------------------------------------------------------------------------------------------------------------------------*/

bool move_pacman (Map *map, Pacman *pacman, bool *win) {
	int next_square;
	// Move de acordo com a direção x/y do movimento
	if (pacman->dyn.direction_x) {
		// Movimento
		pacman->dyn.x += pacman->dyn.direction_x * pacman->dyn.v / FPS; // Move
		pacman->dyn.y = (int)(pacman->dyn.y) + 0.5; // Centraliza na outra direção
		if (pacman->dyn.x < pacman->size) // Faz o túnel, une as bordas esquerda e direita
			pacman->dyn.x = map->w - pacman->size;
		else if (pacman->dyn.x + pacman->size > map->w)
			pacman->dyn.x = pacman->size;
		next_square = map->m[(int)(pacman->dyn.y)][(int)(pacman->dyn.x+pacman->dyn.direction_x*pacman->size)] % 4; // Detecta o que há à frente
		if (!next_square) // Se parede, volta e centraliza
			pacman->dyn.x = (int)(pacman->dyn.x) + pacman->size;
		// Come as pellets
		if (next_square == 1 && map->m[(int)(pacman->dyn.y)][(int)(pacman->dyn.x+pacman->dyn.direction_x*(pacman->size+map->pellet_rad-0.5))] == 1) {
			map->m[(int)(pacman->dyn.y)][(int)(pacman->dyn.x+pacman->dyn.direction_x*(pacman->size+map->pellet_rad-0.5))] = 2;
			map->pellet_n--;
			pacman->points += 10;
			if (!map->pellet_n)
				*win = true;
		}
		// Come as vitaminas e retorna avisando que comeu
		if (next_square == 3 && map->m[(int)(pacman->dyn.y)][(int)(pacman->dyn.x+pacman->dyn.direction_x*(pacman->size+map->vitamin_rad-0.5))] == 3) {
			map->m[(int)(pacman->dyn.y)][(int)(pacman->dyn.x+pacman->dyn.direction_x*(pacman->size+map->vitamin_rad-0.5))] = 2;
			pacman->points += 50;
			pacman->vitamin = true;
			return true;
		}
	} else if (pacman->dyn.direction_y) {
		// Movimento
		pacman->dyn.x = (int)(pacman->dyn.x) + 0.5; // Centraliza na outra direção
		pacman->dyn.y += pacman->dyn.direction_y * pacman->dyn.v / FPS; // Move
		if (pacman->dyn.y < pacman->size) // Faz o túnel, une as bordas superior e inferior
			pacman->dyn.y = map->h - pacman->size;
		else if (pacman->dyn.y + pacman->size > map->h)
			pacman->dyn.y = pacman->size;
		next_square = map->m[(int)(pacman->dyn.y+pacman->dyn.direction_y*pacman->size)][(int)(pacman->dyn.x)] % 4; // Detecta o que há à frente
		if (!next_square) // Se parede, volta e centraliza
				pacman->dyn.y = (int)(pacman->dyn.y) + pacman->size;
		// Come as pellets
		if (next_square == 1 && map->m[(int)(pacman->dyn.y+pacman->dyn.direction_y*(pacman->size+map->pellet_rad-0.5))][(int)(pacman->dyn.x)] == 1) {
			map->m[(int)(pacman->dyn.y+pacman->dyn.direction_y*(pacman->size+map->pellet_rad-0.5))][(int)(pacman->dyn.x)] = 2;
			map->pellet_n--;
			pacman->points += 10;
			if (!map->pellet_n)
				*win = true;
		}
		// Come as vitaminas e retorna avisando que comeu
		if (next_square == 3 && map->m[(int)(pacman->dyn.y+pacman->dyn.direction_y*(pacman->size+map->vitamin_rad-0.5))][(int)(pacman->dyn.x)] == 3) {
			map->m[(int)(pacman->dyn.y+pacman->dyn.direction_y*(pacman->size+map->vitamin_rad-0.5))][(int)(pacman->dyn.x)] = 2;
			pacman->points += 50;
			pacman->vitamin = true;
			return true;
		}
	}
	return false;
}

/*-------------------------------------------------------------------------------------------------------------------------*/

// direction_y = 1 (baixo), direction_y = -1 (cima)
// Movements -> 0 é direita, 1 é baixo, 2 é esquerda, 3 é cima

void move_ghosts (Map *map, NodeMap *nodemap, Ghost *ghosts, int *ghosts_n, Pacman *pacman) {
	// Parâmetro de perseguição. Quanto maior mais difícil, mais movimentos em direção ao pac man (de 0 a 24)
	int pursuit_weight = 24; 

	// Se o parâmetro for fora do escopo, ignora
	if(pursuit_weight > 24 || pursuit_weight < 0)
		pursuit_weight = 0;
	
	// Se os fantasmas estão vulneráveis, ao invés de se aproximar, buscam fugir
	if (ghosts[0].vulnerable)
		pursuit_weight = -24;

	for (int i = 0; i < *ghosts_n; i++){
		// Movimento inicial aleatório (direção qualquer)
		if (!ghosts[i].dyn.direction_x && !ghosts[i].dyn.direction_y) {
			int random = rand() % 100; // Gera número entre 0 e 100

			if (random < 25){
				ghosts[i].dyn.direction_x = 1;
			} else if (random < 50) {
				ghosts[i].dyn.direction_x = -1;
			} else if (random < 75) {
				ghosts[i].dyn.direction_y = 1; 
			} else {
				ghosts[i].dyn.direction_y = -1;
			}
		}

		// Se for 7, apenas vai para cima (7 não é nó, apenas indicativo de direção)
		if(map->m[(int)(ghosts[i].dyn.y)][(int)(ghosts[i].dyn.x)] == 7){
				ghosts[i].dyn.direction_y = -1;
				ghosts[i].dyn.direction_x = 0;
				ghosts[i].movement = 3; // 3 é cima
		}

		// Se o fantasma não está mais no último nó utilizado, reseta as coordenadas last_node
		if (((int)ghosts[i].dyn.y != ghosts[i].last_node.x) || ((int)ghosts[i].dyn.x != ghosts[i].last_node.y)) {
			ghosts[i].last_node.x = -1;
			ghosts[i].last_node.y = -1;
		}

		// Define movimentação por nós (antes, verifica se é um nó e se esse nó não já originou uma decisão)
		if (is_node(nodemap, (int)ghosts[i].dyn.y, (int)ghosts[i].dyn.x) && (((int)ghosts[i].dyn.y != ghosts[i].last_node.x) || ((int)ghosts[i].dyn.x != ghosts[i].last_node.y))) {
			bool flag = true;

			while (flag) {
				int random = rand() % 100;
				int temp;

				// // DEBUG
				// printf("\n\nrandom: %d", random);
				// printf("\nghost: [%d]", i);
				// printf("\nnodemap[%d][%d] : [%d][%d][%d][%d]", (int)ghosts[i].dyn.y, (int)ghosts[i].dyn.x, nodemap->m[(int)ghosts[i].dyn.y][(int)ghosts[i].dyn.x][0], nodemap->m[(int)ghosts[i].dyn.y][(int)ghosts[i].dyn.x][1], nodemap->m[(int)ghosts[i].dyn.y][(int)ghosts[i].dyn.x][2], nodemap->m[(int)ghosts[i].dyn.y][(int)ghosts[i].dyn.x][3]);
				// printf("\ndirection x: %d, direction y: %d", ghosts[i].dyn.direction_x, ghosts[i].dyn.direction_y);
				// Maior chance de continuar na mesma direção, se possível
				// Não volta ao contrário da direção atual
				// 50 % de chance de virar na direção perpendicular

				// Se movimentação atual é na horizontal
				if (ghosts[i].dyn.direction_x) {
					// Verifica movimentação atual
					if (ghosts[i].dyn.direction_x > 0)
						temp = 2; // Direita
					else
						temp = 3; // Esquerda

					// Peso de perseguição (ajusta probabilidades)
					if(pacman->dyn.y > ghosts[i].dyn.y)
						pursuit_weight *= -1; // Maior probabilidade de ir para baixo
					else if(pacman->dyn.y == ghosts[i].dyn.y)
						pursuit_weight = 0; // Sem ajuste

					if (random < 50 && ghosts[i].dyn.direction_x && nodemap->m[(int)ghosts[i].dyn.y][(int)ghosts[i].dyn.x][temp]) {
						//printf("\nNó -> Continua\n");
						flag = false;
					} else if (random >= 50 && random < (75 + pursuit_weight) && nodemap->m[(int)ghosts[i].dyn.y][(int)ghosts[i].dyn.x][0]) {
						//printf("\nPac man para cima.\n");
						ghosts[i].dyn.direction_y = -1; 
						ghosts[i].dyn.direction_x = 0;
						ghosts[i].movement = 3; // 3 é cima
						flag = false;
					} else if(random >= (75 + pursuit_weight) && random <= 100 && nodemap->m[(int)ghosts[i].dyn.y][(int)ghosts[i].dyn.x][1]) {
						//printf("\nPac man para baixo.\n");
						ghosts[i].dyn.direction_y = 1;
						ghosts[i].dyn.direction_x = 0;
						ghosts[i].movement = 1; // 1 é baixo
						flag = false;
					}
				}
				// Se movimentação atual é na vertical
				else if (ghosts[i].dyn.direction_y){
					if (ghosts[i].dyn.direction_y < 0) {
						temp = 0; // Cima
					} else{
						temp = 1; // Baixo
					}

					// Peso de perseguição (ajusta probabilidades)
					if(pacman->dyn.x < ghosts[i].dyn.x)
						pursuit_weight *= -1; // Maior probabilidade de ir para a esquerda
					else if(pacman->dyn.x == ghosts[i].dyn.x)
						pursuit_weight = 0; // Sem ajuste
					
					// Continua movimento atual
					if (random < 50 && ghosts[i].dyn.direction_y && nodemap->m[(int)ghosts[i].dyn.y][(int)ghosts[i].dyn.x][temp])
						flag = false;
					// Para a direita
					if (random >= 50 && random < (75 + pursuit_weight) && nodemap->m[(int)ghosts[i].dyn.y][(int)ghosts[i].dyn.x][2]) {
						//printf("\nPac man para a direita.\n");
						ghosts[i].dyn.direction_x = 1;
						ghosts[i].dyn.direction_y = 0;
						ghosts[i].movement = 0; // 0 é direita
						flag = false;
					// Para a esquerda
					} else if (random >= (75 + pursuit_weight) && random < 100 && nodemap->m[(int)ghosts[i].dyn.y][(int)ghosts[i].dyn.x][3]) {
						//printf("\nPac man para a esquerda.\n");
						ghosts[i].dyn.direction_x = -1;
						ghosts[i].dyn.direction_y = 0;
						ghosts[i].movement = 2; // 2 é esquerda
						flag = false;
					}
				}
			}
			ghosts[i].last_node.x = (int) ghosts[i].dyn.y;
			ghosts[i].last_node.y = (int) ghosts[i].dyn.x;
		}
		// Se não for nó, apenas continua com o movimento

		/*
			Onde for 5 ou 7 no mapa, o fantasma apenas segue reto (não considera como nó) E permite somente o algoritmo de colisão.. Onde for 1/2/3, o algoritmo pode classificar como nó.
			Onde for 7, especifica-se ir para cima.

			Ter nós um do lado do outro não funciona bem. Use 5 para regiões adjascentes ao nó pretendido (quadrado onde é possível mudar a direção de acordo com os blocos disponíveis adjascentes)
		*/

		// PRETENSÃO DE MOVIMENTO EM X
		if (ghosts[i].dyn.direction_x) {
			// Movimento
			ghosts[i].dyn.x += ghosts[i].dyn.direction_x * ghosts[i].dyn.v / FPS; // Move
			ghosts[i].dyn.y = (int)(ghosts[i].dyn.y) + 0.5; // Centraliza na outra direção

			if (ghosts[i].dyn.x < ghosts[i].size) // Faz o túnel, une as bordas esquerda e direita
				ghosts[i].dyn.x = map->w - ghosts[i].size;
			else if (ghosts[i].dyn.x + ghosts[i].size > map->w)
				ghosts[i].dyn.x = ghosts[i].size;
			if (!(map->m[(int)(ghosts[i].dyn.y)][(int)(ghosts[i].dyn.x+ghosts[i].dyn.direction_x*ghosts[i].size)]%4)){ // Se parede,  volta e muda a direção
				ghosts[i].dyn.x = (int)(ghosts[i].dyn.x) + ghosts[i].size;
				// Se é 5 ou 7, ativa algoritmo de colisão se necessário
				if(map->m[(int)(ghosts[i].dyn.y)][(int)(ghosts[i].dyn.x)] == 5 || map->m[(int)(ghosts[i].dyn.y)][(int)(ghosts[i].dyn.x)] == 7)
					change_direction(&ghosts[i], pacman, pursuit_weight);
			}
				
		// PRETENSÃO DE MOVIMENTO EM Y
		} else if (ghosts[i].dyn.direction_y) {
			// Movimento
			ghosts[i].dyn.x = (int)(ghosts[i].dyn.x) + 0.5; // Centraliza na outra direção
			ghosts[i].dyn.y += ghosts[i].dyn.direction_y * ghosts[i].dyn.v / FPS; // Move
			
			if (ghosts[i].dyn.y < ghosts[i].size) // Faz o túnel, une as bordas superior e inferior
				ghosts[i].dyn.y = map->h - ghosts[i].size;
			else if (ghosts[i].dyn.y + ghosts[i].size > map->h)
				ghosts[i].dyn.y = ghosts[i].size;
			if (!(map->m[(int)(ghosts[i].dyn.y+ghosts[i].dyn.direction_y*ghosts[i].size)][(int)(ghosts[i].dyn.x)]%4)){ // Se parede, volta e muda a direção
				ghosts[i].dyn.y = (int)(ghosts[i].dyn.y) + ghosts[i].size;
				// Se é 5 ou 7, ativa algoritmo de colisão se necessário
				if(map->m[(int)(ghosts[i].dyn.y)][(int)(ghosts[i].dyn.x)] == 5 || map->m[(int)(ghosts[i].dyn.y)][(int)(ghosts[i].dyn.x)] == 7)
					change_direction(&ghosts[i], pacman, pursuit_weight);
			}
		}
	}
}

/*-------------------------------------------------------------------------------------------------------------------------*/

// Verifica se x,y é um nó
bool is_node (NodeMap *nodemap, int x, int y) {
	for (int i = 0; i < 4; i++)
		if (nodemap->m[x][y][i] != 0)
			return true;
	return false;
}

/*-------------------------------------------------------------------------------------------------------------------------*/

// Inteligência de movimento do fantasma
void change_direction (Ghost *ghost, Pacman *pacman, int pursuit_weight) {
	int random = rand() % 100; // Gera número entre 0 e 100

	if (ghost->dyn.direction_x) {
		ghost->dyn.direction_x = 0; // Reseta movimento

		// Ajuste de pesos de perseguição
		if(pacman->dyn.y > ghost->dyn.y)
			pursuit_weight *= -1; // Maior probabilidade de ir para baixo
		else if(pacman->dyn.y == ghost->dyn.y)
			pursuit_weight = 0; // Sem ajuste
		
		if (random < 50 + pursuit_weight*2) {
			ghost->dyn.direction_y = -1;
			ghost->movement = 3; // 3 é cima
		} else {
			ghost->dyn.direction_y = 1;
			ghost->movement = 1; // 1 é baixo
		}
	}
	else if (ghost->dyn.direction_y) {
		ghost->dyn.direction_y = 0; // Reseta movimento

		// Ajuste de pesos de perseguição
		if(pacman->dyn.x < ghost->dyn.x)
			pursuit_weight *= -1; // Maior probabilidade de ir para a esquerda
		else if(pacman->dyn.x == ghost->dyn.x)
			pursuit_weight = 0; // Sem ajuste

		if (random < 50 + pursuit_weight*2) {
			ghost->dyn.direction_x = 1;
			ghost->movement = 0; // 0 é direita
		} else {
			ghost->dyn.direction_x = -1;
			ghost->movement = 2; // 2 é esquerda
		}
	}
}

/*-------------------------------------------------------------------------------------------------------------------------*/

double apply_vitamin (bool turn_on_effect, bool second_vitamin, Pacman *pacman, Ghost *ghosts, int *ghosts_n, Map *map, NodeMap *nodemap) {
	double time;
	if (turn_on_effect) {
		pacman->dyn.v *= 1.5;
		for (int i = 0; i < *ghosts_n; i++) {
			ghosts[i].vulnerable = true;
			ghosts[i].dyn.v *= 0.6;
		}

		switch (map->id) {
		default:
			time = 10.0;
			break;
		case 1:
			for (int i = 0; i < map->h; i++)
				for (int j = 0; j < map->w; j++)
					if (map->m[i][j] == 8)
						map->m[i][j] = 10;
			free_node_map(nodemap);
			get_node_map(map, nodemap);
			time = 15.0;
			break;
		}
	} else {
		pacman->vitamin = false;
		pacman->dyn.v = pacman->dyn.start_v;
		for (int i = 0; i < *ghosts_n; i++) {
			ghosts[i].vulnerable = false;
			ghosts[i].dyn.v = ghosts[i].dyn.start_v;
		}
		time = -0.1;

		// Mapa de id 1 precisa de detalhes extras
		if (map->id == 1) {
			// Se for a segunda ou mais vitamina em seguida, não restaura a posição
			// Se o tempo acabou mesmo, restaura a posição inicial (para evitar ficar preso)
			if (!second_vitamin) {
				pacman->dyn.x = pacman->dyn.start_x;
				pacman->dyn.y = pacman->dyn.start_y;
			}
			for (int i = 0; i < map->h; i++)
				for (int j = 0; j < map->w; j++)
					if (map->m[i][j] == 10)
						map->m[i][j] = 8;
			free_node_map(nodemap);
			get_node_map(map, nodemap);
		}
	}
	return time;
}

/*-------------------------------------------------------------------------------------------------------------------------*/

// Verifica se há colisão. Se há, congela todos os personagens e o jogador tem que apertar enter para iniciar de novo com outra vida.
bool check_death (Pacman *pacman, Ghost *ghosts, int *ghosts_n) {
	// Verifica se o jogador trombou com algum fantasma
	for (int i = 0; i < *ghosts_n; i++)
		if ((int)pacman->dyn.x == (int)ghosts[i].dyn.x && (int)pacman->dyn.y == (int)ghosts[i].dyn.y) {
			if (ghosts[i].vulnerable) {
				pacman->points += 200;
				ghosts[i].vulnerable = false;
				ghosts[i].dyn.x = ghosts[i].dyn.start_x;
				ghosts[i].dyn.y = ghosts[i].dyn.start_y;
				ghosts[i].dyn.v = ghosts[i].dyn.start_v;
			} else {
				// Pausa personagens
				freeze(pacman, ghosts, ghosts_n);
				// Perde vida
				pacman->lives--;
				// Avisa que perdeu vida
				return true;
			}
		}
	return false;
}

/*-------------------------------------------------------------------------------------------------------------------------*/

void freeze (Pacman *pacman, Ghost *ghosts, int *ghosts_n) {
	for (int i = 0; i < *ghosts_n; i++)
		ghosts[i].dyn.v = 0;
	pacman->dyn.v = 0;
}
