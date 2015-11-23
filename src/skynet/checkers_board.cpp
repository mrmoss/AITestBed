#include "checkers_board.hpp"

#include <cctype>
#include <sstream>
#include <stdexcept>

static const char look_move[32][4]={{-1,-1,5,4},  {-1,-1,6,5},  {-1,-1,7,6},  {-1,-1,-1,7}, {-1,0,8,-1},  {0,1,9,8},    {1,2,10,9},   {2,3,11,10},
		     	     {4,5,13,12},  {5,6,14,13},  {6,7,15,14},  {7,-1,-1,15}, {-1,8,16,-1}, {8,9,17,16},  {9,10,18,17}, {10,11,19,18},
		     	     {12,13,21,20},{13,14,22,21},{14,15,23,22},{15,-1,-1,23},{-1,16,24,-1},{16,17,25,24},{17,18,26,25},{18,19,27,26},
		     	     {20,21,29,28},{21,22,30,29},{22,23,31,30},{23,-1,-1,31},{-1,24,-1,-1},{24,25,-1,-1},{25,26,-1,-1},{26,27,-1,-1}};

static const char look_jump[32][4]={{-1,-1,9,-1}, {-1,-1,10,8}, {-1,-1,11,9}, {-1,-1,-1,10},{-1,-1,13,-1},{-1,-1,14,12},{-1,-1,15,13},{-1,-1,-1,14},
			     {-1,1,17,-1}, {0,2,18,16},  {1,3,19,17},  {2,-1,-1,18}, {-1,5,21,-1}, {4,6,22,20},  {5,7,23,21},  {6,-1,-1,22},
			     {-1,9,25,-1}, {8,10,26,24}, {9,11,27,25}, {10,-1,-1,26},{-1,13,29,-1},{12,14,30,28},{13,15,31,29},{14,-1,-1,30},
			     {-1,17,-1,-1},{16,18,-1,-1},{17,19,-1,-1},{18,-1,-1,-1},{-1,21,-1,-1},{20,22,-1,-1},{21,23,-1,-1},{22,-1,-1,-1}};

static bool is_red_pawn(const char piece)
{
	return (piece=='r');
}

static bool is_red_king(const char piece)
{
	return (piece=='R');
}

static bool is_blk_pawn(const char piece)
{
	return (piece=='b');
}

static bool is_blk_king(const char piece)
{
	return (piece=='B');
}

static bool is_empty(const char piece)
{
	return (piece=='_');
}

static bool is_red(const char piece)
{
	return (is_red_pawn(piece)||is_red_king(piece));
}

static bool is_blk(const char piece)
{
	return (is_blk_pawn(piece)||is_blk_king(piece));
}

static bool not_same_color(const char piece1,const char piece2)
{
	return ((is_red(piece1)&&is_blk(piece2))||(is_blk(piece1)&&is_red(piece2)));
}

static bool find_jump(const skynet::checkers_board_t& board,const size_t position,std::vector<skynet::checkers_board_t>& boards)
{
	bool ret=false;
	bool start=true;
	bool found=false;
	size_t rsp=0;
	const size_t stack_size=20;
	size_t stack[stack_size][2];
	skynet::checkers_board_t board_stack[stack_size];
	board_stack[0]=board;
	stack[0][0]=position;
	stack[0][1]=0;

	while(start||rsp>=0)
	{
		start=false;

		while(stack[rsp][1]<4)
		{
			if(look_move[stack[rsp][0]][stack[rsp][1]]>-1&&
				not_same_color(board_stack[rsp][look_move[stack[rsp][0]][stack[rsp][1]]],board_stack[rsp][stack[rsp][0]])&&
				look_jump[stack[rsp][0]][stack[rsp][1]]>-1&&is_empty(board_stack[rsp][look_jump[stack[rsp][0]][stack[rsp][1]]]))
			{
				if((stack[rsp][1]>1&&(is_red(board_stack[rsp][stack[rsp][0]])||is_blk_king(board_stack[rsp][stack[rsp][0]])))||
					(stack[rsp][1]<2&&(is_blk(board_stack[rsp][stack[rsp][0]])||is_red_king(board_stack[rsp][stack[rsp][0]]))))
				{
					found=true;
					++rsp;
					board_stack[rsp]=board_stack[rsp-1];
					board_stack[rsp][look_jump[stack[rsp-1][0]][stack[rsp-1][1]]]=board_stack[rsp-1][stack[rsp-1][0]];
					board_stack[rsp][stack[rsp-1][0]]='_';
					board_stack[rsp][look_move[stack[rsp-1][0]][stack[rsp-1][1]]]='_';
					stack[rsp][0]=look_jump[stack[rsp-1][0]][stack[rsp-1][1]];
					stack[rsp][1]=0;
					break;
				}
			}

			++stack[rsp][1];
		}

		if(rsp>0&&found&&stack[rsp][1]>3)
		{
			found=false;

			for(size_t ii=0;ii<4;++ii)
				if(is_blk(board_stack[rsp][ii]))
					board_stack[rsp][ii]=toupper(board_stack[rsp][ii]);

			for(size_t ii=28;ii<32;++ii)
				if(is_red(board_stack[rsp][ii]))
					board_stack[rsp][ii]=toupper(board_stack[rsp][ii]);

			boards.push_back(board_stack[rsp]);
			ret=true;
			--rsp;
			++stack[rsp][1];
		}

		if(!found&&stack[rsp][1]>3)
		{
			if(rsp==0)
				break;

			--rsp;
			++stack[rsp][1];
		}
	}

	return ret;
}

bool skynet::is_valid(const skynet::checkers_board_t& board)
{
	if(board.size()!=32)
		return false;

	for(auto ii:board)
		if(ii!='r'&&ii!='R'&&ii!='b'&&ii!='B'&&ii!='_')
			return false;

	return true;
}

skynet::checkers_board_list_t skynet::move_generator(const skynet::checkers_board_t& board,const skynet::checkers_player_t& player)
{
	if(!is_valid(board))
		throw std::runtime_error("Invalid board \""+board+"\".");

	if(player!="red"&&player!="black")
		throw std::runtime_error("Invalid player \""+player+"\" (expected \"red\" or \"black\").");

	skynet::checkers_board_list_t boards;

	bool jump=false;

	//Find Jumps
	for(size_t ii=0;ii<32;++ii)
		if((is_red(board[ii])&&player=="red")||(is_blk(board[ii])&&player=="black"))
			jump|=find_jump(board,ii,boards);

	//Find Moves
	for(size_t ii=0;ii<32&&!jump;++ii)
	{
		//Check Black pieces and Red Kings
		if((is_red_king(board[ii])&&player=="red")||(is_blk(board[ii])&&player=="black"))
		{
			for(size_t jj=0;jj<2;++jj)
			{
				if(look_move[ii][jj]>-1&&is_empty(board[look_move[ii][jj]]))
				{
					skynet::checkers_board_t b(board);
					b[ii]='_';

					if(look_move[ii][jj]<4&&is_blk(board[ii]))
						b[look_move[ii][jj]]='B';
					else
						b[look_move[ii][jj]]=board[ii];

					boards.push_back(b);
				}
			}
		}

		//Check Black Kings and Red pieces
		if((is_red(board[ii])&&player=="red")||(is_blk_king(board[ii])&&player=="black"))
		{
			for(size_t jj=2;jj<4;++jj)
			{
				if(look_move[ii][jj]>-1&&is_empty(board[look_move[ii][jj]]))
				{
					skynet::checkers_board_t b(board);
					b[ii]='_';

					if(look_move[ii][jj]>27&&is_red(board[ii]))
						b[look_move[ii][jj]]='R';
					else
						b[look_move[ii][jj]]=board[ii];

					boards.push_back(b);
				}
			}
		}
	}

	return boards;
}
