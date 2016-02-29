#include <vector>
#include <assert.h>
#include <limits>
#include "CDefs.h"

#define LARGE_DOUBLE 99999.0
#define ILLEGAL_VALUE 8888.0
#define BOARD_CONTROL_WEIGHT 3
#define DIRECTIONS 8
#define DEPTH 15
#define EPSILON 0.0001

class CMoveValue {
	private:
		int mv_x;
		int mv_y;
		int mv_value;
	public:
		CMoveValue();
		CMoveValue(int x, int y, int value);
		void setParameters(int x, int y, int value);
		int getX() const;
		int getY() const;
};

class CScore {
	private:
		unsigned int s_score[2];
	public:
		CScore() {
			s_score[White] = 0;
			s_score[Black] = 0;
		}
		unsigned int getScore(EChipColor color) const {
			assert(color != EChipColor::NoColor);
			return s_score[color];
		}
		void setScore(EChipColor color, unsigned int value) {
			assert(color != EChipColor::NoColor);
			s_score[color] = value;
		}
		void inc(EChipColor color) {
			assert(color != EChipColor::NoColor);
			++s_score[color];
		}
		void dec(EChipColor color) {
			assert(color != EChipColor::NoColor);
			--s_score[color];
		}
		void addScore(EChipColor color, unsigned int value) {
			assert(color != EChipColor::NoColor);
			s_score[color] += value;
		}
		void subScore(EChipColor color, unsigned int value) {
			assert(color != EChipColor::NoColor);
			//assert(value <= s_score[color]);
			s_score[color] -= value;
		}
};

class CSquareStackEntry {
	private:
		int sse_x;
		int sse_y;
	public:		CSquareStackEntry() {}
		void setXY(int x, int y);
		int getX() const;
		int getY() const;
};

class CSquareStack {
	private:
		std::vector<CSquareStackEntry> ss_stack;
		int ss_top;
	public:
		CSquareStack();
		CSquareStack(int size);
		void resize(int size);
		void init(int size);
		CSquareStackEntry pop();
		void push(int x, int y);
};

class CEngine {
	private:
		const int e_arrDX[DIRECTIONS] = {0, 0, 1, 1, 1, -1, -1, -1};
		const int e_arrDY[DIRECTIONS] = {1, -1, 1, 0, -1, 1, 0, -1};
		int e_bcBoard[BOARD_SIZE + 1][BOARD_SIZE + 1] = {
			{20, -3, 11, 8, 8, 11, -3, 20},
	    		{-3, -7, -4, 1, 1, -4, -7, -3},
	    		{11, -4, 2, 2, 2, 2, -4, 11},
	    		{8, 1, 2, -3, -3, 2, 1, 8},
	    		{8, 1, 2, -3, -3, 2, 1, 8},
	    		{11, -4, 2, 2, 2, 2, -4, 11},
			{-3, -7, -4, 1, 1, -4, -7, -3},
	   		{20, -3, 11, 8, 8, 11, -3, 20},
		};	
		EChipColor e_playerColor;
		EChipColor e_opponentColor;
		EChipColor e_lastBoardState[BOARD_SIZE][BOARD_SIZE];
		EChipColor e_board[BOARD_SIZE + 2][BOARD_SIZE + 2];
		CScore* e_score;
		CScore* e_boardContolScore;
		CSquareStack e_stack;
		int e_depth;
		int e_coeff;
		int e_playerFrontierTiles;
		int e_opponentFrontierTiles;
		int e_nodesSearched;
		unsigned long long int e_coordBit[BOARD_SIZE + 1][BOARD_SIZE + 1];
		unsigned long long int e_neighbourBits[BOARD_SIZE + 1][BOARD_SIZE + 1];
		bool e_exhaustive;

		double ComputeMove(int xplay, int yplay, int cutoffval, unsigned long long int playerBits, 
				unsigned long long int opponentBits, int depth);
		ReversiMove DoMove();
		double TryAllMoves(int depth, int cutoffval, unsigned long long int opponentBits,
				unsigned long long int playerBits);
		double EvaluatePosition(EChipColor playerColor);
		void SetupBits();
		void SetupLastBoardState();
		int CalcBoardControlScore(EChipColor color);
		void CalcFrontierTiles();
		unsigned long long int ComputeOccupiedBits(EChipColor color);
		void SetChipColor(const ReversiMove& move);
		void TurnChips(const ReversiMove&  move);
		bool IsFlippable(int direction, const ReversiMove& move) const;
		bool IsLegalMove(char opponent, char player, const ReversiMove& move) const;
		bool IsAbleMove(char opponent, char player, char* str) const;
		int CountValidMoves(char opponent, char player) const;
	public:
		CEngine(EChipColor playerColor, EChipColor opponentColor);
		~CEngine();
		void opponentMove(int x, int y);
		void playerMove(int& resx, int& resy);
};
