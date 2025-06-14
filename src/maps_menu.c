#include"pacman.h"

int maps_menu (ALLEGRO_EVENT *ev, ALLEGRO_EVENT_QUEUE **queue, bool *running, ALLEGRO_FONT *font, int width, int height, Map *map) {
	int next_menu = -1;
	int map_sel = 0;
	int maps_n = MAPS_N;
	char *names[MAPS_N] = {"Original", "Dungeon"};

	Button* b = malloc((maps_n + 3) * sizeof(Button));
	for (int i = 0; i < maps_n; i++)
		b[i] = (Button){width*(0.08-0.02+0.07*i), width*(0.08+0.02+0.07*i), height*(0.03-0.02), height*(0.03+0.02), false};
	b[maps_n] = (Button){width*(0.5-0.15), width*(0.5+0.15), height*(0.97-0.02), height*(0.97+0.02), false};
	b[maps_n+1] = (Button){width*(0.025-0.015), width*(0.025+0.015), height*(0.5-0.025), height*(0.5+0.025), false};
	b[maps_n+2] = (Button){width*(0.975-0.015), width*(0.975+0.015), height*(0.5-0.025), height*(0.5+0.025), false};
	double incl_1 = (b[maps_n+1].y_f - b[maps_n+1].y_i) / (2.0 * (b[maps_n+1].x_f - b[maps_n+1].x_i));
	double incl_2 = (b[maps_n+2].y_f - b[maps_n+2].y_i) / (2.0 * (b[maps_n+2].x_f - b[maps_n+2].x_i));

	int mouse_x, mouse_y;
	bool redraw = false;
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
			for (int i = 0; i < maps_n+1; i++) {
				b[i].hover = (mouse_x >= b[i].x_i && mouse_x <= b[i].x_f && mouse_y >= b[i].y_i && mouse_y <= b[i].y_f);
			}
			b[maps_n+1].hover = (mouse_x <= b[maps_n+1].x_f && mouse_y >= (b[maps_n+1].y_i - incl_1 * (mouse_x - b[maps_n+1].x_f)) && mouse_y <= (b[maps_n+1].y_f + incl_1 * (mouse_x - b[maps_n+1].x_f)));
			b[maps_n+2].hover = (mouse_x >= b[maps_n+2].x_i && mouse_y <= (b[maps_n+2].y_f - incl_2 * (mouse_x - b[maps_n+2].x_i)) && mouse_y >= (b[maps_n+2].y_i + incl_2 * (mouse_x - b[maps_n+2].x_i)));
			break;
		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
			// Ao clicar sobre o botão, vai seleciona o mapa correspondente e volta para o menu principal
			for (int i = 0; i < maps_n; i++)
				if (b[i].hover) {
					map_sel = i;
				}
			if (b[maps_n].hover)
				next_menu = 1;
			if (b[maps_n+1].hover)
				if (--map_sel < 0)
					map_sel += maps_n;
			if (b[maps_n+2].hover)
				if (++map_sel >= maps_n)
					map_sel -= maps_n;
			break;
		case ALLEGRO_EVENT_KEY_DOWN:
			if (ev->keyboard.keycode == ALLEGRO_KEY_LEFT)
				if (--map_sel < 0)
					map_sel += maps_n;
			if (ev->keyboard.keycode == ALLEGRO_KEY_RIGHT)
				if (++map_sel >= maps_n)
					map_sel -= maps_n;
			if (ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)
				next_menu = 0;
			if (ev->keyboard.keycode == ALLEGRO_KEY_ENTER || ev->keyboard.keycode == ALLEGRO_KEY_PAD_ENTER)
				next_menu = 1;
			break;
		case ALLEGRO_EVENT_TIMER:
			redraw = true; // Marca que a tela precisa ser redesenhada
			break;
		}
		if (map->id != map_sel) {
			free_map(map);
			get_map(map_sel, map);
			if (0.86 * height * map->w / map->h <= 0.9 * width) {
				map->y_i = height*(0.5-0.43);
				map->y_f = height*(0.5+0.43);
				map->y_fac = 1.0*(map->y_f - map->y_i)/map->h;
				map->x_i = (int)(0.5 * width - map->y_fac * map->w / 2);
				map->x_f = (int)(0.5 * width + map->y_fac * map->w / 2);
				map->x_fac = 1.0*(map->x_f - map->x_i)/map->w;
			} else {
				map->x_i = width*(0.5-0.45);
				map->x_f = width*(0.5+0.45);
				map->x_fac = 1.0*(map->x_f - map->x_i)/map->w;
				map->y_i = (int)(0.5 * height - map->x_fac * map->h / 2);
				map->y_f = (int)(0.5 * height + map->x_fac * map->h / 2);
				map->y_fac = 1.0*(map->y_f - map->y_i)/map->h;
			}
		}
		if (redraw && al_is_event_queue_empty(*queue)) {
			maps_menu_show(&font, b, &maps_n, &map_sel, map, names);
			redraw = false;
		}
	}
	free(b);
	return next_menu;
}

/*-------------------------------------------------------------------------------------------------------------------------*/

void maps_menu_show (ALLEGRO_FONT **font, const Button *b, const int *maps_n, const int *map_sel, Map *map, char **names) {
	al_clear_to_color(al_map_rgb(30, 40, 30));

	ALLEGRO_COLOR b_color = al_map_rgb(255, 255, 255),
		      b_color_sel = al_map_rgb(0, 0, 0);

	char number[12];
	for (int i = 0; i < *maps_n; i++) {
		if (i == *map_sel) {
			al_draw_filled_rectangle(b[i].x_i, b[i].y_i, b[i].x_f, b[i].y_f, b_color_sel);
			snprintf(number, sizeof(number), "%d", i+1);
			al_draw_text(*font, b_color, (b[i].x_i+b[i].x_f)/2, (b[i].y_i+b[i].y_f)/2, ALLEGRO_ALIGN_CENTER, number);
		} else {
			al_draw_filled_rectangle(b[i].x_i, b[i].y_i, b[i].x_f, b[i].y_f, b_color);
			snprintf(number, sizeof(number), "%d", i+1);
			al_draw_text(*font, b_color_sel, (b[i].x_i+b[i].x_f)/2, (b[i].y_i+b[i].y_f)/2, ALLEGRO_ALIGN_CENTER, number);
		}
	}
	al_draw_filled_triangle(b[*maps_n+1].x_f, b[*maps_n+1].y_i, b[*maps_n+1].x_f, b[*maps_n+1].y_f, b[*maps_n+1].x_i, (b[*maps_n+1].y_i+b[*maps_n+1].y_f)/2, b_color);
	al_draw_filled_triangle(b[*maps_n+2].x_i, b[*maps_n+2].y_i, b[*maps_n+2].x_i, b[*maps_n+2].y_f, b[*maps_n+2].x_f, (b[*maps_n+2].y_i+b[*maps_n+2].y_f)/2, b_color);

	al_draw_text(*font, al_map_rgb(255, 255, 102), (b[*maps_n].x_i+b[*maps_n].x_f)/2, (b[0].y_i+b[0].y_f)/2, ALLEGRO_ALIGN_CENTER, names[*map_sel]);

	al_draw_filled_rectangle(b[*maps_n].x_i, b[*maps_n].y_i, b[*maps_n].x_f, b[*maps_n].y_f, b_color);
	al_draw_text(*font, al_map_rgb(0, 0, 0), (b[*maps_n].x_i+b[*maps_n].x_f)/2, (b[*maps_n].y_i+b[*maps_n].y_f)/2, ALLEGRO_ALIGN_CENTER, "Select");

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
	al_flip_display();
}
