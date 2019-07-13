#include <iostream>
#include <queue>
#include "map.h"

using namespace std;

/* 5 x 9 
 *  _ _ _ _ _ _ _ _ _ 
 * |S|    _ _ _ _  | |
 * | |_| |_ _ _ _ _| |
 * |  _ _   #  |     |
 * |_|  _ _ _  |     |
 * |_ _ _ _ _|_|_ _ _|
 */

void rotate_until_left_wall_and_no_front_wall();
void rotate_to_side(Side next_side);
void drive_forward();
bool wall_exists(int side);
void print_node(Node node);
std::ostream& operator << (std::ostream& o, const Node& a);
std::ostream& operator << (std::ostream& o, const Direction& dir);

num get_next_row(num row, Direction curr_dir, int side);
num get_next_col(num row, Direction curr_dir, int side);
Side get_priority_side(int priority);

Node Map[NUM_COLS][NUM_COLS];
int orientation = UNDETERMINED;

int move_counter = 0;


int main() {
    init_map();
    rotate_until_left_wall_and_no_front_wall();
    Coord pos = {0, 0};
    Direction direction = Direction::RIGHT;
    add_node(pos, direction);
    // print_node(*get_node(pos));
    Node *curr_node;

    while(!is_centre_found()) { // Exploration
        curr_node = get_node(pos);
        Node next_node;
        int i;
        Side next_side = Side::NONE;
        for (i = 0; i < NUM_SIDES; i++) { 
            // Explores sides near current position.
            // Gets FRONT, RIGHT, LEFT then BACK. 
            // Chooses the first node that's "SEEN", otherwise will go to a node that is visited but has potential paths
            // Otherwise chooses a node where the edge hasn't been traversed (one way).
            // Will never go to a node with value "VISITED" since no potential paths exist.
            Side potential_side;
            potential_side = get_priority_side(i);
            int next_direction = (int)NEW_DIRECTION(direction, potential_side);
            if (curr_node->edges[next_direction].dest.col != -1 && !curr_node->edges[next_direction].visited) {
                next_node = *get_node(curr_node->edges[next_direction].dest);
                if (next_node.value == SEEN) {
                    next_side = potential_side;
                    break;
                } else if (next_node.value != VISITED) {
                    if (next_side == Side::NONE) {
                        next_side = potential_side;
                    }
                }
            }
        }
        if (next_side == Side::NONE) {
            break; // No places to go found
        }

        rotate_to_side(next_side); // TODO: Change this!
        direction = NEW_DIRECTION(direction, next_side);
        
        drive_forward();
        cout << "Now at: " << next_node << endl;
        decrement_value(pos); // We are going to explore one of its points.
        mark_edge_visited(curr_node, (int)direction);

        Coord last_coord = pos;
        pos.row = next_node.row; 
        pos.col = next_node.col;

        if (next_node.value == UNVISITED || next_node.value == SEEN) {
            add_node(pos, direction);
            if (get_node(last_coord)->value != VISITED) {
                increment_value(pos);
                set_back_edge(pos, last_coord, direction);
            }
        }
        // print_node(*get_node(pos));
        
        if (orientation == UNDETERMINED && 
            ((pos.row >= NUM_ROWS || pos.row <= -NUM_ROWS) || 
                (pos.col >= NUM_ROWS || pos.col <= -NUM_ROWS))) {
            if (pos.row >= NUM_ROWS || pos.row <= -NUM_ROWS) {
                orientation = WRONG;
            } else {
                orientation = CORRECT;
            }
        }
        // cout << "########################\n" 
        //     << "END OF ITERATION " << move_counter << "\nOrientation: " << orientation << endl
        //     << "Direction: " << direction << " at (" << pos.row << ", " << pos.col << ")\n"
        //     << "########################\n"; 
        move_counter++;
    }
    
    // The centre is found now we need to get there.
    print_everything();
    print_map(orientation);
    
    Coord centre; 
    if (orientation) {
        centre = {NUM_ROWS/2, NUM_COLS/2};
    } else {
        centre = {NUM_COLS/2, NUM_ROWS/2};
    }

    Coord path[NUM_ROWS*NUM_COLS];
    if (pos.row == centre.row && pos.col == centre.col) {
        cout << "DONE!!!" << endl;
        return EXIT_SUCCESS;
    }

    int steps = get_path(pos, centre, path);
    for (int i = steps -2; i >= 0; i--) {
        Direction next_dir;
        curr_node = get_node(pos);
        for(int k = 0; k < NUM_SIDES; k++) {
            if (curr_node->edges[k].dest.row == path[i].row && curr_node->edges[k].dest.col == path[i].col) {
                next_dir = (Direction)k;
                break;
            }
        }
        int next_side = (NUM_SIDES + (int)next_dir - (int)direction) % NUM_SIDES;
        rotate_to_side((Side)next_side);
        direction = next_dir;
        drive_forward();
        pos = path[i];
    }
    cout << "GOT TO: " << *get_node(pos) << " DONE!" << endl;
    return EXIT_SUCCESS;
}

void init_map() {
    for (int i = 0; i < MAX_DIM; i++) {
        for(int j = 0; j < MAX_DIM; j++) {
            Map[i][j].value = UNVISITED;
            Map[i][j].row = i;
            Map[i][j].col = j;
            for(int k = 0; k < NUM_SIDES; k++) {
                Map[i][j].edges[k].dest.row = -1;
                Map[i][j].edges[k].dest.col = -1;
                Map[i][j].edges[k].visited = false;
            }
        }
    }
    cout << "MAP init complete" << endl;
}

void add_node(Coord pos, Direction direction) {
    int num_paths = 0;
    Node *curr_node = get_node(pos);
    for (int i = 0; i < NUM_SIDES; i++) { // Should not read Side::BACK
        Coord dest = {get_next_row(pos.row, direction, i), get_next_col(pos.col, direction, i)};
        Node *dest_node = get_node(dest);
        if (!wall_exists(i)) {
            if ((dest_node->value != VISITED)) {
                dest_node->row = dest.row;
                dest_node->col = dest.col;
                if (i == (int)Side::BACK) {
                    continue; // For back, wall never exists except for start.
                }
                num_paths++;
                int edge_dir = (int)NEW_DIRECTION(direction, i);
                curr_node->edges[edge_dir].dest.row = dest.row;
                curr_node->edges[edge_dir].dest.col = dest.col;
                dest_node->edges[(edge_dir + NUM_SIDES/2) % NUM_SIDES].dest.row = pos.row;
                dest_node->edges[(edge_dir + NUM_SIDES/2) % NUM_SIDES].dest.col = pos.col;
                if (dest_node->value == UNVISITED) {
                    decrement_value(dest); // It is SEEN now
                }
            }
        }    
    }
    curr_node->row = pos.row;
    curr_node->col = pos.col;
    curr_node->value = num_paths;
}

void set_back_edge(Coord pos, Coord val, Direction direction) {
    int edge_dir = (int)NEW_DIRECTION(direction, Side::BACK);
    Node *curr_node = get_node(pos);
    curr_node->edges[edge_dir].dest.row = val.row;
    curr_node->edges[edge_dir].dest.col = val.col; 
}

void decrement_value(Coord pos) {
    get_node(pos)->value--;
}

void increment_value(Coord pos) {
    get_node(pos)->value++;
}

Node *get_node(Coord pos) {
    if (pos.row >= 0) {
        return &Map[pos.row][pos.col];
    } else {
        return &Map[-pos.row][pos.col];
    }
}

void update_value(Coord pos, int new_value) {
    get_node(pos)->value = new_value;
}

void mark_edge_visited(Node *node, int edge) {
    node->edges[edge].visited = true;
}

bool is_centre_found() {
    return (orientation == CORRECT && Map[NUM_ROWS/2][NUM_COLS/2].value != UNVISITED) ||
        (orientation == WRONG && Map[NUM_COLS/2][NUM_ROWS/2].value != UNVISITED);
}

num get_next_row(num row, Direction curr_dir, int side) {
    Direction next_direction = NEW_DIRECTION(curr_dir, side);
    switch (next_direction) {
    case Direction::UP:
        return row - 1;
        break;
    case Direction::DOWN:
        return row + 1;
    default:
        return row;
    }
}

num get_next_col(num col, Direction curr_dir, int side) {
    Direction next_direction = NEW_DIRECTION(curr_dir, side);
    switch (next_direction) {
    case Direction::LEFT:
        return col - 1;
        break;
    case Direction::RIGHT:
        return col + 1;
    default:
        return col;
    }
}

Side get_priority_side(int priority) {
    switch (priority) {
        case 0:
            return Side::FRONT;
        case 1:
            return Side::RIGHT;
        case 2:
            return Side::LEFT;
        case 3:
            return Side::BACK;
        default:
            return Side::BACK;
    }
}

int get_path(Coord start, Coord end, Coord *path) {
    queue<Coord> q;
    Node *dest = get_node(end);
    q.push(start);
    Coord visited[NUM_NODES];
    for (int i = 0; i < NUM_NODES; i++) {
        visited[i].col = -1;
    }
    visited[HASH_COORD(start)].col = -2;

    while (!q.empty() && visited[HASH_COORD(end)].col == -1) {
        Node *curr_node = get_node(q.front()); q.pop();
        for (int i = 0; i < NUM_SIDES; i++) {
            if (curr_node->edges[i].dest.col != -1 && visited[HASH_COORD(curr_node->edges[i].dest)].col == -1) {
                visited[HASH_COORD(curr_node->edges[i].dest)].row = curr_node->row;
                visited[HASH_COORD(curr_node->edges[i].dest)].col = curr_node->col;
                q.push(curr_node->edges[i].dest);
                if (dest->row == curr_node->edges[i].dest.row && dest->col == curr_node->edges[i].dest.col) {
                    cout << "FOUND END " << endl; 
                    break;
                }
            }
        }
    }

    if (visited[HASH_COORD(end)].col == -1) {
        return -1;
    }

    int steps = 0;
    path[0].row = dest->row;
    path[0].col = dest->col;

    for (steps = 1; path[steps-1].col != -2; steps++) {
        path[steps].row = visited[HASH_COORD(path[steps - 1])].row;
        path[steps].col = visited[HASH_COORD(path[steps - 1])].col;
    }

    return steps -1;
}

void print_map(int orientation) {
    int row_lim = orientation ? NUM_ROWS : NUM_COLS;
    int col_lim = orientation ? NUM_COLS : NUM_ROWS;

    bool is_negative = false;
    for (int j = 0; j < col_lim; j++) {
        if (Map[0][j].edges[(int)Direction::UP].dest.col != -1) {
            is_negative = true;
            break;
        }
        if (Map[0][j].edges[(int)Direction::DOWN].dest.col != -1) {
            is_negative = false;
            break;
        }
    }

    cout << "IS negative: " << is_negative << endl;
    /* if (is_negative) {
        for(int i = 0; i < row_lim*2; i++) {
            if (i == 0) {
                for (int k = 0; k < col_lim; k++) {
                    cout << " ---";
                }
                cout << endl;
            }
            for (int j = col_lim -1; j >= 0; j--) {
                if (i%2 == 0) {
                    if (Map[i/2][j].edges[(int)Direction::RIGHT].dest.col == -1) {
                        if (j == col_lim -1 || Map[i/2][j].value >= 0 || Map[i/2][j+1].value >= 0) {
                            cout << "|";
                        } else {
                            // cout << get_node(Map[i/3][j-1].edges[(int)Direction::LEFT].dest)->value << " ";
                            cout << "*";
                        }
                    } else {
                        cout << " ";
                    }
                    
                    if (i/2 == 0 && j == 0) {
                        cout << " W ";
                    } else if (i/2 == row_lim/2 && j == col_lim/2) {
                        cout << " X ";
                    } else {
                        cout << "   ";
                    }
                    if (j == 0) {
                        cout << "|";
                    }
                } else {
                    if (Map[i/2][j].edges[(int)Direction::UP].dest.col == -1) {
                        if (i/2 == row_lim -1 || Map[i/2][j].value >= 0 || Map[i/2 + 1][j].value >= 0) {
                            cout << " ---";
                        } else {
                            cout << " ***";
                        }
                    } else {
                        cout << "    ";
                    }
                }
            }
            cout << endl;
        }
    } else {
        for(int i = 0; i < row_lim*2; i++) {
            if (i == 0) {
                for (int k = 0; k < col_lim; k++) {
                    cout << " ---";
                }
                cout << endl;
            }
            for (int j = 0; j < col_lim; j++) {
                if (i%2 == 0) {
                    if (Map[i/2][j].edges[(int)Direction::LEFT].dest.col == -1) {
                        if (j == 0 || Map[i/2][j].value >= 0 || Map[i/2][j-1].value >= 0) {
                            cout << "|";
                        } else {
                            // cout << get_node(Map[i/3][j-1].edges[(int)Direction::LEFT].dest)->value << " ";
                            cout << "*";
                        }
                    } else {
                        cout << " ";
                    }
                    
                    if (i/2 == 0 && j == 0) {
                        cout << " E ";
                    } else if (i/2 == row_lim/2 && j == col_lim/2) {
                        cout << " X ";
                    } else {
                        cout << "   ";
                    }
                    if (j == col_lim - 1) {
                        cout << "|";
                    }
                } else {
                    if (Map[i/2][j].edges[(int)Direction::DOWN].dest.col == -1) {
                        if (i/2 == row_lim -1 || Map[i/2][j].value >= 0 || Map[i/2 + 1][j].value >= 0) {
                            cout << " ---";
                        } else {
                            cout << " ***";
                        }
                    } else {
                        cout << "    ";
                    }
                }
            }
            cout << endl;
        }
    }*/

    int col_start = is_negative ? col_lim -1 : 0;
    col_lim = is_negative ? 0 : col_lim -1;
    int col_inc = is_negative ? -1 : 1;
    for (int i = 0; i < row_lim*2; i++) {
        if (i == 0) {
            for (int k = 0; k <= col_lim + col_start; k++) {
                cout << " ---";
            }
            cout << endl;
        }
        for (int j = col_start; 0 <= j && j <= col_lim + col_start; j += col_inc) {
            if (i%2 == 0) {
                
                if ((!is_negative && Map[i/2][j].edges[(int)Direction::LEFT].dest.col == -1) ||
                    (is_negative && Map[i/2][j].edges[(int)Direction::RIGHT].dest.col == -1)) {
                    if (j == col_start || Map[i/2][j].value >= 0 || Map[i/2][j-col_inc].value >= 0) {
                        cout << "|";
                    } else {
                        cout << "*";
                    }
                } else {
                    cout << " ";
                }
                
                if (i/2 == 0 && j == 0) {
                    cout << (is_negative ? " W " : " E ");
                } else if (i/2 == row_lim/2 && j == (col_lim + col_start)/2) {
                    cout << " X ";
                } else {
                    cout << "   ";
                }
                
                if (j == col_lim) {
                    cout << "|";
                }
            } else {
                if ((!is_negative && Map[i/2][j].edges[(int)Direction::DOWN].dest.col == -1) ||
                    (is_negative && Map[i/2][j].edges[(int)Direction::UP].dest.col == -1)) {
                    if (i/2 == row_lim -1 || Map[i/2][j].value >= 0 || Map[i/2 + 1][j].value >= 0) {
                        cout << " ---";
                    } else {
                        cout << " ***";
                    }
                } else {
                    cout << "    ";
                }
            }
        }
        cout << endl;
    }
}

void print_node(Node node) {
    cout << "PRINTING NODE (" << node.row << ", " << node.col << ")" << endl;
    cout << "VALUE: " << node.value << endl;
    cout << "EDGES: " << endl;
    for (int i = 0; i < NUM_SIDES; i++) {
        cout << "EDGE " << i << " with status: " << node.edges[i].visited << " has coord: (" << node.edges[i].dest.row << ", " << node.edges[i].dest.col << ")\n";
    } 
    cout << "END node" << endl;
}

void print_everything() {
    cout << "VISITED" << endl;
    for (int i = 0; i < NUM_COLS; i++) {
        for (int j = 0; j < NUM_COLS; j++) {
            if (Map[i][j].value == VISITED) {
                cout << Map[i][j] << "\t";
            }
        }
    }
    cout << endl << "SEEN" << endl;
    for (int i = 0; i < NUM_COLS; i++) {
        for (int j = 0; j < NUM_COLS; j++) {
            if (Map[i][j].value == SEEN) {
                cout << Map[i][j] << "\t";
            }
        }
    }
    cout << endl << "Other" << endl;
    for (int i = 0; i < NUM_COLS; i++) {
        for (int j = 0; j < NUM_COLS; j++) {
            if (Map[i][j].value > VISITED) {
                cout << Map[i][j] << " With val: " << Map[i][j].value <<  "\t";
            }
        }
    }
    cout << endl;
}

void rotate_until_left_wall_and_no_front_wall() {
    return;
}

void rotate_to_side(Side next_side) {
    cout << "ROTATING to side: " << (int)next_side << " by " << 90*(int)next_side<< endl;
    return;
}

void drive_forward() {
    cout << "DRIVING forward" << endl;
    return;
}

bool wall_exists(int side) {
    // cout << "CHECKING WALL WITH: " << move_counter << " " << side << endl;
    switch (move_counter) {
    case 0: // 0,0, 0,1
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        switch (side) {
            case (int)Side::FRONT:
                return false;
            case (int)Side::RIGHT:
                return true;
            case (int)Side::LEFT:
                return true;
            case (int)Side::BACK:
                return false;
        }
    case 6: // 0,7
        switch (side) {
            case (int)Side::FRONT:
                return true;
            case (int)Side::RIGHT:
                return false;
            case (int)Side::LEFT:
                return true;
            case (int)Side::BACK:
                return false;
        }
    case 7: // 1,7
    case 8: 
    case 9: // 4, 7
        switch (side) {
            case (int)Side::FRONT:
                return false;
            case (int)Side::RIGHT:
                return true;
            case (int)Side::LEFT:
                return true;
            case (int)Side::BACK:
                return false;
        }
    case 10: // 4,6
        switch (side) {
            case (int)Side::FRONT:
                return true;
            case (int)Side::RIGHT:
                return false;
            case (int)Side::LEFT:
                return true;
            case (int)Side::BACK:
                return false;
        }
    case 11: // 4,5
    case 12:
        switch (side) {
            case (int)Side::FRONT:
                return false;
            case (int)Side::RIGHT:
                return true;
            case (int)Side::LEFT:
                return true;
            case (int)Side::BACK:
                return false;
        }
    case 13: // 3,3
        switch (side) {
            case (int)Side::FRONT:
                return true;
            case (int)Side::RIGHT:
                return false;
            case (int)Side::LEFT:
                return true;
            case (int)Side::BACK:
                return false;
        }
    case 14: // 2, 3
        switch (side) {
            case (int)Side::FRONT:
                return false;
            case (int)Side::RIGHT:
                return false;
            case (int)Side::LEFT:
                return true;
            case (int)Side::BACK:
                return false;
        }
    }
    return false;
}


std::ostream& operator << (std::ostream& o, const Node& a) {
    o << "(" << a.row << ", " << a.col << ")";
    return o;
}

std::ostream& operator << (std::ostream& o, const Direction& dir) {
    switch (dir) {
        case Direction::RIGHT:
            o << "RIGHT"; 
            break;
        case Direction::LEFT:
            o << "LEFT"; 
            break;
        case Direction::UP:
            o << "UP"; 
            break;
        case Direction::DOWN:
            o << "DOWN"; 
            break;
        case Direction::NONE:
            o << "NONE"; 
            break;
    }
    return o;
}
