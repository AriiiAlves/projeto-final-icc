#include"pacman.h"

int main() {
	ALLEGRO_DISPLAY *display;
	ALLEGRO_EVENT_QUEUE *queue;
	ALLEGRO_TIMER *timer;
	ALLEGRO_FONT *font;
	ALLEGRO_SAMPLE *sample;
	ALLEGRO_SAMPLE_INSTANCE *sample_instance;
	int width = 1920, height = 1080; // Valores padrão caso o Allegro falhe em obter os do sistema

	start(&display, &queue, &timer, &width, &height);

	int menu_id = 0;
	Map map = (Map){NULL, 0, 0, 0, width*(0.5-0.25), width*(0.5+0.25), height*(0.54-0.44), height*(0.54+0.44), 0.0, 0.0, 0.125, 0.375};
	bool running = true;      // Indica se a aplicação deve continuar executando

	if (!load_media(&font, &sample))
		return -1;

	// Play music
	sample_instance = al_create_sample_instance(sample);
	al_attach_sample_instance_to_mixer(sample_instance, al_get_default_mixer());
	al_set_sample_instance_playmode(sample_instance, ALLEGRO_PLAYMODE_LOOP);
	al_play_sample_instance(sample_instance);


	ALLEGRO_EVENT ev;         // Estrutura para eventos
	al_start_timer(timer);    // Inicia o temporizador
	while (running) {
		switch (menu_id) {
		case 0:
			menu_id = main_menu(&ev, &queue, &running, font, width, height);
			break;
		case 1:
			if (map.m == NULL)
				get_map(0, &map);
			menu_id = game(&ev, &queue, &running, &map, font, width, height);
			break;
		case 2:
			menu_id = maps_menu(&ev, &queue, &running, font, width, height, &map);
			break;
		default:
			running = false;
			break;
		}
	}

	destroy_all(&sample_instance, &sample, &font, &timer, &queue, &display);
	return 0;
}
