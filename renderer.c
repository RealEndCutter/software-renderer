#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

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
    float x;
    float y;
    float z;
} point3f;

typedef struct {
    point2i a;
    point2i b;
    point2i c;
} triangle;

int create_display(display* disp, int w, int h) {
    pixel** data = malloc(sizeof(pixel*) * h + sizeof(pixel) * w * h);
    if(!data) return 0;

    disp->data = data;
    disp->w = w;
    disp->h = h;

    pixel* pixels = (pixel*)(data + h);  
    
    for(int i = 0; i < h; ++i) {
        data[i] = pixels + i * w;
    }

    for(int i = 0; i < h; ++i) {
        for(int j = 0; j < w; ++j) {
            data[i][j] = COLORS[COLOR_BLACK];
        }
    }
    
    return 1;
}

int from_obj(const char* filename, point3f** vertex_buff, size_t* v_size, uint32_t** index_buff, size_t* i_size) {
    FILE* file = fopen(filename, "r");

    if(!file) {
        printf("Cannot open file: %s:", filename);
        return 0;
    }

    // vertices and indices count
    *v_size = 0;
    *i_size = 0;

    char line[64];
    while(fgets(line, sizeof(line), file)) {
        if(*line == 'v')
            (*v_size)++;
        else if(*line == 'f')
            (*i_size)++;
    }

    printf("Vertex count: %lu\n", *v_size);
    printf("Index count: %lu\n", *i_size);

    point3f* vertex_buff_ptr = (point3f*)malloc(*v_size * sizeof(point3f));
    uint32_t* index_buff_ptr = (uint32_t*)malloc(*i_size * 3 * sizeof(uint32_t));

    if(!vertex_buff_ptr || !index_buff_ptr) {
        fclose(file);
        return 0;
    }

    rewind(file);
    *vertex_buff = vertex_buff_ptr;
    *index_buff = index_buff_ptr;

    while(fgets(line, sizeof(line), file)) {
        if(*line == '#')
            continue;

        if(*line == 'v') {
            sscanf(line, "v %f %f %f", &vertex_buff_ptr->x, &vertex_buff_ptr->y, &vertex_buff_ptr->z);
            vertex_buff_ptr++;
        }

        else if(*line == 'f') {
            uint32_t a, b, c;
            sscanf(line, "f %u %u %u", &a, &b, &c);
            index_buff_ptr[0] = a - 1;
            index_buff_ptr[1] = b - 1;
            index_buff_ptr[2] = c - 1;
            index_buff_ptr += 3;
        }
    }

    // check
    // for(int i = 0; i < *v_size; ++i) {
    //     puts("Parsed vertices:");
    //     printf("v%d: %f %f %f\n", i, vertex_buff[i].x, vertex_buff[i].y, vertex_buff[i].z);
    // }

    fclose(file);
    return 1;
}

int to_bmp(display* disp, const char* filename) {
    FILE* file = fopen(filename, "wb");
    if(!file) return 0;

    unsigned char header[54] = {
        0x42, 0x4D, 0, 0, 0, 0, 0, 0, 0, 0,
        54, 0, 0, 0, 40, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        1, 0, 24, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0
    };

    int rowSize = (disp->w * 3 + 3) & ~3;
    int imageSize = rowSize * disp->h;
    int fileSize = 54 + imageSize;

    *(int*)&header[2] = fileSize;
    *(int*)&header[18] = disp->w;
    *(int*)&header[22] = disp->h;
    *(int*)&header[34] = imageSize;

    fwrite(header, 54, 1, file);

    int padding = rowSize - disp->w * 3;
    
    if (padding < 0 || padding > 3) {
        printf("ERROR: Invalid padding %d for width %d\n", padding, disp->w);
        fclose(file);
        return 0;
    }

    for (int y = disp->h - 1; y >= 0; --y) {
        for (int x = 0; x < disp->w; ++x) {
            fputc(disp->data[y][x].b, file);
            fputc(disp->data[y][x].g, file);
            fputc(disp->data[y][x].r, file);
        }
        
        for (int p = 0; p < padding; p++) {
            fputc(0, file);
        }
    }

    fclose(file);
    printf("BMP saved: %s (%dx%d)\n", filename, disp->w, disp->h);
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

int min(int a, int b) { return a < b ? a : b; }
int max(int a, int b) { return a > b ? a : b; }

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

void vertex_operations(point3f* vertex_buff, size_t v_size, point2i** vertex_screen, int width, int height) {
    *vertex_screen = (point2i*)malloc(sizeof(point2i) * v_size);

    if(!*vertex_screen) return;

    // Projection params
    float fov = 60.0f * M_PI / 180.0f;
    float focal_length = 1.0f / tanf(fov / 2.0f);

    float scale = 1.0f;

    for(size_t i = 0; i < v_size; ++i) {
        point3f v = vertex_buff[i];

        // translation
        v.z = -focal_length / scale;

        // screen space
        (*vertex_screen)[i].x = (int)((v.x / v.z) * focal_length * height/2 + width/2);
        (*vertex_screen)[i].y = (int)((v.y / v.z) * focal_length * height/2 + height/2);
    }
}

int main(int argc, char** argv) {
    srand(time(NULL));

    display disp;
    if(!create_display(&disp, WIDTH, HEIGHT))
        return 1;
    
    printf("Display created W: %d, H: %d\n", disp.w, disp.h);

    point3f* vbo = NULL;
    size_t vbo_size = 0;
    uint32_t* ebo = NULL;
    size_t ebo_size = 0;


    if(from_obj("triangle.obj", &vbo, &vbo_size, &ebo, &ebo_size)) {
        point2i* tris_2d = NULL;

        vertex_operations(vbo, vbo_size, &tris_2d, disp.w, disp.h);
        renderer(&disp, tris_2d, ebo, ebo_size);
        free(tris_2d);
    }

    free(vbo);
    free(ebo);

    // #define TRI_COUNT 1000
    // printf("Triangles count %d\n", TRI_COUNT);
    // point2i vertices[TRI_COUNT*3];
    // uint32_t indices[TRI_COUNT*3];

    // create_random_trs(vertices, indices, TRI_COUNT);

    //renderer(&disp, vertices, indices, TRI_COUNT);


    if(argc == 2) {
        to_bmp(&disp, argv[1]);
    }
    free(disp.data);
    return 0;
}