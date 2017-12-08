#ifndef BOT_H_
#define BOT_H_

#include <iostream>
#include <algorithm>
#include <map>
#include <ctime>
#include <inttypes.h>
#include "2048.h"

typedef uint64_t board_t;
typedef uint16_t row_t;

const board_t ROW_MASK = 0xFFFFULL;
const board_t COL_MASK = 0x000F000F000F000FULL;

typedef std::map<board_t, double> trans_table_t;

const double TILE_GEN_PROB = (TILE_GEN_RATIO / (TILE_GEN_RATIO + 1.0));

struct eval_state {	
	trans_table_t trans_table; // transposition table, to cache previously-seen moves
	double cprob_thresh;
	int max_depth;
	int current_depth;
	int cache_hits;
	int moves_evaled;

	eval_state() : cprob_thresh(0), max_depth(0), current_depth(0), cache_hits(0), moves_evaled(0) {
	}
};

class Bot {
private:	
	int CACHE_DEPTH_LIMIT;
	int SEARCH_DEPTH_LIMIT;
	// paramereter
	double CPROB_THRESH_BASE;
	double EMPTY_TILE_HEURISTIC_SCORE;
	double EDGE_TILE_HEURISTIC_SCORE;
	double CLOSE_TILE_HEURISTIC_SCORE;
	double ORDERED_TILE_HEURISTIC_SCORE;
	double DEPTH_CHANGE_FREQ;

	int _cache_depth_limit;
	int _search_depth_limit;
	board_t _board;
	double _start_time, _end_time, _last_depth_change_time;
	int _move_count;
	// lookup tables	
	row_t _row_left_table[65536];
	row_t _row_right_table[65536];
	board_t _col_up_table[65536];
	board_t _col_down_table[65536];
	double _heur_score_table[65536];
	double _score_table[65536];
	// member function
	void initial_board();
	void initialize_tables();
	board_t execute_move(board_t board, dir_e move);
	dir_e find_best_move();	
	double expectimax_top_move(dir_e move);
	double expectimax_env_move(eval_state &state, board_t board, double cprob);
	double expectimax_after_env_move(eval_state &state, board_t board, double cprob);
	double score_heur_board(board_t board);
	double score_board(board_t board);
	double compute_score(board_t board, const double* table);
	
	// utility member function
	static unsigned unif_random(unsigned n);
	static board_t draw_tile();
	static board_t insert_tile_rand(board_t board, board_t tile);
	static board_t transpose(board_t board);
	static int count_empty(board_t board);
	static board_t unpack_col(row_t row);
	static row_t reverse_row(row_t row);
	static int get_max_rank(board_t board);
	static bool contain(board_t board, uint64_t n);
	static board_t grid_to_board(Grid &grid);
	static void print_grid(const board_t &board);
	static void print_grid(board_t board, int row, int column);
	static double cpuTime();
public:
	// ctor
	Bot();
	Bot(double cprob_thresh_base, double empty_tile_heuristic_score, double edge_tile_heuristic_score, double close_tile_heuristic_score, double ordered_tile_heuristic_score, double depth_change_freq);
	// dtor
	~Bot() {};

	void Run(int n);
};

/*************************************************
Game inline
*************************************************/
inline 
unsigned Bot::unif_random(unsigned n) {
	static int seeded = 0;

	if (!seeded) {
		srand(time(NULL));
		seeded = 1;
	}
	return rand() % n;
}

inline
board_t Bot::draw_tile() {
	return (unif_random(10) < 9) ? 1 : 2;
}

inline
board_t Bot::insert_tile_rand(board_t board, board_t tile) {
	int index = unif_random(count_empty(board));
	board_t tmp = board;
	while (true) {
		while ((tmp & 0xf) != 0) {
			tmp >>= 4;
			tile <<= 4;
		}
		if (index == 0) break;
		--index;
		tmp >>= 4;
		tile <<= 4;
	}
	return board | tile;
}

inline
board_t Bot::execute_move(board_t board, dir_e move){
	board_t ret = board;
	board_t t = transpose(board);

	switch (move) {
	case UP: // up
		ret ^= _col_up_table[(t >> 0) & ROW_MASK] << 0;
		ret ^= _col_up_table[(t >> 16) & ROW_MASK] << 4;
		ret ^= _col_up_table[(t >> 32) & ROW_MASK] << 8;
		ret ^= _col_up_table[(t >> 48) & ROW_MASK] << 12;
		break;
	case DOWN: // down
		ret ^= _col_down_table[(t >> 0) & ROW_MASK] << 0;
		ret ^= _col_down_table[(t >> 16) & ROW_MASK] << 4;
		ret ^= _col_down_table[(t >> 32) & ROW_MASK] << 8;
		ret ^= _col_down_table[(t >> 48) & ROW_MASK] << 12;
		break;
	case LEFT: // left
		ret ^= board_t(_row_left_table[(board >> 0) & ROW_MASK]) << 0;
		ret ^= board_t(_row_left_table[(board >> 16) & ROW_MASK]) << 16;
		ret ^= board_t(_row_left_table[(board >> 32) & ROW_MASK]) << 32;
		ret ^= board_t(_row_left_table[(board >> 48) & ROW_MASK]) << 48;
		break;
	case RIGHT: // right
		ret ^= board_t(_row_right_table[(board >> 0) & ROW_MASK]) << 0;
		ret ^= board_t(_row_right_table[(board >> 16) & ROW_MASK]) << 16;
		ret ^= board_t(_row_right_table[(board >> 32) & ROW_MASK]) << 32;
		ret ^= board_t(_row_right_table[(board >> 48) & ROW_MASK]) << 48;
		break;
	default:
		break;
	}
	return ret;
}

inline
void Bot::print_grid(board_t board, int row, int column) 
{
	board >>= (row * 4 + column) * 4;
	std::cout << "0123456789abcdef"[board & 0xf];
}

inline 
void Bot::print_grid(const board_t &board)
{
	for (int row = 0; row < 4; row++)
	{
		for (int column = 0; column < 4; column++)
			print_grid(board, row, column);
		std::cout << std::endl;
	}
}

inline
board_t Bot::transpose(board_t board)
{
	board_t a1 = board & 0xF0F00F0FF0F00F0FULL;
	board_t a2 = board & 0x0000F0F00000F0F0ULL;
	board_t a3 = board & 0x0F0F00000F0F0000ULL;
	board_t a = a1 | (a2 << 12) | (a3 >> 12);
	board_t b1 = a & 0xFF00FF0000FF00FFULL;
	board_t b2 = a & 0x00FF00FF00000000ULL;
	board_t b3 = a & 0x00000000FF00FF00ULL;
	return b1 | (b2 >> 24) | (b3 << 24);
}

inline
int Bot::count_empty(board_t board)
{
	board |= (board >> 2) & 0x3333333333333333ULL;
	board |= (board >> 1);
	board = ~board & 0x1111111111111111ULL;
	// At this point each nibble is:
	//  0 if the original nibble was non-zero
	//  1 if the original nibble was zero
	// Next sum them all
	board += board >> 32;
	board += board >> 16;
	board += board >> 8;
	board += board >> 4; // this can overflow to the next nibble if there were 16 empty positions
	return board & 0xf;
}

inline 
board_t Bot::unpack_col(row_t row) {
	board_t tmp = row;
	return (tmp | (tmp << 12ULL) | (tmp << 24ULL) | (tmp << 36ULL)) & COL_MASK;
}

inline 
row_t Bot::reverse_row(row_t row) {
	return (row >> 12) | ((row >> 4) & 0x00F0) | ((row << 4) & 0x0F00) | (row << 12);
}

inline 
int Bot::get_max_rank(board_t board)
{
	int maxrank = 0;
	while (board)
	{
		maxrank = std::max<int>(maxrank, int(board & 0xf));
		board >>= 4;
	}
	return maxrank;
}

inline
bool Bot::contain(board_t board, uint64_t n)
{
	while (board) 
	{
		if ((board & 0xf) == n)
			return true;
		board >>= 4;
	}
	return false;
}

inline
board_t Bot::grid_to_board(Grid &grid)
{
	board_t board = 0;

	for (int i = 15; i >= 0; i--)
	{
		board <<= 4;
		int num = grid[i];
		unsigned d = 0;
		while (num >>= 1) { ++d; }
		board |= d;
	}
	return board;
}

inline
double Bot::cpuTime()
{
#ifdef _WIN32
	FILETIME a, b, c, d;
	if (GetProcessTimes(GetCurrentProcess(), &a, &b, &c, &d) != 0)
		return (double)(d.dwLowDateTime | ((unsigned long long)d.dwHighDateTime << 32)) * 0.0000001;
	else
		return 0;
#elif defined(__linux__)
	return (double)clock() / CLOCKS_PER_SEC;
#endif
}

#endif // !BOT_H_