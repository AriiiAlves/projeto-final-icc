#include"pacman.h"

int main() {
	// Declara as variáveis do Allegro
	ALLEGRO_DISPLAY *display;
	ALLEGRO_EVENT_QUEUE *queue;
	ALLEGRO_TIMER *timer;
	ALLEGRO_FONT *font;
	ALLEGRO_FONT *title_font;
	ALLEGRO_SAMPLE *menu_sample;
	ALLEGRO_SAMPLE *game_sample;
	ALLEGRO_SAMPLE_INSTANCE *menu_sample_instance;
	ALLEGRO_SAMPLE_INSTANCE *game_sample_instance;
	ALLEGRO_BITMAP *background;
	int width = 1920, height = 1080; // Valores padrão caso o Allegro falhe em obter os do sistema

	start(&display, &queue, &timer, &width, &height);

	int menu_id = 0;                                                                // Menu atual
	Map map = (Map){NULL, -1, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.125, 0.375, 0}; // Mapa
	NodeMap nodemap = (NodeMap){NULL, 0, 0};                                        // Nodemap declarado
	bool running = true;                                                            // Indica se a aplicação deve continuar executando

	// Carrega as mídias
	if (!load_media(&font, &title_font, &menu_sample, &game_sample, &background))
		return -1;

	// Play music
	menu_sample_instance = al_create_sample_instance(menu_sample);
	al_attach_sample_instance_to_mixer(menu_sample_instance, al_get_default_mixer());
	al_set_sample_instance_playmode(menu_sample_instance, ALLEGRO_PLAYMODE_LOOP);
	al_play_sample_instance(menu_sample_instance);

	game_sample_instance = al_create_sample_instance(game_sample);
	al_attach_sample_instance_to_mixer(game_sample_instance, al_get_default_mixer());
	al_set_sample_instance_playmode(game_sample_instance, ALLEGRO_PLAYMODE_LOOP);

	ALLEGRO_EVENT ev;      // Estrutura para eventos
	al_start_timer(timer); // Inicia o temporizador
	while (running) {
		switch (menu_id) {
		case 0: // Menu principal
			al_stop_sample_instance(game_sample_instance);
			al_play_sample_instance(menu_sample_instance);
			menu_id = main_menu(&ev, &queue, &running, font, title_font, width, height, background);
			break;
		case 1: // Jogo
			al_stop_sample_instance(menu_sample_instance);
			al_play_sample_instance(game_sample_instance);
			// Caso nenhum mapa tenha sido escolhido, inicializa-se um mapa aleatório
			if (map.m == NULL)
				get_map(rand() % MAPS_N, &map);
			// E obtém-se o respectivo mapa de nós
			if (nodemap.m == NULL)
				get_node_map(&map, &nodemap);
			menu_id = game(&ev, &queue, &running, &map, &nodemap, font, width, height, &timer,, &menu_id);
			break;
		case 2: // Menu de escolha de mapas
			menu_id = maps_menu(&ev, &queue, &running, font, width, height, &map);
			break;
		default: // Se em nenhum menu, encerra o programa
			running = false;
			break;
		}
	}

	// Destrói as variáveis do Allegro e encerra o programa
	destroy_all(&menu_sample_instance, &menu_sample, &font, &timer, &queue, &display);
	return 0;
}
