#include"pacman.h"

// Inicializa o Allegro e o rand()
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
	// Põe em tela cheia e pega a resolução da tela
	al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
	ALLEGRO_DISPLAY_MODE mode;
	if (al_get_display_mode(0, &mode)) {
		*width = mode.width;
		*height = mode.height;
	}

	*display = al_create_display(*width, *height); // Cria display
	*queue = al_create_event_queue();              // Cria fila de eventos
	*timer = al_create_timer(1.0 / FPS);           // Cria timer
	// Registra as fontes de eventos que a fila vai monitorar
	al_register_event_source(*queue, al_get_display_event_source(*display));
	al_register_event_source(*queue, al_get_timer_event_source(*timer));
	al_register_event_source(*queue, al_get_mouse_event_source());
	al_register_event_source(*queue, al_get_keyboard_event_source());
	srand(time(NULL)); // Inicializa o rand() usando o timestamp atual como seed
}

/*-------------------------------------------------------------------------------------------------------------------------*/

// Destrói tudo do Allegro
void destroy_all (ALLEGRO_SAMPLE_INSTANCE **sample_instance, ALLEGRO_SAMPLE **sample, ALLEGRO_FONT **font, ALLEGRO_TIMER **timer, ALLEGRO_EVENT_QUEUE **queue, ALLEGRO_DISPLAY **display) {
	al_destroy_sample_instance(*sample_instance);
	al_destroy_sample(*sample);
	al_destroy_font(*font);
	al_destroy_timer(*timer);
	al_destroy_event_queue(*queue);
	al_destroy_display(*display);
}

/*-------------------------------------------------------------------------------------------------------------------------*/

// Inicializa as fontes, sons e imagens básicas do jogo
bool load_media (ALLEGRO_FONT **font, ALLEGRO_FONT **title_font, ALLEGRO_SAMPLE **menu_sample, ALLEGRO_SAMPLE **game_sample, ALLEGRO_BITMAP **background) {
	*font = al_load_ttf_font("../../fontes/PressStart.ttf", 19, 0);
	*title_font = al_load_ttf_font("../../fontes/PressStart.ttf", 60, 0);
	if (!(*font)) {
		printf("Erro ao carregar fonte.\n");
		return false;
	}

	// Som menu
	al_reserve_samples(1);
	*menu_sample = al_load_sample("../../sons/menu.wav");
	if (!(*menu_sample)) {
		printf("Erro ao carregar som.\n");
		return false;
	}

	// Som jogo
	al_reserve_samples(1);
	*game_sample = al_load_sample("../../sons/menu.wav");
	if (!(*game_sample)) {
		printf("Erro ao carregar som.\n");
		return false;
	}

	// Imagem de fundo menu
	*background = al_load_bitmap("../../imagens/menu.png");
	if (!(*background)) {
		printf("Erro ao carregar a imagem de fundo.\n");
		return false;
	}
	return true;
}

/*-------------------------------------------------------------------------------------------------------------------------*/

// Aloca o mapa e o pega do arquivo em mapas/
void get_map (int map_id, Map *map) {
	map->id = map_id;
	map->pellet_n = 0; // Conta as pellets
	FILE* map_file;
	// Pega do arquivo correspondente
	switch (map_id) {
	default:
		map_file = fopen("../../mapas/original.txt", "r");
		break;
	case 1:
		map_file = fopen("../../mapas/dungeon.txt", "r");
	}
	fscanf(map_file, "%d %d", &map->w, &map->h);
	map->m = malloc(map->h * sizeof(int*));
	for (int i = 0; i < map->h; i++)
		map->m[i] = malloc(map->w * sizeof(int));
	char *line = malloc((map->w+1) * sizeof(char)); // Lê a linha inteira do mapa de uma vez só, para evitar bugs com os \n
	for (int i = 0; i < map->h; i++) {
		fscanf(map_file, "%s ", line); // Lê a linha
		for (int j = 0; j < map->w; j++) {
			// Guarda na matriz de acordo com cada caractere da linha lida
			switch (line[j]) {
			default: // Wall
				map->m[i][j] = 0;
				break;
			case '1': // Pellet
				map->pellet_n++;
				map->m[i][j] = 1;
				break;
			case '2': // Empty
				map->m[i][j] = 2;
				break;
			case '3': // Vitamin
				map->m[i][j] = 3;
				break;
			case '4': // Door of the ghosts' beginning
				map->m[i][j] = 4;
				break;
			case '5':
				map->m[i][j] = 8;

			}
		}
	}
	fclose(map_file);
}

/*-------------------------------------------------------------------------------------------------------------------------*/

// Desaloca o mapa
void free_map (Map *map) {
	map->id = -1;
	if (map->m == NULL)
		return;
	for (int i = 0; i < map->h; i++)
		free(map->m[i]);
	free(map->m);
	map->m = NULL;
}
