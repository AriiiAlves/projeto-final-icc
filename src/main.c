#include"pacman.h"

int main() {
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
	double sprite_timer = 0.0; // Timer do sprite
	double sprite_delay = 0.05; // Delay entre um sprite e outro

	start(&display, &queue, &timer, &width, &height);

	int menu_id = 0;
	Map map = (Map){NULL, 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.125, 0.375, 0};
	bool running = true;      // Indica se a aplicação deve continuar executando

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

	ALLEGRO_EVENT ev;         // Estrutura para eventos
	al_start_timer(timer);    // Inicia o temporizador
	while (running) {
		switch (menu_id) {
		case 0:
			al_stop_sample_instance(game_sample_instance);
			al_play_sample_instance(menu_sample_instance);
			menu_id = main_menu(&ev, &queue, &running, font, title_font, width, height, background);
			break;
		case 1:
			al_stop_sample_instance(menu_sample_instance);
			al_play_sample_instance(game_sample_instance);
			if (map.m == NULL)
				get_map(0, &map);
			menu_id = game(&ev, &queue, &running, &map, font, width, height, &timer, &sprite_timer, &sprite_delay, &menu_id);
			break;
		case 2:
			menu_id = maps_menu(&ev, &queue, &running, font, width, height, &map);
			break;
		default:
			running = false;
			break;
		}
	}

	destroy_all(&menu_sample_instance, &menu_sample, &font, &timer, &queue, &display);
	return 0;
}
