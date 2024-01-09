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

#ifndef _MU_RENDERER_H_
#define _MU_RENDERER_H_

#include <SDL.h>
#include <microui.h>

typedef struct renderer mr_renderer;

mr_renderer *mr_init(SDL_Renderer *renderer);
void mr_destroy(mr_renderer *r);
void mr_draw_rect(mr_renderer *r, mu_Rect rect, mu_Color color);
void mr_draw_text(mr_renderer *r, const char *text, mu_Vec2 pos, mu_Color color);
void mr_draw_icon(mr_renderer *r, int id, mu_Rect rect, mu_Color color);
void mr_set_clip_rect(mr_renderer *r, mu_Rect rect);
void mr_clear(mr_renderer *r, mu_Color color);
void mr_present(mr_renderer *r);

int mr_get_text_width(const char *text, int len);
int mr_get_text_height(void);

#endif
