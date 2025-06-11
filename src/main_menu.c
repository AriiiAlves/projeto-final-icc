#include"pacman.h"

int main_menu (ALLEGRO_EVENT *ev, ALLEGRO_EVENT_QUEUE **queue, bool *running, ALLEGRO_FONT *font, ALLEGRO_FONT *title_font, int width, int height, ALLEGRO_BITMAP **background) {
	int next_menu = -1;
	int b_n = 3;
	Button* b;
	b = malloc(b_n * sizeof(Button));
	b[0] = (Button){width*(0.5-0.2), width*(0.5+0.2), height*(0.35-0.05), height*(0.35+0.05), false};
	b[1] = (Button){width*(0.5-0.2), width*(0.5+0.2), height*(0.46-0.05), height*(0.46+0.05), false};
	b[2] = (Button){width*(0.5-0.2), width*(0.5+0.2), height*(0.57-0.05), height*(0.57+0.05), false};
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
			// Ao clicar sobre o botão, vai para o menu correspondente
			for (int i = 0; i < b_n; i++)
				if (b[i].hover)
					next_menu = i+1;
			break;
		case ALLEGRO_EVENT_KEY_DOWN:
			if (ev->keyboard.keycode == ALLEGRO_KEY_LEFT || ev->keyboard.keycode == ALLEGRO_KEY_UP) {
				if (select < 0)
					select = 0;
				else if (--select < 0)
					select += b_n;
			}
			if (ev->keyboard.keycode == ALLEGRO_KEY_RIGHT || ev->keyboard.keycode == ALLEGRO_KEY_DOWN) {
				if (select < 0)
					select = 0;
			else if (++select == b_n)
					select -= b_n;
			}
			if (select >= 0 && ev->keyboard.keycode == ALLEGRO_KEY_ESCAPE)
				select = -1;
			if (ev->keyboard.keycode == ALLEGRO_KEY_ENTER || ev->keyboard.keycode == ALLEGRO_KEY_PAD_ENTER)
				if (select >= 0)
					next_menu = select+1;
			break;
		case ALLEGRO_EVENT_TIMER:
			redraw = true; // Marca que a tela precisa ser redesenhada
			break;
		}
		if (redraw && al_is_event_queue_empty(*queue)) {
			main_menu_show(&font, &title_font, b, &b_n, &select, background, width, height);
			redraw = false;
		}
	}
	free(b);
	return next_menu;
}

/*-------------------------------------------------------------------------------------------------------------------------*/

void main_menu_show (ALLEGRO_FONT **font, ALLEGRO_FONT **title_font, const Button *b, const int *b_n, const int *select, ALLEGRO_BITMAP **background, int width, int height) {
	al_clear_to_color(al_map_rgb(30, 40, 30));

	// Imagem de fundo
	al_draw_scaled_bitmap(background, 
                      0, 0, al_get_bitmap_width(background), al_get_bitmap_height(background),
                      0, 0, width, height, 0);

	ALLEGRO_COLOR b_color = al_map_rgb(255, 255, 102),
		      b_color_hover = al_map_rgb(204, 204, 0);

	if (*select >= 0)
		al_draw_filled_rounded_rectangle(b[*select].x_i, b[*select].y_i, b[*select].x_f, b[*select].y_f, 10, 10, b_color_hover);
	for (int i = 0; i < *b_n; i++)
		if (*select < 0 && b[i].hover)
			al_draw_filled_rounded_rectangle(b[i].x_i, b[i].y_i, b[i].x_f, b[i].y_f, 10, 10, b_color_hover);
		else if (*select != i)
			al_draw_filled_rounded_rectangle(b[i].x_i, b[i].y_i, b[i].x_f, b[i].y_f, 10, 10, b_color);
	al_draw_text(*title_font, al_map_rgb(255, 255, 102), (b[0].x_i+b[0].x_f)/2, (b[0].y_i+b[0].y_f)/2*0.5, ALLEGRO_ALIGN_CENTER, "Pac Man");

	al_draw_text(*font, al_map_rgb(0, 0, 0), (b[0].x_i+b[0].x_f)/2, (b[0].y_i+b[0].y_f)/2, ALLEGRO_ALIGN_CENTER, "Start");
	al_draw_text(*font, al_map_rgb(0, 0, 0), (b[1].x_i+b[1].x_f)/2, (b[1].y_i+b[1].y_f)/2, ALLEGRO_ALIGN_CENTER, "Maps");
	al_draw_text(*font, al_map_rgb(0, 0, 0), (b[2].x_i+b[2].x_f)/2, (b[2].y_i+b[2].y_f)/2, ALLEGRO_ALIGN_CENTER, "Quit");
	al_flip_display();
}
