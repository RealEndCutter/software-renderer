#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#define WIDTH 1024
#define HEIGHT 1024

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} pixel;

typedef enum {
    COLOR_BLACK,
    COLOR_WHITE,
    COLOR_GREY,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE,
    COLOR_YELLOW,
    COLOR_PURPLE,
    COLOR_ORANGE,
    COLOR_CYAN,
    COLOR_PINK,
    COLOR_BROWN,
    COLOR_DARK_RED,
    COLOR_DARK_GREEN,
    COLOR_DARK_BLUE,
    COLOR_LIGHT_GREY,
    COLOR_DARK_GREY,
    COLOR_LIME,
    COLOR_TEAL,
    COLOR_NAVY,
    COLOR_MAROON,
    COLOR_OLIVE,
    COLOR_INDIGO,
    COLOR_GOLD,
    COLOR_COUNT
} color;

static const pixel COLORS[] = {
    {0, 0, 0},       // COLOR_BLACK
    {255, 255, 255}, // COLOR_WHITE
    {127, 127, 127}, // COLOR_GREY
    {255, 0, 0},     // COLOR_RED
    {0, 255, 0},     // COLOR_GREEN
    {0, 0, 255},     // COLOR_BLUE
    {255, 255, 0},   // COLOR_YELLOW
    {255, 0, 255},   // COLOR_PURPLE
    {255, 165, 0},   // COLOR_ORANGE
    {0, 255, 255},   // COLOR_CYAN
    {255, 192, 203}, // COLOR_PINK
    {139, 69, 19},   // COLOR_BROWN
    {139, 0, 0},     // COLOR_DARK_RED
    {0, 100, 0},     // COLOR_DARK_GREEN
    {0, 0, 139},     // COLOR_DARK_BLUE
    {211, 211, 211}, // COLOR_LIGHT_GREY
    {64, 64, 64},    // COLOR_DARK_GREY
    {50, 205, 50},   // COLOR_LIME
    {0, 128, 128},   // COLOR_TEAL
    {0, 0, 128},     // COLOR_NAVY
    {128, 0, 0},     // COLOR_MAROON
    {128, 128, 0},   // COLOR_OLIVE
    {75, 0, 130},    // COLOR_INDIGO
    {255, 215, 0},   // COLOR_GOLD
};

typedef struct {
    pixel** data;
    int w;
    int h;
} display;

typedef struct {
    int x;
    int y;
} point2i;

typedef struct {
    int x;
    int y;
    int z;
} point3i;

typedef struct {
    point2i a;
    point2i b;
    point2i c;
} triangle;

int create_display(display* disp, int w, int h) {
    char *mem = malloc(sizeof(pixel*) * h + sizeof(pixel) * w * h);

    if(!mem) return 0;

    disp->data = (pixel**)mem;
    disp->w = w;
    disp->h = h;

    pixel *pixels = (pixel*)(mem + sizeof(pixel*) * h);
    for(int i = 0; i < h; ++i) {
        disp->data[i] = pixels + i*w;
    }

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            disp->data[i][j] = COLORS[COLOR_BLACK];
        }
    }
    
    return 1;
}

int from_obj(const char* filename, point3i* vertex_buff, size_t* v_size, uint32_t* index_buff, size_t* i_size) {
    FILE* file = fopen(filename, "r");

    if(!file) {
        printf("Cannot open file: %s:", filename);
        return 0;
    }

    // vertices and indices count
    *v_size = 0;
    *i_size = 0;

    char line[512];
    while(fgets(line, sizeof(line), file)) {
        if(*line == 'v')
            (*v_size)++;
        else if(*line == 'f')
            (*i_size)++;
    }

    printf("Vertex count: %lu\n", *v_size);
    printf("Index count: %lu\n", *i_size);

    fclose(file);
}

int to_bmp(display* disp, const char* filename) {

    FILE* file = fopen(filename, "wb");

    if(!file) return 0;

    unsigned char header[54] = {
        0x42, 0x4D // BM
    };

    int rowSize = (disp->w * 3 + 3) & ~3;
    int imageSize = rowSize * disp->h;
    int fileSize = 54 + imageSize;

    *(int*)&header[2] = fileSize;
    *(int*)&header[10] = 54;
    *(int*)&header[14] = 40;
    *(int*)&header[18] = disp->w;
    *(int*)&header[22] = disp->h;
    *(short*)&header[26] = 1;
    *(short*)&header[28] = 24;
    *(int*)&header[34] = imageSize;

    fwrite(header, 54, 1, file);

    unsigned char pad[3] = {0};
    for (int y = disp->h - 1; y >= 0; --y) {
        for (int x = 0; x < disp->w; ++x) {
            fputc(disp->data[y][x].b, file);
            fputc(disp->data[y][x].g, file);
            fputc(disp->data[y][x].r, file);
        }

        fwrite(pad, rowSize - disp->w * 3, 1, file);
    }

    fclose(file);
    return 1;
}

int cross(const point2i* v1, const point2i* v2) {
    return  v1->x*v2->y - v1->y*v2->x;
}

point2i sub(const point2i* v1, const point2i* v2) {
    return (point2i){v1->x - v2->x, v1->y - v2->y};
}

int random_range(int a, int b) {
    return a + rand() % (b - a + 1);
}

void create_random_trs(point2i* vertices, uint32_t* indices, size_t count) {
    for(size_t i = 0; i < 3*count; i+=3) {
        point2i p = {rand()%WIDTH, rand()%HEIGHT};


        vertices[i] = (point2i){p.x + random_range(-100, 100), p.y + random_range(-100, 100)};
        vertices[i+1] = (point2i){p.x + random_range(-100, 100), p.y + random_range(-100, 100)};
        vertices[i+2] = (point2i){p.x + random_range(-100, 100), p.y + random_range(-100, 100)};

        indices[i] = i;
        indices[i+1] = i+1;
        indices[i+2] = i+2;
    }
}

bool check_in_triangle(const point2i* a, const point2i* b, const point2i* c,  const point2i* p) {
    point2i v1 = sub(b, a);
    point2i v2 = sub(p, a);
    int e0 = cross(&v1, &v2);

    v1 = sub(c, b);
    v2 = sub(p, b);
    int e1 = cross(&v1, &v2);

    v1 = sub(a, c);
    v2 = sub(p, c);
    int e2 = cross(&v1, &v2);

    return (e0 >= 0 && e1 >= 0 && e2 >= 0) ||
            (e0 <= 0 && e1 <= 0 && e2 <= 0);
}

void draw_triangle(display* disp, const point2i* a, const point2i* b, const point2i* c) {
    int start_x = min(a->x, min(b->x, c->x));
    int start_y = min(a->y, min(b->y, c->y));

    int end_x = max(a->x, max(b->x, c->x));
    int end_y = max(a->y, max(b->y, c->y));

    if (end_x < 0 || start_x >= disp->w || end_y < 0 || start_y >= disp->h)
        return;

    start_x = min(max(start_x, 0), disp->w-1);
    start_y = min(max(start_y, 0), disp->h-1);

    end_x = min(max(end_x, 0), disp->w-1);
    end_y = min(max(end_y, 0), disp->h-1);

    pixel col = COLORS[rand()%24];

    for(int i = start_y; i <= end_y; ++i) {
        for(int j = start_x; j <= end_x; ++j) {
            point2i p = {j, i};
            if(check_in_triangle(a, b, c, &p)) {
                disp->data[i][j] = col;
            }
        }
    }
}

void renderer(display* disp, const point2i* vertices, const uint32_t* indices, size_t count) {
    clock_t start = clock();
    for(size_t i = 0; i < 3*count; i+=3) {
        const point2i* a = vertices + indices[i];
        const point2i* b = vertices + indices[i+1];
        const point2i* c = vertices + indices[i+2];
        draw_triangle(disp, a, b, c);
    }
    double frame_time = (double)(clock() - start) / CLOCKS_PER_SEC * 100;
    printf("frame time: %.2f ms\n", frame_time);

}

int main(int argc, char** argv) {
    srand(time(NULL));

    display disp;
    if(!create_display(&disp, WIDTH, HEIGHT))
        return 1;
    
    printf("Display created W: %d, H: %d\n", disp.w, disp.h);

    point3i* vbo = NULL;
    size_t vbo_size = 0;
    uint32_t* ebo = NULL;
    size_t ebo_size = 0;

    from_obj("triangle.obj", vbo, &vbo_size, ebo, &ebo_size);

    // #define TRI_COUNT 1000
    // printf("Triangles count %d\n", TRI_COUNT);
    // point2i vertices[TRI_COUNT*3];
    // uint32_t indices[TRI_COUNT*3];

    // create_random_trs(vertices, indices, TRI_COUNT);

    // renderer(&disp, vertices, indices, TRI_COUNT);



    printf("render: OK", disp.w, disp.h);

    if(argc == 2) {
        to_bmp(&disp, argv[1]);
    }
    free(disp.data);
    return 0;
}