#include"pacman.h"

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
	map->x_fac = 1.0*(map->x_f - map->x_i)/map->w;
	map->y_fac = 1.0*(map->y_f - map->y_i)/map->h;

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
	int defeat_active = 0; // Indica se há derrota para se relacionar com a tecla enter
	double vitamin_time = 0.0;
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
			// DEBUG, aka impaciência
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
			// Pausa o jogo --> Todo
			if (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
				//*running = false;
				// Volta ao menu principal
				*menu_id = 0;
				game_running = false;
			}
			// Após colidir com um fantasma
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
			*sprite_timer += 1.0 / FPS;
			// Muda os sprites
			if (*sprite_timer >= *sprite_delay) {
				pacman.frame = (pacman.frame + 1) % PACMAN_SPRITE_COLS; // Avança para o próximo frame (Pac man)
				for (int i = 0; i < ghosts_n; i++){
					ghosts[i].frame = (ghosts[i].frame + 1) % GHOST_SPRITE_COLS; // Avança para o próximo frame (Ghosts)
				}
			*sprite_timer = 0.0;
			}
			// Timer do efeito da vitamina
			if (vitamin_time > 0.0)
				vitamin_time -= 1.0 / FPS;
			else {
				vitamin_time = 0.0;
				pacman.vitamin = false;
				for (int i = 0; i < ghosts_n; i++)
					ghosts[i].vulnerable = false;
			}
			// Movimento e detecção da vitamina
			if (move_pacman(map, &pacman)) {
				for (int i = 0; i < ghosts_n; i++)
					ghosts[i].vulnerable = true;
				vitamin_time = 15.0;
			}
			move_ghosts(map, nodemap, ghosts, &ghosts_n); // Move fantasmas

			verify_defeat(&pacman, ghosts, &ghosts_n, &defeat_active);

			// Indica que a tela deve ser atualizada
			redraw = true;
			break;
		}
		// Atualiza a tela
		if (redraw && al_is_event_queue_empty(*queue)) {
			game_show(map, &font, b, &b_n, &select, &pacman, ghosts, &ghosts_n, &width, &height);
			redraw = false;
		}
	}
	// Libera os espaços alocados e volta para o menu principal
	free(b);
	free(ghosts);
	free_map(map);
	return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------*/

// Design (by Ariel)
void game_show (Map *map, ALLEGRO_FONT **font, const Button *b, const int *b_n, const int *select, Pacman *pacman, Ghost *ghosts, const int *ghosts_n, int *width, int *height) {
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
			case 5: // Empty (Ariel)
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
		al_draw_scaled_bitmap(ghosts[i].sprite, ghosts[i].frame * SPRITE_SIZE, ghosts[i].movement * SPRITE_SIZE, SPRITE_SIZE, SPRITE_SIZE, map->x_i+map->x_fac*(ghosts[i].dyn.x-ghosts[i].size), map->y_i+map->y_fac*(ghosts[i].dyn.y-ghosts[i].size), map->x_fac*(2*ghosts[i].size), map->y_fac*(2*ghosts[i].size), 0);

	char show_points[50]; // Espaço para armazenar o texto formatado
	char show_lives[10];
	sprintf(show_points, "Points: %d", pacman->points);
	sprintf(show_lives, "Lifes: %d", pacman->lives);

	al_draw_text(*font, al_map_rgb(255, 255, 255), *width-275, *height-100, ALLEGRO_ALIGN_LEFT, show_points);
	al_draw_text(*font, al_map_rgb(255, 255, 255), *width-275, *height-150, ALLEGRO_ALIGN_LEFT, show_lives);
	al_flip_display();
}

/*-------------------------------------------------------------------------------------------------------------------------*/

Ghost* get_entities (Map *map, Pacman *pacman, int *ghosts_n) {
	Ghost* ghosts;
	// Inicializa pacman e fantasmas, de acordo com o mapa
	switch (map->id) {
	default:
		// Pacman
		*pacman = (Pacman){(Dynamics){0.0, 0.0, 5.0, 0, 0}, 0, 0.51, false, 3, 0, 0, NULL};
		pacman->sprite = al_load_bitmap("../../sprites/Pac Man.png");
		pacman->dyn.x = map->w/2.0;
		pacman->dyn.y = 24.5;

		// Fantasmas
		*ghosts_n = 4; // Número de fantasmas
		char *ghosts_path[4] = {"../../sprites/Ghost_Blue.png", "../../sprites/Ghost_Green.png", "../../sprites/Ghost_Purple.png", "../../sprites/Ghost_Red.png"};
		ghosts = malloc(*ghosts_n * sizeof(Ghost));
		for (int i = 0; i < *ghosts_n; i++) {
			ghosts[i] = (Ghost){(Dynamics){0.0, 0.0, 5.5, 0, 0}, 0.51, false, 0, 0,(NodeCoord) {0, 0}, NULL};
			ghosts[i].sprite = al_load_bitmap(ghosts_path[i]);
		}
		// Centraliza os fantasmas
		for (int i = 0; i < *ghosts_n; i++) {
			if (!ghosts[i].sprite) {// Retorna erro se não achar os sprites
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

bool move_pacman (Map *map, Pacman *pacman) {
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
		next_square = map->m[(int)(pacman->dyn.y)][(int)(pacman->dyn.x+pacman->dyn.direction_x*pacman->size)]; // Detecta o que há à frente
		if (!next_square) // Se parede, volta e centraliza
			pacman->dyn.x = (int)(pacman->dyn.x) + pacman->size;
		// Come as pellets
		if (next_square == 1 && map->m[(int)(pacman->dyn.y)][(int)(pacman->dyn.x+pacman->dyn.direction_x*(pacman->size+map->pellet_rad-0.5))] == 1) {
			map->m[(int)(pacman->dyn.y)][(int)(pacman->dyn.x+pacman->dyn.direction_x*(pacman->size+map->pellet_rad-0.5))] = 2;
			map->pellet_n--;
			pacman->points += 10;
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
		next_square = map->m[(int)(pacman->dyn.y+pacman->dyn.direction_y*pacman->size)][(int)(pacman->dyn.x)]; // Detecta o que há à frente
		if (!next_square) // Se parede, volta e centraliza
				pacman->dyn.y = (int)(pacman->dyn.y) + pacman->size;
		// Come as pellets
		if (next_square == 1 && map->m[(int)(pacman->dyn.y+pacman->dyn.direction_y*(pacman->size+map->pellet_rad-0.5))][(int)(pacman->dyn.x)] == 1) {
			map->m[(int)(pacman->dyn.y+pacman->dyn.direction_y*(pacman->size+map->pellet_rad-0.5))][(int)(pacman->dyn.x)] = 2;
			map->pellet_n--;
			pacman->points += 10;
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

/* Essa função é executada a todo momento. 
 * Se ghosts[i].dyn.direction_x = 1, se move para a direita. Se é -1, se move para a esquerda.
 * Se ghosts[i].dyn.direction_y= 1, se move para cima. Se é -1, se move para baixo.
*/

// void move_ghosts (Map *map, Ghost *ghosts, int *ghosts_n) {
// 	for (int i = 0; i < *ghosts_n; i++){
// 		int random = rand() % 100; // Gera número entre 0 e 100

// 		// Movimento inicial
// 		if (!ghosts[i].dyn.direction_x && !ghosts[i].dyn.direction_y){
// 			if (random < 25){
// 				ghosts[i].dyn.direction_x = 1;
// 			} else if (random < 50) {
// 				ghosts[i].dyn.direction_x = -1;
// 			} else if (random < 75) {
// 				ghosts[i].dyn.direction_y = 1; 
// 			} else {
// 				ghosts[i].dyn.direction_y = -1;
// 			}
// 		}
		
// 		//basic_move(&ghosts[i], map);

// 		// PRETENSÃO DE MOVIMENTO EM X
// 		if (ghosts[i].dyn.direction_x) {
// 			// Movimento
// 			ghosts[i].dyn.x += ghosts[i].dyn.direction_x * ghosts[i].dyn.v / FPS; // Move
// 			ghosts[i].dyn.y = (int)(ghosts[i].dyn.y) + 0.5; // Centraliza na outra direção

// 			if (ghosts[i].dyn.x < ghosts[i].size) // Faz o túnel, une as bordas esquerda e direita
// 				ghosts[i].dyn.x = map->w - ghosts[i].size;
// 			else if (ghosts[i].dyn.x + ghosts[i].size > map->w)
// 				ghosts[i].dyn.x = ghosts[i].size;
			
// 			// TRATAMENTO DE COLISÕES
// 			if (!map->m[(int)(ghosts[i].dyn.y)][(int)(ghosts[i].dyn.x+ghosts[i].dyn.direction_x*ghosts[i].size)]){ 
// 				ghosts[i].dyn.x = (int)(ghosts[i].dyn.x) + ghosts[i].size;
// 				// My code
// 				printf("\nColisão para esquerda/direita Identificada (AÇÃO)");

// 				ghosts[i].dyn.direction_x = 0;
// 				// Verifica se ambos (baixo e cima) estão livres (y)
// 				if(map->m[(int)(ghosts[i].dyn.y+ghosts[i].size)][(int)ghosts[i].dyn.x] && map->m[(int)(ghosts[i].dyn.y-ghosts[i].size)][(int)ghosts[i].dyn.x]){
// 					printf("\n	Aleatorio cima/baixo: %d, %d", map->m[(int)(ghosts[i].dyn.y+ghosts[i].size)][(int)ghosts[i].dyn.x], map->m[(int)(ghosts[i].dyn.y-ghosts[i].size)][(int)ghosts[i].dyn.x]);
// 					if(random < 50){
// 						ghosts[i].dyn.direction_y = 1;
// 						ghosts[i].movement = 1; // 1 é cima
// 					} else {
// 						ghosts[i].dyn.direction_y = -1;
// 						ghosts[i].movement = 3; // 3 é baixo
// 					}
// 				} else if(!map->m[(int)(ghosts[i].dyn.y+ghosts[i].size)][(int)ghosts[i].dyn.x]){ // Verifica colisão para cima
// 					printf("\n	Colisão para cima, baixo livre");
// 					ghosts[i].dyn.direction_y = -1;
// 					ghosts[i].movement = 3; // 3 é baixo
// 				} else{
// 					printf("\n	Colisão para baixo, cima livre");
// 					ghosts[i].dyn.direction_y = 1;
// 					ghosts[i].movement = 1; // 1 é cima
// 				}
// 				printf("\n	perp moved = false\n	colide priority = true");
// 				ghosts[i].perp_moved = false; // Permite se mover perpendicularmente após a colisão
// 				ghosts[i].colide_priority = true;
// 				// change_direction(&ghosts[i]); // Muda de direção ao bater
			
// 			// CASO NÃO HAJA COLISÃO (VERIFICA LADOS PERPENDICULARES)
// 			} else if(!ghosts[i].perp_moved && !ghosts[i].colide_priority) { 
// 				printf("\nSem colisão (AÇÃO)");
// 				// Verifica se ambos (baixo e cima) estão livres (y)
// 				if(map->m[(int)(ghosts[i].dyn.y+ghosts[i].size)][(int)ghosts[i].dyn.x] && map->m[(int)(ghosts[i].dyn.y-ghosts[i].size)][(int)ghosts[i].dyn.x]){
// 					printf("\n	Aleatorio cima/baixo: %d, %d", map->m[(int)(ghosts[i].dyn.y+ghosts[i].size)][(int)ghosts[i].dyn.x], map->m[(int)(ghosts[i].dyn.y-ghosts[i].size)][(int)ghosts[i].dyn.x]);
// 					if(90 < random < 95){
// 						ghosts[i].dyn.direction_y = 1;
// 						ghosts[i].movement = 1; // 1 é cima
// 					} else if(95 < random && random < 100) {
// 						ghosts[i].dyn.direction_y = -1;
// 						ghosts[i].movement = 3; // 3 é baixo
// 					}
// 				} else if(map->m[(int)(ghosts[i].dyn.y+ghosts[i].size)][(int)ghosts[i].dyn.x]){ // Verifica caminho livre para cima
// 					if(random > 0){
// 						printf("\n	Cima: %d, %d", map->m[(int)(ghosts[i].dyn.y+ghosts[i].size)][(int)ghosts[i].dyn.x], map->m[(int)(ghosts[i].dyn.y-ghosts[i].size)][(int)ghosts[i].dyn.x]);
// 						ghosts[i].dyn.direction_y = 1;
// 						ghosts[i].movement = 1; // 1 é cima
// 					}
// 				} else if(map->m[(int)(ghosts[i].dyn.y-ghosts[i].size)][(int)ghosts[i].dyn.x]){
// 					if(random > 0){
// 						printf("\n	Baixo: %d, %d", map->m[(int)(ghosts[i].dyn.y+ghosts[i].size)][(int)ghosts[i].dyn.x], map->m[(int)(ghosts[i].dyn.y-ghosts[i].size)][(int)ghosts[i].dyn.x]);
// 						ghosts[i].dyn.direction_y = -1;
// 						ghosts[i].movement = 3; // 3 é baixo
// 					}
// 				}
// 				if(ghosts[i].dyn.direction_y != 0){
// 					printf("\n	Perp moved = true");
// 					ghosts[i].dyn.direction_x = 0;
// 					ghosts[i].perp_moved = true;
// 				}
// 			}
// 			else if(ghosts[i].colide_priority){
// 				ghosts[i].colide_priority = false;
// 				printf("\n	colide priority = false");
// 			}
				
// 		// PRETENSÃO DE MOVIMENTO EM Y
// 		} else if (ghosts[i].dyn.direction_y) {
// 			// Movimento
// 			ghosts[i].dyn.x = (int)(ghosts[i].dyn.x) + 0.5; // Centraliza na outra direção
// 			ghosts[i].dyn.y += ghosts[i].dyn.direction_y * ghosts[i].dyn.v / FPS; // Move
			
// 			if (ghosts[i].dyn.y < ghosts[i].size) // Faz o túnel, une as bordas superior e inferior
// 				ghosts[i].dyn.y = map->h - ghosts[i].size;
// 			else if (ghosts[i].dyn.y + ghosts[i].size > map->h)
// 				ghosts[i].dyn.y = ghosts[i].size;
// 			if (!map->m[(int)(ghosts[i].dyn.y+ghosts[i].dyn.direction_y*ghosts[i].size)][(int)(ghosts[i].dyn.x)]){ // Se há colisão, faz tratamento
// 				ghosts[i].dyn.y = (int)(ghosts[i].dyn.y) + ghosts[i].size;
// 				// My code
// 				printf("\nColisão para cima/baixo Identificada (AÇÃO):");

// 				ghosts[i].dyn.direction_y = 0;
// 				// Verifica se ambos (esquerda e direita) estão livres (x)
// 				if(map->m[(int)ghosts[i].dyn.y][(int)(ghosts[i].dyn.x+ghosts[i].size)] && map->m[(int)ghosts[i].dyn.y][(int)(ghosts[i].dyn.x-ghosts[i].size)]){
// 					printf("\n	Aleatorio direita/esquerda, %d, %d", map->m[(int)ghosts[i].dyn.y][(int)(ghosts[i].dyn.x+ghosts[i].size)], map->m[(int)ghosts[i].dyn.y][(int)(ghosts[i].dyn.x-ghosts[i].size)]);
// 					if(random < 50){
// 						ghosts[i].dyn.direction_x = 1;
// 						ghosts[i].movement = 0; // 0 é direita
// 					}
// 					else {
// 						ghosts[i].dyn.direction_x = -1;
// 						ghosts[i].movement = 2; // 2 é esquerda
// 					}
// 				} else if(!map->m[(int)ghosts[i].dyn.y][(int)(ghosts[i].dyn.x+ghosts[i].size)]){ // Verifica colisão para a direita
// 					printf("\n	Colisão para a esquerda, direita livre");
// 					ghosts[i].dyn.direction_x = -1;
// 					ghosts[i].movement = 2; // 2 é esquerda
// 				} else{
// 					printf("\n	Colisão para a direita, esquerda livre");
// 					ghosts[i].dyn.direction_x = 1;
// 					ghosts[i].movement = 0; // 0 é direita
// 				}
// 				ghosts[i].perp_moved = false; // Permite se mover perpendicularmente após a colisão
// 				ghosts[i].colide_priority = true;
// 				// change_direction(&ghosts[i]); // Muda de direção ao bater
			
// 			// CASO NÃO HAJA COLISÃO (VERIFICA LADOS PERPENDICULARES)
// 			} else if(!ghosts[i].perp_moved && !ghosts[i].colide_priority){
// 				printf("\nSem colisão (AÇÃO)");
// 				// Verifica se ambos (esquerda e direita) estão livres (x)
// 				if(map->m[(int)ghosts[i].dyn.y][(int)(ghosts[i].dyn.x+ghosts[i].size)] && map->m[(int)ghosts[i].dyn.y][(int)(ghosts[i].dyn.x-ghosts[i].size)]){
// 					printf("\n	Aleatorio direita/esquerda, %d, %d", map->m[(int)ghosts[i].dyn.y][(int)(ghosts[i].dyn.x+ghosts[i].size)], map->m[(int)ghosts[i].dyn.y][(int)(ghosts[i].dyn.x-ghosts[i].size)]);
// 					if(90 < random && random < 95){	
// 						ghosts[i].dyn.direction_x = 1;
// 						ghosts[i].movement = 0; // 0 é direita
// 					}
// 					else if(90 <= random < 100) {
// 						ghosts[i].dyn.direction_x = -1;
// 						ghosts[i].movement = 2; // 2 é esquerda
// 					}
// 				} else if(map->m[(int)ghosts[i].dyn.y][(int)(ghosts[i].dyn.x+ghosts[i].size)]){ // Verifica caminho livre à direita
// 					printf("\n	Direita");
// 					if(random > 90){
// 						ghosts[i].dyn.direction_x = 1;
// 						ghosts[i].movement = 0; // 0 é direita
// 					}
// 				} else if(map->m[(int)ghosts[i].dyn.y][(int)(ghosts[i].dyn.x-ghosts[i].size)]){ // Verifica caminho livre à esquerda
// 					printf("\n	Esquerda");
// 					if(random > 90){
// 						ghosts[i].dyn.direction_x = -1;
// 						ghosts[i].movement = 2; // 2 é esquerda
// 					}
// 				}
// 				if(ghosts[i].dyn.direction_x != 0){
// 					printf("\n	Perp moved = true");
// 					ghosts[i].dyn.direction_y = 0;
// 					ghosts[i].perp_moved = true;
// 				}
// 			}
// 			else if(ghosts[i].colide_priority){
// 				ghosts[i].colide_priority = false;
// 				printf("\n	colide priority = false");
// 			}
// 		}
// 	}
// }

void move_ghosts (Map *map, NodeMap *nodemap, Ghost *ghosts, int *ghosts_n) {
	for (int i = 0; i < *ghosts_n; i++){
		// Movimento inicial
		if (!ghosts[i].dyn.direction_x && !ghosts[i].dyn.direction_y){
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

		if(((int) ghosts[i].dyn.y != ghosts[i].last_node.x) || ((int) ghosts[i].dyn.x != ghosts[i].last_node.y)){
			ghosts[i].last_node.x = -1;
			ghosts[i].last_node.y = -1;
		}

		// Define nova movimentação por nós (antes, verifica se é um nó e se esse nó não já originou uma decisão)
		if(isnode(nodemap, (int) ghosts[i].dyn.y, (int) ghosts[i].dyn.x) && (((int) ghosts[i].dyn.y != ghosts[i].last_node.x) || ((int) ghosts[i].dyn.x != ghosts[i].last_node.y))){
			bool flag = true;
			// Preso em loop aqui
			while(flag){
				// printf("\nLoop, node return: %d", isnode(nodemap, (int) ghosts[i].dyn.y, (int) ghosts[i].dyn.x));
				// printf("\nnodemap[x][y]: %d, %d", (int)ghosts[i].dyn.y, (int)ghosts[i].dyn.x);
				int random = rand() % 100; // Gera número entre 0 e 100
				int temp;

				// Maior chance de continuar na mesma direção, se possível
				// Não volta ao contrário da direção atual
				// 50 % de chance de virar na direção perpendicular
				if(ghosts[i].dyn.direction_x){
					if(ghosts[i].dyn.direction_x > 0){
						temp = 2;
					} else{
						temp = 3;
					}
					if (random < 50 && ghosts[i].dyn.direction_x && nodemap->m[(int)ghosts[i].dyn.y][(int)ghosts[i].dyn.x][temp]){
						//printf("\nNó -> Continua\n");
						flag = false;
					} else if (random >= 50 && random < 75 && nodemap->m[(int)ghosts[i].dyn.y][(int)ghosts[i].dyn.x][0]) {
						//printf("\nNó -> cima\n");
						ghosts[i].dyn.direction_y = 1; 
						ghosts[i].dyn.direction_x = 0;
						ghosts[i].movement = 1; // 1 é cima
						flag = false;
					} else if(random >= 75 && random <= 100 && nodemap->m[(int)ghosts[i].dyn.y][(int)ghosts[i].dyn.x][1]) {
						//printf("\nNó -> baixo\n");
						ghosts[i].dyn.direction_y = -1;
						ghosts[i].dyn.direction_x = 0;
						ghosts[i].movement = 3; // 3 é baixo
						flag = false;
					}
				}
				else if(ghosts[i].dyn.direction_y){
					if(ghosts[i].dyn.direction_y > 0){
						temp = 0;
					} else{
						temp = 1;
					}
					if (random < 50 && ghosts[i].dyn.direction_y && nodemap->m[(int)ghosts[i].dyn.y][(int)ghosts[i].dyn.x][temp]) {
						//printf("\nNó -> baixo\n");
						flag = false;
					}
					if (random >= 50 && random < 75 && nodemap->m[(int)ghosts[i].dyn.y][(int)ghosts[i].dyn.x][2]){
						//printf("\nNó -> Direita\n");
						ghosts[i].dyn.direction_x = 1;
						ghosts[i].dyn.direction_y = 0;
						ghosts[i].movement = 0; // 0 é direita
						flag = false;
					} else if (random >= 75 && random < 100 && nodemap->m[(int)ghosts[i].dyn.y][(int)ghosts[i].dyn.x][3]) {
						//printf("\nNó -> esquerda\n");
						ghosts[i].dyn.direction_x = -1;
						ghosts[i].dyn.direction_y = 0;
						ghosts[i].movement = 2; // 2 é esquerda
						flag = false;
					}
				}
			}
			ghosts[i].last_node.x = (int) ghosts[i].dyn.y;
			ghosts[i].last_node.y = (int) ghosts[i].dyn.x;
			//printf("Last node. x: %d, y: %d", ghosts[i].last_node.x, ghosts[i].last_node.y);
		}
		// Se não for nó, apenas continua com o movimento

		/*
			Onde for 5 no mapa (ou outro número), o fantasma apenas segue reto (não considera como nó). Onde for 1, o algoritmo pode classificar como nó.
			E também ativa somente o algoritmo de colisão.

			Ter nós um do lado do outro não funciona bem.

			Exemplo com parte inicial dos fantasmas

			0 0 2 0 2 0 0
			0 5 2 5 2 5 0
			0 5 5 5 5 5 0
			0 5 5 5 5 5 0
			0 0 0 0 0 0 0
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
			// TRATAMENTO DE COLISÕES
			if (!map->m[(int)(ghosts[i].dyn.y)][(int)(ghosts[i].dyn.x+ghosts[i].dyn.direction_x*ghosts[i].size)]){
				ghosts[i].dyn.x = (int)(ghosts[i].dyn.x) + ghosts[i].size;
				if(map->m[(int)(ghosts[i].dyn.y)][(int)(ghosts[i].dyn.x)] == 5)
					change_direction(&ghosts[i]);
				//printf("\nColisão");
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
			// TRATAMENTO DE COLISÕES
			if (!map->m[(int)(ghosts[i].dyn.y+ghosts[i].dyn.direction_y*ghosts[i].size)][(int)(ghosts[i].dyn.x)]){
				ghosts[i].dyn.y = (int)(ghosts[i].dyn.y) + ghosts[i].size;
				if(map->m[(int)(ghosts[i].dyn.y)][(int)(ghosts[i].dyn.x)] == 5)
					change_direction(&ghosts[i]);
				//printf("\nColisão");
			}
		}
	}
}

int isnode(NodeMap *nodemap, int x, int y){
	for(int i = 0; i < 4; i++){
		if(nodemap->m[x][y][i] != 0){
			//printf("\n[%d] -> diferente de zero", i);
			return 1;
		}
	}
	return 0;
}


/*-------------------------------------------------------------------------------------------------------------------------*/

// Inteligência de movimento do fantasma
void change_direction (Ghost *ghost) {
	int random = rand() % 100; // Gera número entre 0 e 100

	if(ghost->dyn.direction_x){
		ghost->dyn.direction_x = 0; // Reseta movimento

		if(random < 50){
			ghost->dyn.direction_y = 1;
			ghost->movement = 1; // 1 é cima
		} else {
			ghost->dyn.direction_y = -1;
			ghost->movement = 3; // 3 é baixo
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
