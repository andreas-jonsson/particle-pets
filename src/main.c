// Copyright (C) 2024 Andreas T Jonsson
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include <math.h>

#include <SDL.h>

#include <microui.h>
#include "mu_renderer.h"

static const char button_map[256] = {
	[ SDL_BUTTON_LEFT   & 0xFF ] = MU_MOUSE_LEFT,
	[ SDL_BUTTON_RIGHT  & 0xFF ] = MU_MOUSE_RIGHT,
	[ SDL_BUTTON_MIDDLE & 0xFF ] = MU_MOUSE_MIDDLE,
};

static const char key_map[256] = {
	[ SDLK_LSHIFT       & 0xFF ] = MU_KEY_SHIFT,
	[ SDLK_RSHIFT       & 0xFF ] = MU_KEY_SHIFT,
	[ SDLK_LCTRL        & 0xFF ] = MU_KEY_CTRL,
	[ SDLK_RCTRL        & 0xFF ] = MU_KEY_CTRL,
	[ SDLK_LALT         & 0xFF ] = MU_KEY_ALT,
	[ SDLK_RALT         & 0xFF ] = MU_KEY_ALT,
	[ SDLK_RETURN       & 0xFF ] = MU_KEY_RETURN,
	[ SDLK_BACKSPACE    & 0xFF ] = MU_KEY_BACKSPACE,
};

const SDL_FPoint world_size = { 1280.0f, 720.0f };
const float max_distance = world_size.y * 0.25f;
float update_interval_ms = 33.0f;

enum {
	RED,
	GREEN,
	BLUE,
	YELLOW,
	NUM_GROUPS
};

typedef struct {
	const char *name;
	SDL_Color color;
	SDL_FPoint *positions, *velocities;
	float gravity[NUM_GROUPS];
	int count;
	float spawn;
} group;

group groups[NUM_GROUPS] = {
	{"RED", {0xFF, 0, 0, 0xFF}, NULL, NULL, {0.0f}, 0, 200.0f},
	{"GREEN", {0, 0xFF, 0, 0xFF}, NULL, NULL, {0.0f}, 0, 200.0f},
	{"BLUE", {0, 0, 0xFF, 0xFF}, NULL, NULL, {0.0f}, 0, 200.0f},
	{"YELLOW", {0xFF, 0xFF, 0, 0xFF}, NULL, NULL, {0.0f}, 0, 200.0f}
};

static int text_width(mu_Font font, const char *text, int len) {
	(void)font;
	if (len == -1)
		len = (int)strlen(text);
	return mr_get_text_width(text, len);
}

static int text_height(mu_Font font) {
	(void)font;
	return mr_get_text_height();
}

static void spawn_group(group *g) {
	g->count = (int)g->spawn;
	g->positions = SDL_realloc(g->positions, sizeof(SDL_FPoint) * g->count); assert(g->positions);
	g->velocities = SDL_realloc(g->velocities, sizeof(SDL_FPoint) * g->count); assert(g->velocities);

	for (int i = 0; i < g->count; i++) {
		g->positions[i].x = ((double)rand() / RAND_MAX) * world_size.x;
		g->positions[i].y = ((double)rand() / RAND_MAX) * world_size.y;
		g->velocities[i].x = g->velocities[i].y = 0.0f;
	}
}

static void spawn(void) {
	for (int i = 0; i < NUM_GROUPS; i++)
		spawn_group(&groups[i]);
}

static void copy(void) {
	char buffer[1024] = "";
	snprintf(buffer, sizeof(buffer),
		"%f %f %f %f %f "
		"%f %f %f %f %f "
		"%f %f %f %f %f "
		"%f %f %f %f %f",
		groups[0].spawn, groups[0].gravity[0], groups[0].gravity[1], groups[0].gravity[2], groups[0].gravity[3],
		groups[1].spawn, groups[1].gravity[0], groups[1].gravity[1], groups[1].gravity[2], groups[1].gravity[3],
		groups[2].spawn, groups[2].gravity[0], groups[2].gravity[1], groups[2].gravity[2], groups[2].gravity[3],
		groups[3].spawn, groups[3].gravity[0], groups[3].gravity[1], groups[3].gravity[2], groups[3].gravity[3]);

	SDL_SetClipboardText(buffer);
}

static void paste(void) {
	if (!SDL_HasClipboardText())
		return;

	group temp[NUM_GROUPS];
	memcpy(temp, groups, sizeof(group) * NUM_GROUPS);
	char *buffer = SDL_GetClipboardText();

	int ret = sscanf(buffer,
		"%f %f %f %f %f "
		"%f %f %f %f %f "
		"%f %f %f %f %f "
		"%f %f %f %f %f",
		&temp[0].spawn, &temp[0].gravity[0], &temp[0].gravity[1], &temp[0].gravity[2], &temp[0].gravity[3],
		&temp[1].spawn, &temp[1].gravity[0], &temp[1].gravity[1], &temp[1].gravity[2], &temp[1].gravity[3],
		&temp[2].spawn, &temp[2].gravity[0], &temp[2].gravity[1], &temp[2].gravity[2], &temp[2].gravity[3],
		&temp[3].spawn, &temp[3].gravity[0], &temp[3].gravity[1], &temp[3].gravity[2], &temp[3].gravity[3]);

	if (ret == 20) {
		memcpy(groups, temp, sizeof(group) * NUM_GROUPS);
		spawn();
	}
	SDL_free(buffer);
}

static void update_windows(mu_Context *ctx) {
	if (mu_begin_window_ex(ctx, "Settings", mu_rect(40, 40, 400, 475), 0)) {

		mu_layout_row(ctx, 3, (int[]){0, 0, -1}, 25);
		if (mu_button(ctx, "Copy"))	copy();
		if (mu_button(ctx, "Paste")) paste();
		if (mu_button(ctx, "Reset")) spawn();
		
		mu_layout_row(ctx, 2, (int[]){100, -1}, 25);
		mu_label(ctx, "Update Interval");
		mu_slider(ctx, &update_interval_ms, 16.0f, 480.0f);

		for (int i = 0; i < NUM_GROUPS; i++) {
			group *g = &groups[i];
			mu_push_id(ctx, g, sizeof(group));

			if (mu_header_ex(ctx, g->name, (i < 2) ? MU_OPT_EXPANDED : 0)) {
				mu_layout_row(ctx, 2, (int[]){-100, -1}, 25);

				mu_slider(ctx, &g->spawn, 1.0f, 500.0f);

				mu_push_id(ctx, &i, sizeof(int));
				if (mu_button(ctx, "Respawn"))
					spawn_group(g);
				mu_pop_id(ctx);
			
				for (int j = 0; j < NUM_GROUPS; j++) {
					group *rg = &groups[j];
					mu_layout_row(ctx, 2, (int[]){0, -1}, 25);

					int id = (i << 8) | j;
					mu_push_id(ctx, &id, sizeof(int));
					mu_label(ctx, rg->name);
					mu_pop_id(ctx);

					mu_slider(ctx, &g->gravity[j], -1.0f, 1.0f);
				}
			}
			mu_pop_id(ctx);
		}
		mu_end_window(ctx);
	}
}

static void update_particle_rule(group *a, group *b, float gravity) {
	for (int i = 0; i < a->count; i++) {
		SDL_FPoint fa = {0};
		SDL_FPoint *pa = &a->positions[i];

		for (int j = 0; j < b->count; j++) {
			SDL_FPoint pb = b->positions[j];
			SDL_FPoint v = { pa->x - pb.x, pa->y - pb.y };
			
			float d = sqrtf(v.x * v.x + v.y * v.y);
			if (d > 0.0f && d < max_distance) {
				float f = gravity * 1.0f / d;
				fa.x += f * v.x;
				fa.y += f * v.y;
			}
		}

		SDL_FPoint *va = &a->velocities[i];
		va->x = (va->x + fa.x) * 0.5f;
		va->y = (va->y + fa.y) * 0.5f;

		pa->x += va->x;
		pa->y += va->y;

		if (pa->x < 0.0f) {
			pa->x = 0.0f;
			va->x *= -1.0f;
		} else if (pa->x >= world_size.x) {
			pa->x = world_size.x - 1.0f;
			va->x *= -1.0f;
		}

		if (pa->y < 0.0f) {
			pa->y = 0.0f;
			va->y *= -1.0f;
		} else if (pa->y >= world_size.y) {
			pa->y = world_size.y - 1.0f;
			va->y *= -1.0f;
		}
	}
}

int main(int argc, char *argv[]) {
	(void)argc; (void)argv;
	srand(time(NULL));

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		SDL_Log("SDL_Init() failed with error: %s\n", SDL_GetError());
		return -1;
	}

	SDL_Window *window = SDL_CreateWindow(
		"Particle Pets", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		(int)world_size.x, (int)world_size.y, SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE);

	if (!window) {
		SDL_Log("SDL_CreateWindow() failed with error: %s\n", SDL_GetError());
		return -1;
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
	if (!renderer) {
		SDL_Log("SDL_CreateRenderer() failed with error: %s\n", SDL_GetError());
		return -1;
	}

	if (SDL_RenderSetLogicalSize(renderer, (int)world_size.x, (int)world_size.y)) {
		SDL_Log("SDL_RenderSetLogicalSize() failed with error: %s\n", SDL_GetError());
		return -1;
	}

	mr_renderer *mr = mr_init(renderer);
	mu_Context *ctx = SDL_malloc(sizeof(mu_Context));
	mu_init(ctx);
	ctx->text_width = text_width;
	ctx->text_height = text_height;

	spawn();

	// Initial setup
	#define RULE(a, b, g) groups[a].gravity[b] = g
		RULE(RED, RED, -0.32f);
		RULE(RED, GREEN, -0.17f);
		RULE(RED, YELLOW, 0.34f);
		RULE(GREEN, GREEN, -0.1f);
		RULE(GREEN, RED, -0.34f);
		RULE(YELLOW, YELLOW, 0.15f);
		RULE(YELLOW, RED, -0.2f);
		RULE(BLUE, YELLOW, 0.05f);
	#undef RULE

	bool running = true;
	int64_t ticks = 0;
	Uint64 timer = SDL_GetTicks64();

	while (running) {
		Uint64 now = SDL_GetTicks64();
		ticks += (int64_t)(now - timer);
		timer = now;

		// Process inputs
		for (SDL_Event e; SDL_PollEvent(&e);) {
			switch (e.type) {
				case SDL_QUIT: running = false; break;
				case SDL_MOUSEMOTION: mu_input_mousemove(ctx, e.motion.x, e.motion.y); break;
				case SDL_MOUSEWHEEL: mu_input_scroll(ctx, 0, e.wheel.y * -30); break;
				case SDL_TEXTINPUT: mu_input_text(ctx, e.text.text); break;
				case SDL_MOUSEBUTTONDOWN: case SDL_MOUSEBUTTONUP: {
					int b = button_map[e.button.button & 0xFF];
					if (b && e.type == SDL_MOUSEBUTTONDOWN) { mu_input_mousedown(ctx, e.button.x, e.button.y, b); }
					if (b && e.type == SDL_MOUSEBUTTONUP) { mu_input_mouseup(ctx, e.button.x, e.button.y, b); }
					break;
				}
				case SDL_KEYDOWN: case SDL_KEYUP: {
					mu_Container *cont = mu_get_container(ctx, "Settings");
					if (cont && !cont->open) {
						cont->open = 1;
						mu_bring_to_front(ctx, cont);
					}
					int c = key_map[e.key.keysym.sym & 0xFF];
					if (c && e.type == SDL_KEYDOWN) { mu_input_keydown(ctx, c); }
					if (c && e.type == SDL_KEYUP) { mu_input_keyup(ctx, c); }
					break;
				}
			}
		}

		// Update simulation
		while (ticks > 0) {
			for (int i = 0; i < NUM_GROUPS; i++) {
				group *g = &groups[i];
				for (int j = 0; j < NUM_GROUPS; j++)
					update_particle_rule(g, &groups[j], g->gravity[j]);
			}

			// Drop frames after 100 ms?
			if ((ticks -= (int64_t)update_interval_ms) > 100)
				ticks = 100;
		}

		// Render particles and UI
		{
			mr_clear(mr, mu_color(0, 0, 0, 0xFF));

			for (int i = 0; i < NUM_GROUPS; i++) {
				group *g = &groups[i];
				SDL_SetRenderDrawColor(renderer, g->color.r, g->color.g, g->color.b, g->color.a);
				SDL_RenderDrawPointsF(renderer, g->positions, g->count);
			}
		
			mu_begin(ctx);
			update_windows(ctx);
			mu_end(ctx);

			mu_Command *cmd = NULL;
			while (mu_next_command(ctx, &cmd)) {
				switch (cmd->type) {
					case MU_COMMAND_TEXT: mr_draw_text(mr, cmd->text.str, cmd->text.pos, cmd->text.color); break;
					case MU_COMMAND_RECT: mr_draw_rect(mr, cmd->rect.rect, cmd->rect.color); break;
					case MU_COMMAND_ICON: mr_draw_icon(mr, cmd->icon.id, cmd->icon.rect, cmd->icon.color); break;
					case MU_COMMAND_CLIP: mr_set_clip_rect(mr, cmd->clip.rect); break;
				}
			}

			mr_present(mr);
		}
	}

	mr_destroy(mr);
	SDL_free(ctx);

	for (int i = 0; i < NUM_GROUPS; i++) {
		group *g = &groups[i];
		SDL_free(g->positions);
		SDL_free(g->velocities);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}