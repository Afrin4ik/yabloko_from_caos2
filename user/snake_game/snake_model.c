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
    model->has_apple = 0;
    model->apple.x = 0;
    model->apple.y = 0;

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
    const int total_cells = SNAKE_FIELD_WIDTH * SNAKE_FIELD_HEIGHT;
    int free_cells = total_cells - (int)model->length;

    if (free_cells <= 0) {
        model->has_apple = 0;
        return 0;
    }

    int target = (int)(snake_model_rand_u32(model) % (uint32_t)free_cells);

    for (int y = 0; y < SNAKE_FIELD_HEIGHT; ++y) {
        for (int x = 0; x < SNAKE_FIELD_WIDTH; ++x) {
            if (model->occupancy[y][x] != 0) {
                continue;
            }
            if (target == 0) {
                model->apple.x = (uint8_t)x;
                model->apple.y = (uint8_t)y;
                model->has_apple = 1;
                return 1;
            }
            --target;
        }
    }

    model->has_apple = 0;
    return 0;
}

int snake_model_is_occupied(const snake_model_t* model, int x, int y) {
    if (x < 0 || y < 0 || x >= SNAKE_FIELD_WIDTH || y >= SNAKE_FIELD_HEIGHT) {
        return 0;
    }
    return model->occupancy[y][x] != 0;
}

int snake_model_step_no_growth(snake_model_t* model, snake_dir_t next_dir) {
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

    if (snake_model_is_occupied(model, next_x, next_y) && !moving_into_tail) {
        return 0;
    }

    if (model->length > 1) {
        for (int i = (int)model->length - 1; i > 0; --i) {
            model->body[i] = model->body[i - 1];
        }
    }

    model->occupancy[tail_y][tail_x] = 0;
    model->body[0].x = (uint8_t)next_x;
    model->body[0].y = (uint8_t)next_y;
    model->occupancy[next_y][next_x] = 1;

    model->head = model->body[0];
    model->tail = model->body[model->length - 1];
    model->dir = next_dir;

    return 1;
}
