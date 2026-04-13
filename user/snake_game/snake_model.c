#include "snake_model.h"

static uint32_t snake_model_rand_u32(snake_model_t* model) {
    uint32_t x = model->rng_state;
    if (x == 0) {
        x = 0x9E3779B9u;
    }
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    model->rng_state = x;
    return x;
}

static int wrap_coord(int value, int limit) {
    int wrapped = value % limit;
    if (wrapped < 0) {
        wrapped += limit;
    }
    return wrapped;
}

static void snake_model_clear_occupancy(snake_model_t* model) {
    for (int y = 0; y < SNAKE_FIELD_HEIGHT; ++y) {
        for (int x = 0; x < SNAKE_FIELD_WIDTH; ++x) {
            model->occupancy[y][x] = 0;
        }
    }
}

static int snake_model_cells_equal(snake_cell_t a, snake_cell_t b) {
    return a.x == b.x && a.y == b.y;
}

static int snake_model_find_apple_at(const snake_model_t* model, int x, int y) {
    for (uint8_t i = 0; i < model->apple_count; ++i) {
        if ((int)model->apples[i].x == x && (int)model->apples[i].y == y) {
            return (int)i;
        }
    }
    return -1;
}

static int snake_model_enqueue_growth_point(snake_model_t* model, snake_cell_t point) {
    if (model->growth_count >= SNAKE_MAX_LENGTH) {
        return 0;
    }

    uint16_t index = (uint16_t)((model->growth_head + model->growth_count) % SNAKE_MAX_LENGTH);
    model->growth_points[index] = point;
    model->growth_count++;
    return 1;
}

static snake_cell_t snake_model_growth_point_front(const snake_model_t* model) {
    return model->growth_points[model->growth_head];
}

static void snake_model_pop_growth_point(snake_model_t* model) {
    if (model->growth_count == 0) {
        return;
    }
    model->growth_head = (uint16_t)((model->growth_head + 1) % SNAKE_MAX_LENGTH);
    model->growth_count--;
}

void snake_model_init_center(snake_model_t* model, snake_dir_t initial_dir, uint16_t initial_length) {
    if (initial_length == 0) {
        initial_length = 1;
    }
    if (initial_length > SNAKE_MAX_LENGTH) {
        initial_length = SNAKE_MAX_LENGTH;
    }

    const int cx = SNAKE_FIELD_WIDTH / 2;
    const int cy = SNAKE_FIELD_HEIGHT / 2;
    int tail_dx = -1;
    int tail_dy = 0;

    switch (initial_dir) {
        case SNAKE_DIR_UP:
            tail_dx = 0;
            tail_dy = 1;
            break;
        case SNAKE_DIR_RIGHT:
            tail_dx = -1;
            tail_dy = 0;
            break;
        case SNAKE_DIR_DOWN:
            tail_dx = 0;
            tail_dy = -1;
            break;
        case SNAKE_DIR_LEFT:
            tail_dx = 1;
            tail_dy = 0;
            break;
        default:
            initial_dir = SNAKE_DIR_RIGHT;
            tail_dx = -1;
            tail_dy = 0;
            break;
    }

    model->dir = initial_dir;
    model->length = initial_length;
    model->apple_count = 0;
    model->growth_head = 0;
    model->growth_count = 0;

    snake_model_clear_occupancy(model);

    for (uint16_t i = 0; i < model->length; ++i) {
        int x = wrap_coord(cx + tail_dx * (int)i, SNAKE_FIELD_WIDTH);
        int y = wrap_coord(cy + tail_dy * (int)i, SNAKE_FIELD_HEIGHT);

        model->body[i].x = (uint8_t)x;
        model->body[i].y = (uint8_t)y;
        model->occupancy[y][x] = 1;
    }

    model->head = model->body[0];
    model->tail = model->body[model->length - 1];
}

void snake_model_seed_random(snake_model_t* model, uint32_t seed) {
    model->rng_state = seed;
    if (model->rng_state == 0) {
        model->rng_state = 0xA341316Cu;
    }
}

int snake_model_spawn_apple(snake_model_t* model) {
    if (model->apple_count >= SNAKE_MAX_APPLES) {
        return 0;
    }

    const int total_cells = SNAKE_FIELD_WIDTH * SNAKE_FIELD_HEIGHT;
    int free_cells = total_cells - (int)model->length - (int)model->apple_count;

    if (free_cells <= 0) {
        return 0;
    }

    int target = (int)(snake_model_rand_u32(model) % (uint32_t)free_cells);

    for (int y = 0; y < SNAKE_FIELD_HEIGHT; ++y) {
        for (int x = 0; x < SNAKE_FIELD_WIDTH; ++x) {
            if (model->occupancy[y][x] != 0) {
                continue;
            }
            if (snake_model_find_apple_at(model, x, y) >= 0) {
                continue;
            }
            if (target == 0) {
                model->apples[model->apple_count].x = (uint8_t)x;
                model->apples[model->apple_count].y = (uint8_t)y;
                model->apple_count++;
                return 1;
            }
            --target;
        }
    }

    return 0;
}

int snake_model_is_occupied(const snake_model_t* model, int x, int y) {
    if (x < 0 || y < 0 || x >= SNAKE_FIELD_WIDTH || y >= SNAKE_FIELD_HEIGHT) {
        return 0;
    }
    return model->occupancy[y][x] != 0;
}

int snake_model_try_consume_apple(snake_model_t* model) {
    if (model->apple_count == 0) {
        return 0;
    }

    int apple_index = snake_model_find_apple_at(model, (int)model->head.x, (int)model->head.y);
    if (apple_index < 0) {
        return 0;
    }

    snake_cell_t consumed_apple = model->apples[(uint8_t)apple_index];

    for (uint8_t i = (uint8_t)apple_index; i + 1 < model->apple_count; ++i) {
        model->apples[i] = model->apples[i + 1];
    }
    model->apple_count--;

    (void)snake_model_enqueue_growth_point(model, consumed_apple);
    return 1;
}

int snake_model_step(snake_model_t* model, snake_dir_t next_dir) {
    int dx = 0;
    int dy = 0;

    switch (next_dir) {
        case SNAKE_DIR_UP:
            dy = -1;
            break;
        case SNAKE_DIR_RIGHT:
            dx = 1;
            break;
        case SNAKE_DIR_DOWN:
            dy = 1;
            break;
        case SNAKE_DIR_LEFT:
            dx = -1;
            break;
        default:
            return 0;
    }

    int next_x = wrap_coord((int)model->head.x + dx, SNAKE_FIELD_WIDTH);
    int next_y = wrap_coord((int)model->head.y + dy, SNAKE_FIELD_HEIGHT);
    int tail_x = (int)model->tail.x;
    int tail_y = (int)model->tail.y;
    int moving_into_tail = (next_x == tail_x) && (next_y == tail_y);
    int should_grow = 0;

    if (model->growth_count > 0 && model->length > 1 && model->length < SNAKE_MAX_LENGTH) {
        snake_cell_t reached_on_normal_step = model->body[model->length - 2];
        should_grow = snake_model_cells_equal(reached_on_normal_step, snake_model_growth_point_front(model));
    }

    if (snake_model_is_occupied(model, next_x, next_y) && (!moving_into_tail || should_grow)) {
        return 0;
    }

    if (should_grow) {
        for (int i = (int)model->length; i > 0; --i) {
            model->body[i] = model->body[i - 1];
        }
        model->length++;
        snake_model_pop_growth_point(model);
    } else if (model->length > 1) {
        for (int i = (int)model->length - 1; i > 0; --i) {
            model->body[i] = model->body[i - 1];
        }
    }

    if (!should_grow) {
        model->occupancy[tail_y][tail_x] = 0;
    }
    model->body[0].x = (uint8_t)next_x;
    model->body[0].y = (uint8_t)next_y;
    model->occupancy[next_y][next_x] = 1;

    model->head = model->body[0];
    model->tail = model->body[model->length - 1];
    model->dir = next_dir;

    return 1;
}
