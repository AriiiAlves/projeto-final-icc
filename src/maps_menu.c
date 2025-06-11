#include"pacman.h"

int maps_menu (ALLEGRO_EVENT *ev, ALLEGRO_EVENT_QUEUE **queue, bool *running, ALLEGRO_FONT *font, int width, int height, Map *map) {
	int next_menu = -1;
	int thumbs_n = 3;
	Image* thumbs = malloc(thumbs_n * sizeof(Image));
	thumbs[0].img = al_load_bitmap("../../imagens/thumbs/original.png");
	thumbs[1].img = al_load_bitmap("../../imagens/thumbs/original.png");
	thumbs[2].img = al_load_bitmap("../../imagens/thumbs/original.png");
	for (int i = 0; i < thumbs_n; i++) {
		thumbs[i].w = al_get_bitmap_width(thumbs[i].img);
		thumbs[i].h = al_get_bitmap_width(thumbs[i].img);
	}
	int b_n = 3;
	Button* b = malloc(b_n * sizeof(Button));
	b[0] = (Button){width*(0.23-0.13), width*(0.23+0.13), height*(0.5-0.1), height*(0.5+0.1), false};
	b[1] = (Button){width*(0.5-0.13), width*(0.5+0.13), height*(0.5-0.1), height*(0.5+0.1), false};
	b[2] = (Button){width*(0.77-0.13), width*(0.77+0.13), height*(0.5-0.1), height*(0.5+0.1), false};
	int mouse_x, mouse_y;
	bool redraw = false;
	int select = -1;
	while (*running && next_menu < 0) {
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
					next_menu = 0;
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
					next_menu = 0;
			}
			if (ev->keyboard.keycode == ALLEGRO_KEY_ENTER || ev->keyboard.keycode == ALLEGRO_KEY_PAD_ENTER)
				if (select >= 0 && select < thumbs_n) {
					if (map->m != NULL)
						free_map(map);
					get_map(select, map);
					next_menu = 0;
				}
			break;
		case ALLEGRO_EVENT_TIMER:
			redraw = true; // Marca que a tela precisa ser redesenhada
			break;
		}
		if (redraw && al_is_event_queue_empty(*queue)) {
			maps_menu_show(&font, b, &b_n, &select, thumbs, &thumbs_n);
			redraw = false;
		}
	}
	free(thumbs);
	free(b);
	return 0;
}

/*-------------------------------------------------------------------------------------------------------------------------*/

void maps_menu_show (ALLEGRO_FONT **font, const Button *b, const int *b_n, const int *select, Image *thumbs, int *thumbs_n) {
	al_clear_to_color(al_map_rgb(30, 40, 30));
	ALLEGRO_COLOR thumbs_color = al_map_rgba_f(1, 1, 1, 0.75),
		      thumbs_color_hover = al_map_rgba_f(1, 1, 1, 1);

	for (int i = 0; i < *thumbs_n; i++) {
		if (b[i].hover || *select == i)
			al_draw_tinted_scaled_bitmap(thumbs[i].img, thumbs_color_hover, 0, 0, thumbs[i].w, thumbs[i].h, b[i].x_i, b[i].y_i, (b[i].x_f-b[i].x_i), (b[i].y_f-b[i].y_i), 0);
		else
			al_draw_tinted_scaled_bitmap(thumbs[i].img, thumbs_color, 0, 0, thumbs[i].w, thumbs[i].h, b[i].x_i, b[i].y_i, (b[i].x_f-b[i].x_i), (b[i].y_f-b[i].y_i), 0);
	}
	al_flip_display();
}
