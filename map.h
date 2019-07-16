#define VISITED 0
#define UNVISITED -1
#define SEEN -2
#define ALTERNATIVE 1

#define NUM_SIDES 4

#define NUM_ROWS 5
#define NUM_COLS 9
#define MAX_DIM NUM_COLS
#define NUM_NODES NUM_COLS*NUM_COLS

#define CORRECT 1
#define WRONG 0
#define UNDETERMINED -1

// (Direction + side) % 4 = new Direction
#define NEW_DIRECTION(direction, side) (Direction)(((int)direction + (int)side) % NUM_SIDES)
#define HASH_COORD(coord) (coord.row >= 0 ? coord.row*NUM_COLS + coord.col : -coord.row*NUM_COLS + coord.col)

typedef int16_t num;

struct __attribute__ ((packed))  Coord {
    num row;
    num col;
};

struct __attribute__ ((packed)) Edge {
    Coord dest;
    bool visited;
};

struct __attribute__ ((packed)) Node {
    num value;
    num row;
    num col;
    Edge edges[NUM_SIDES];
};

// Clockwise ordering
enum class Direction {
    RIGHT = 0,
    DOWN  = 1,
    LEFT  = 2,
    UP    = 3,
    NONE  =-1,
};

// # of clockwise rotations
enum class Side {
    FRONT = 0,
    RIGHT = 1,
    BACK  = 2,
    LEFT  = 3,
    NONE  = -1,
};

void init_map();
bool is_centre_found();
void print_map(int orientation);
void print_everything();

void add_node(Coord pos, Direction direction);
void set_back_edge(Coord pos, Coord val, Direction direction);
void decrement_value(Coord pos);
void increment_value(Coord pos);

Node *get_node(Coord pos);
void mark_edge_visited(Node *node, int edge);
void update_value(Coord pos, int new_value);

int get_path(Coord start, Coord end, Coord *path);
