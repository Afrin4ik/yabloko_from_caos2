#pragma once

#include "snake_model.h"

void snake_render_menu(uint8_t apple_count);
void snake_render_full(const snake_model_t* model);
void snake_render_pause(const snake_model_t* model);
void snake_render_game_over(const snake_model_t* model);
void snake_render_step(const snake_model_t* model, snake_cell_t prev_head, snake_cell_t prev_tail);
