#include "Bot.h"

Bot::Bot():
CACHE_DEPTH_LIMIT(7), SEARCH_DEPTH_LIMIT(8), CPROB_THRESH_BASE(0.0001), _cache_depth_limit(6), _search_depth_limit(7), EMPTY_TILE_HEURISTIC_SCORE(10000.0), EDGE_TILE_HEURISTIC_SCORE(20000.0), CLOSE_TILE_HEURISTIC_SCORE(1000.0), ORDERED_TILE_HEURISTIC_SCORE(10000.0), DEPTH_CHANGE_FREQ(0.1),
_board(0), _start_time(0), _end_time(0), _last_depth_change_time(0),
_move_count(0)
{
	initialize_tables();
}

Bot::Bot(double cprob_thresh_base, double empty_tile_heuristic_score, double edge_tile_heuristic_score, double close_tile_heuristic_score, double ordered_tile_heuristic_score, double depth_change_freq) :
CACHE_DEPTH_LIMIT(7), SEARCH_DEPTH_LIMIT(8),
CPROB_THRESH_BASE(cprob_thresh_base), _cache_depth_limit(6), 
_search_depth_limit(7), EMPTY_TILE_HEURISTIC_SCORE(empty_tile_heuristic_score),
EDGE_TILE_HEURISTIC_SCORE(edge_tile_heuristic_score), CLOSE_TILE_HEURISTIC_SCORE(close_tile_heuristic_score),
ORDERED_TILE_HEURISTIC_SCORE(ordered_tile_heuristic_score), DEPTH_CHANGE_FREQ(depth_change_freq),
_board(0), _start_time(0), _end_time(0), _last_depth_change_time(0),
_move_count(0)
{
	initialize_tables();
}

void Bot::Run(int n)
{
	dir_e move;
	Game game = Game();
	_move_count = 0;
	_start_time = cpuTime();

	for (int i = 0; i < n; i++)
	{
		Grid grid;
		game.getCurrentGrid(grid);
		_board = grid_to_board(grid);
		
		while (!game.isGameOver())
		{
			game.getCurrentGrid(grid);
			_board = grid_to_board(grid);
			move = find_best_move();
			if (move == INVALID)
				break;
			game.insertDirection(move);
			_move_count++;
			// dynamic change the search depth
			if ((_move_count / (cpuTime() - _start_time)) > 105)
			{
				if (_cache_depth_limit < CACHE_DEPTH_LIMIT)
				{
					_cache_depth_limit++;
					_last_depth_change_time = cpuTime();
				}
				if (_search_depth_limit < SEARCH_DEPTH_LIMIT)
				{
					_search_depth_limit++;
					_last_depth_change_time = cpuTime();
				}
			}
			else if ((cpuTime() - _last_depth_change_time) > DEPTH_CHANGE_FREQ)
			{
				if (_cache_depth_limit > 2)
				{
					_cache_depth_limit--;
					_last_depth_change_time = cpuTime();
				}
				if (_search_depth_limit > 2)
				{
					_search_depth_limit--;
					_last_depth_change_time = cpuTime();
				}
			}
		}
		if (i < n - 1) 
		{
			game.reset();
		}
	}
	_end_time = cpuTime();
}

void Bot::initial_board() 
{
	board_t board = draw_tile() << (4 * unif_random(16));
	_board = insert_tile_rand(board, draw_tile());
}

void Bot::initialize_tables()
{
	for (unsigned row = 0; row < 65536; ++row)
	{
		unsigned line[4] =
		{
			(row >> 0) & 0xf,
			(row >> 4) & 0xf,
			(row >> 8) & 0xf,
			(row >> 12) & 0xf
		};
		double heur_score = 0.0f;
		double score = 0.0f;

		for (int i = 0; i < 4; ++i)
		{
			int rank = line[i];
			if (rank == 0)// row空格數越多，分數越高
			{
				heur_score += EMPTY_TILE_HEURISTIC_SCORE;
			}
			else if (rank >= 2)
			{
				// the score is the total sum of the tile and all intermediate merged tiles
				score += (rank - 1) * (1 << rank);
			}
		}
		_score_table[row] = score;

		int max_col = 0;
		for (int i = 1; i < 4; ++i)
		{
			if (line[i] > line[max_col]) 
				max_col = i;
		}
		// row最大分數的在邊界，分數越高
		if (max_col == 0 || max_col == 3)
		{
			heur_score += EDGE_TILE_HEURISTIC_SCORE;
		}
		// Check if maxi's are close to each other, and of diff ranks (eg 128 256)
		for (int i = 1; i < 4; ++i) 
		{
			if ((line[i] == line[i - 1] + 1) || (line[i] == line[i - 1] - 1)) heur_score += CLOSE_TILE_HEURISTIC_SCORE;
		}
		// Check if the values are ordered:
		if ((line[0] < line[1]) && (line[1] < line[2]) && (line[2] < line[3]))
		{
			heur_score += ORDERED_TILE_HEURISTIC_SCORE;
		}
		if ((line[0] > line[1]) && (line[1] > line[2]) && (line[2] > line[3]))
		{
			heur_score += ORDERED_TILE_HEURISTIC_SCORE;
		}
		_heur_score_table[row] = heur_score;

		// execute a move to the left
		for (int i = 0; i < 3; ++i)
		{
			int j;// row由左至右，第一個不是非空格的位置
			for (j = i + 1; j < 4; ++j)
			{
				if (line[j] != 0) break;
			}
			if (j == 4) break; // no more tiles to the right

			if (line[i] == 0)
			{
				line[i] = line[j];
				line[j] = 0;
				i--; // retry this entry
			}
			else if (line[i] == line[j] && line[i] != 0xf)
			{
				line[i]++;
				line[j] = 0;
			}
		}
		row_t result = (line[0] << 0) |
			(line[1] << 4) |
			(line[2] << 8) |
			(line[3] << 12);
		row_t rev_result = reverse_row(result);
		unsigned rev_row = reverse_row(row);

		_row_left_table[row] = row ^ result;
		_row_right_table[rev_row] = rev_row ^ rev_result;
		_col_up_table[row] = unpack_col(row) ^ unpack_col(result);
		_col_down_table[rev_row] = unpack_col(rev_row) ^ unpack_col(rev_result);
	}
}
// find the best move in current state of board
dir_e Bot::find_best_move()
{
	double best = 0;
	dir_e bestmove = INVALID;
	
	for (dir_e move = LEFT; move < INVALID; move = dir_e(move+1)) 
	{
		double res = expectimax_top_move(move);

		if (res > best) 
		{
			best = res;
			bestmove = move;
		}
	}
	return bestmove;
}
// 玩家下第一步 
double Bot::expectimax_top_move(dir_e move) {
	eval_state state;
	state.cprob_thresh = CPROB_THRESH_BASE;

	board_t newboard = execute_move(_board, move);

	if (_board == newboard)
		return 0;
	double res = expectimax_env_move(state, newboard, 1.0);
	//printf("Move %d: result %f: eval'd %d moves (%d cache hits, %d cache size) (maxdepth=%d)\n", move, res, state.moves_evaled, state.cache_hits, (int)state.trans_table.size(), state.max_depth);
	return res;
}
// 計算遊戲環境產生的變化
double Bot::expectimax_env_move(eval_state &state, board_t board, double cprob) 
{
	int num_open = count_empty(board);
	cprob /= num_open;// 移動一步後，剩下的格子被選到產生新tile的機率
	double res = 0.0;
	board_t tmp = board;
	board_t tile_2 = 1;// mask used to retrieve the n-th tile

	while (tile_2) 
	{
		if ((tmp & 0xf) == 0)// tile is empty 
		{
			res += expectimax_after_env_move(state, board | tile_2, cprob * TILE_GEN_PROB) * TILE_GEN_PROB;// if tile is occupied with new tile(2)
			res += expectimax_after_env_move(state, board | (tile_2 << 1), cprob * (1 - TILE_GEN_PROB)) * (1 - TILE_GEN_PROB);// if tile is occupied with new tile(4)
		}
		tmp >>= 4;
		tile_2 <<= 4;
	}
	return res / num_open;// expected value
}
// 新遊戲環境此時怎麼下
double Bot::expectimax_after_env_move(eval_state &state, board_t board, double cprob)
{
	// limits has reached
	if (cprob < state.cprob_thresh || state.current_depth >= _search_depth_limit)
	{
		if (state.current_depth > state.max_depth)
			state.max_depth = state.current_depth;
		return score_heur_board(board);
	}
	// lookup transposition table
	if (state.current_depth <= _cache_depth_limit)
	{
		const trans_table_t::iterator &i = state.trans_table.find(board);
		const trans_table_t::iterator &j = state.trans_table.find(transpose(board));
		if (i != state.trans_table.end())
		{
			state.cache_hits++;
			return i->second;
		}
		if (j != state.trans_table.end())
		{
			state.cache_hits++;
			return j->second;
		}
	}
	double best = 0.0;
	state.current_depth++;

	for (dir_e move = LEFT; move < INVALID; move = dir_e(move + 1))
	{
		board_t newboard = execute_move(board, move);
		state.moves_evaled++;

		if (board != newboard) 
		{
			best = std::max<double>(best, expectimax_env_move(state, newboard, cprob));
		}
	}
	state.current_depth--;
	// save to transposition table
	if (state.current_depth <= _cache_depth_limit) 
	{
		state.trans_table[board] = best;
		state.trans_table[transpose(board)] = best;
	}
	return best;
}
// score a single board heuristically
double Bot::score_heur_board(board_t board)
{
	return compute_score(board, _heur_score_table) + compute_score(transpose(board), _heur_score_table);
}
// score a single board actually (adding in the score from spawned 4 tiles)
double Bot::score_board(board_t board)
{
	return compute_score(board, _score_table);
}

double Bot::compute_score(board_t board, const double* table) {
	return table[(board >> 0) & ROW_MASK] +
		table[(board >> 16) & ROW_MASK] +
		table[(board >> 32) & ROW_MASK] +
		table[(board >> 48) & ROW_MASK];
}