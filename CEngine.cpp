#include <cstdlib>
#include <ctime>
#include "CEngine.h"
#include <iostream>

CMoveValue::CMoveValue() {
	setParameters(0, 0, 0);
}

CMoveValue::CMoveValue(int x, int y, int value) {
	setParameters(x, y, value);
}

void CMoveValue::setParameters(int x, int y, int value) {
	mv_x = x;
	mv_y = y;
	mv_value = value;
}

int CMoveValue::getX() const {
	return mv_x;
}

int CMoveValue::getY() const {
	return mv_y;
}

void CSquareStackEntry::setXY(int x, int y) {
	sse_x = x;
	sse_y = y;
}

int CSquareStackEntry::getX() const {
	return sse_x;
}

int CSquareStackEntry::getY() const {
	return sse_y;
}

CSquareStack::CSquareStack() {
	init(0);
}

CSquareStack::CSquareStack(int size) {
	init(size);
}

void CSquareStack::resize(int size) {
	ss_stack.resize(size);
}

void CSquareStack::init(int size) {
	resize(size);
	ss_top = 0;
	for(int i = 0; i < size; ++i) ss_stack[i].setXY(0, 0);
}

CSquareStackEntry CSquareStack::pop() {
	return ss_stack[--ss_top];
}

void CSquareStack::push(int x, int y) {
	ss_stack[ss_top].setXY(x, y);
	++ss_top;
}

CEngine::CEngine(EChipColor playerColor, EChipColor opponentColor) {
	e_score = new CScore();
	e_boardContolScore = new CScore();
	e_playerColor = playerColor;
	e_opponentColor = opponentColor;
	SetupBits();
	SetupLastBoardState();
	e_score->setScore(EChipColor::White, 2);
	e_score->setScore(EChipColor::Black, 2);
}

CEngine::~CEngine() {
	delete e_score;
	delete e_boardContolScore;
}

void CEngine::SetupLastBoardState() {
	for(int x = 0; x < BOARD_SIZE; ++x)
		for(int y = 0; y < BOARD_SIZE; ++y)
			e_lastBoardState[x][y] = EChipColor::NoColor;

	e_lastBoardState[3][3] = e_lastBoardState[4][4] = EChipColor::White;
	e_lastBoardState[4][3] = e_lastBoardState[3][4] = EChipColor::Black;
}

void CEngine::SetChipColor(const ReversiMove& move) {
	if(e_lastBoardState[move.col][move.row] != EChipColor::NoColor) 
		e_score->dec(e_lastBoardState[move.col][move.row]);
	e_lastBoardState[move.col][move.row] = move.color;
	if(move.color != EChipColor::NoColor) e_score->inc(move.color);
}

void CEngine::TurnChips(const ReversiMove& move/*, bool check*/) {
	SetChipColor(move);
	for(int i = 0; i < DIRECTIONS; ++i) {
		if(!IsFlippable(i, move)) continue;
		for(int x = move.col + e_arrDX[i], y = move.row + e_arrDY[i]; x >= 0 && x < 8 && y >= 0 && y < 8;
				x += e_arrDX[i], y += e_arrDY[i]) {
			if(e_lastBoardState[x][y] == move.color) break;
			SetChipColor(ReversiMove(move.color, x, y));
		}
	}
}

bool CEngine::IsFlippable(int direction, const ReversiMove& move) const {
	int opponentChipsSum = 0;
	bool foundPlayerColor = false;

	EChipColor localOpponentColor = (move.color == EChipColor::White) ? EChipColor::Black : EChipColor::White;
	EChipColor localPlayerColor = move.color;

	for(int x = move.col + e_arrDX[direction], y = move.row + e_arrDY[direction]; x >= 0 && x < 8 && y >= 0 && y < 8;
			x += e_arrDX[direction], y += e_arrDY[direction]) {
		EChipColor currentColor = e_lastBoardState[x][y];
		if(currentColor == localOpponentColor) ++opponentChipsSum;
		else if(currentColor == localPlayerColor) {
			foundPlayerColor = true;
			break;
		}
		else break;
	}

	if(foundPlayerColor && opponentChipsSum != 0) return true;
	else return false;
}

void CEngine::opponentMove(int x, int y) {
	ReversiMove move(e_opponentColor, x, y);
	SetChipColor(move);
	TurnChips(move);
}

void CEngine::playerMove(int& xres, int& yres) { 
	ReversiMove move = DoMove();	
	xres = move.col;
	yres = move.row;
	SetChipColor(move);
	TurnChips(move);
}

int CEngine::CalcBoardControlScore(EChipColor color) {
	int sum = 0;	

	for(int x = 1; x < BOARD_SIZE + 1; ++x)
		for(int y = 1; y < BOARD_SIZE + 1; ++y)
			if(e_board[x][y] == color) sum += e_bcBoard[x][y];

	return sum;
}

void CEngine::CalcFrontierTiles() {
	int curX, curY;
	e_playerFrontierTiles = 0;
	e_opponentFrontierTiles = 0;

	for(int y = 0; y < BOARD_SIZE; ++y) {
		for(int x = 0; x < BOARD_SIZE; ++x) {
			if(e_board[x][y] != EChipColor::NoColor) {
				for(int k = 0; k < BOARD_SIZE; ++k) {
					curX = x + e_arrDX[k]; curY = y + e_arrDY[k];
					if(curX >= 0 && curX < BOARD_SIZE && curY >= 0 && curY < BOARD_SIZE && e_board[curX][curY] == EChipColor::NoColor) {
						if(e_board[x][y] == e_playerColor) ++e_playerFrontierTiles;
						else if(e_board[x][y] == e_opponentColor) ++e_opponentFrontierTiles;
					}
				}
			}
		}
	}
}

void CEngine::SetupBits() {
	unsigned long long int bits = 1;

	for(int y = 1; y < BOARD_SIZE + 1; ++y) {
		for(int x = 1; x < BOARD_SIZE + 1; ++x) {
			e_coordBit[x][y] = bits;
			bits *= 2;
		}
	}

	for(int y = 1; y < BOARD_SIZE + 1; ++y) {	
		for(int x = 1; x < BOARD_SIZE + 1; ++x) {
			e_neighbourBits[x][y] = 0;

			for(int dx = -1; dx <= 1; ++dx)
				for(int dy = -1; dy <= 1; ++dy)
					if(dx != 0 || dy != 0)
						if(x + dx > 0 && x + dx < BOARD_SIZE + 1 && y + dy > 0 && y + dy < BOARD_SIZE + 1)
							e_neighbourBits[x][y] |= e_coordBit[x + dx][y + dy];
		}
	}
}

unsigned long long int CEngine::ComputeOccupiedBits(EChipColor color) {
	unsigned long long int result = 0;
	for(int y = 0; y < BOARD_SIZE + 1; ++y) {
		for(int x = 0; x < BOARD_SIZE + 1; ++x) {
			if(e_board[x][y] == color) {
				result |= e_coordBit[x][y];
			}
		}
	}
	return result;
}

double CEngine::TryAllMoves(int depth, int cutoffval, unsigned long long int opponentBits, unsigned long long int playerBits) {
	double maxVal = -LARGE_DOUBLE;

	for(int y = 1; y < BOARD_SIZE + 1; ++y) {
		for(int x = 1; x < BOARD_SIZE + 1; ++x) {
			if(e_board[x][y] == EChipColor::NoColor && (e_neighbourBits[x][y] & playerBits) != 0) {
				double val = ComputeMove(x, y, depth + 1, maxVal, opponentBits, playerBits);
				if(val - ILLEGAL_VALUE < EPSILON && val > maxVal) {
					maxVal = val;
					if(maxVal > -cutoffval) break;
				}
			}
		}
		if(maxVal > (double)-cutoffval) break;
	}
	return maxVal;
}

bool CEngine::IsAbleMove(char opponent, char player, char* str) const {
	if(str[0] != opponent) return false;
	for(int ctr = 1; ctr < BOARD_SIZE; ++ctr) {
		if(str[ctr] == player) return true;
		if(str[ctr] == '-') return false;
	}

	return false;
}

bool CEngine::IsLegalMove(char opponent, char player, const ReversiMove& move) const {
	if(e_board[move.col][move.row] != EChipColor::NoColor) return false; 
	char str[10];
	int x, y, dx, dy, ctr;
	for(dy = -1; dy <= 1; ++dy) {
		for(dx = -1; dx <= 1; ++dx) {
			if(dx == 0 && dy == 0) continue;
			str[0] = '\0';
			for(ctr = 1; ctr < BOARD_SIZE + 1; ++ctr) {
				x = move.col + ctr * dx;
				y = move.row + ctr * dy;
				if(x >= 1 && x <= BOARD_SIZE && y >= 1 && y <= BOARD_SIZE) {
					switch(e_board[x][y]) {
						case EChipColor::White:
							str[ctr - 1] = 'w';
							break;
						case EChipColor::Black:
							str[ctr - 1] = 'b';
							break;
						case EChipColor::NoColor:
							str[ctr - 1] = '-';
							break;
					}
				}
				else str[ctr] = 0;
			}
			if(IsAbleMove(opponent, player, str)) return true;
		}
	}
	return false;
}

int CEngine::CountValidMoves(char opponent, char player) const {
	int result = 0;
	for(int y = 1; y < BOARD_SIZE + 1; ++y)
		for(int x = 1; x < BOARD_SIZE + 1; ++x)
			if(IsLegalMove(opponent, player, ReversiMove(EChipColor::NoColor, x, y))) ++result;
	return result;
}

double CEngine::EvaluatePosition(EChipColor playerColor) { //TODO: need better function
	double result;
	
	EChipColor opponentColor = (playerColor == EChipColor::White) ? EChipColor::Black : EChipColor::White;
	result = e_score->getScore(playerColor) - e_score->getScore(opponentColor);
	return result;
}

double CEngine::ComputeMove(int xplay, int yplay, int cutoffval, unsigned long long int playerBits,
		unsigned long long int opponentBits, int depth) {
	CSquareStackEntry stackEntry;
	int numberOfTurned = 0;
	e_nodesSearched++;

	e_board[xplay][yplay] = e_playerColor;
	playerBits |= e_coordBit[xplay][yplay];
	e_score->inc(e_playerColor);
	e_boardContolScore->addScore(e_playerColor, e_bcBoard[xplay][yplay]);

	for(int dx = -1; dx <= 1; ++dx) {
		for(int dy = -1; dy <= 1; ++dy) {
			if(dx == 0 && dy == 0) continue;
			int x, y;
			for(x = xplay + dx, y = yplay + dy; x >= 0 && y >= 0 && x < 8 && y < 8; x += dx, y += dy)
				if(e_board[x][y] != e_opponentColor) break;
			if(e_board[x][y] == e_playerColor) {
				for(x -= dx, y -= dy; x != xplay || y != yplay; x -= dx, y -= dy) {
					e_board[x][y] = e_playerColor;
					playerBits |= e_coordBit[x][y];
					opponentBits &= ~e_coordBit[x][y];
					e_stack.push(x, y);

					e_boardContolScore->addScore(e_playerColor, e_bcBoard[x][y]);
					e_boardContolScore->subScore(e_opponentColor, e_bcBoard[x][y]);
					numberOfTurned++;
				}
			}
		}
	}

	double result = -LARGE_DOUBLE;
	if(numberOfTurned > 0) {
		e_score->addScore(e_playerColor, numberOfTurned);
		e_score->subScore(e_opponentColor, numberOfTurned);
		if(depth >= e_depth) result = EvaluatePosition(e_playerColor);
		else {
			double maxVal = TryAllMoves(depth, cutoffval, opponentBits, playerBits);
			if(maxVal + LARGE_DOUBLE > EPSILON) result = -maxVal;
			else {
				result = TryAllMoves(depth, -LARGE_DOUBLE, opponentBits, playerBits);
				if(result + LARGE_DOUBLE < EPSILON) {
					int finalScore = e_score->getScore(e_playerColor) - e_score->getScore(e_opponentColor);
					if(e_exhaustive) result = finalScore;
					else {
						if(finalScore > 0) result = LARGE_DOUBLE - 65 + finalScore;
						else if(finalScore < 0) result = -(LARGE_DOUBLE - 65 + finalScore);
						else result = 0; 
					}
				}
			}
		}
		e_score->addScore(e_opponentColor, numberOfTurned);
		e_score->subScore(e_playerColor, numberOfTurned);
	}

	for(int i = numberOfTurned; i > 0; --i) {
		stackEntry = e_stack.pop();
		e_boardContolScore->addScore(e_opponentColor, e_bcBoard[stackEntry.getX()][stackEntry.getY()]);
		e_boardContolScore->subScore(e_playerColor, e_bcBoard[stackEntry.getX()][stackEntry.getY()]);
		e_board[stackEntry.getX()][stackEntry.getY()] = e_opponentColor;
	}
	
	e_board[xplay][yplay] = EChipColor::NoColor;
	e_score->subScore(e_playerColor, 1);
	e_boardContolScore->subScore(e_playerColor, e_bcBoard[xplay][yplay]);

	if(numberOfTurned < 1) return ILLEGAL_VALUE;
	else return result;
}

ReversiMove CEngine::DoMove() {
	e_exhaustive = false;
	e_stack.init(3000);
	e_depth = DEPTH;
	if(e_score->getScore(EChipColor::Black) + e_score->getScore(EChipColor::White) + e_depth + 3 >= BOARD_SIZE * BOARD_SIZE)
		e_depth = BOARD_SIZE * BOARD_SIZE - e_score->getScore(EChipColor::Black) - e_score->getScore(EChipColor::White);
	else if(e_score->getScore(EChipColor::Black) + e_score->getScore(EChipColor::White) + e_depth + 4 >= BOARD_SIZE * BOARD_SIZE) 
		e_depth += 2;
	else if(e_score->getScore(EChipColor::Black) + e_score->getScore(EChipColor::White) + e_depth + 5 >= BOARD_SIZE * BOARD_SIZE) 
		++e_depth;

	if(e_score->getScore(EChipColor::White) + e_score->getScore(EChipColor::Black) + e_depth >= BOARD_SIZE * BOARD_SIZE) e_exhaustive = true;

	//init board
	for(unsigned int y = 0; y < BOARD_SIZE + 2; ++y) {
		for(unsigned int x = 0; x < BOARD_SIZE + 2; ++x) {
			if(1 <= x && x <= BOARD_SIZE && 1 <= y && y <= BOARD_SIZE)
				e_board[x][y] = e_lastBoardState[x - 1][y - 1];
			else
				e_board[x][y] = EChipColor::NoColor;
		}
	}

	e_boardContolScore->setScore(EChipColor::White, CalcBoardControlScore(EChipColor::White));
	e_boardContolScore->setScore(EChipColor::Black, CalcBoardControlScore(EChipColor::Black));
	CalcFrontierTiles();
	
	unsigned long long int playerBits = ComputeOccupiedBits(e_playerColor);
	unsigned long long int opponentBits = ComputeOccupiedBits(e_opponentColor);

	double maxVal = -LARGE_DOUBLE;
	int maxX = 0;
	int maxY = 0;
	e_nodesSearched = 0;
	CMoveValue moves[60];
	int numberOfMoves = 0;
	int numberOfMaximums = 0;

	for(int y = 1; y < BOARD_SIZE + 1; ++y) {
		for(int x = 1; x < BOARD_SIZE + 1; ++x) {
			if((e_board[x][y] != EChipColor::NoColor) || ((e_neighbourBits[x][y] & opponentBits) == 0)) continue; 

			int val = ComputeMove(x, y, maxVal, playerBits, opponentBits, 1);
			if(val != ILLEGAL_VALUE) {
				moves[numberOfMoves++].setParameters(x, y, val);
				if(val > maxVal) {
					maxVal = val;
					maxX = x;
					maxY = y;

					numberOfMaximums = 1;
				}
				else if(val == maxVal) ++numberOfMaximums;
			}
		}
	}

	if(numberOfMaximums > 1) {
		srand(time(NULL));
		int moveNumber = rand() % numberOfMaximums;
		maxX = moves[moveNumber].getX();
		maxY = moves[moveNumber].getY();
	}

	if(maxVal + LARGE_DOUBLE > EPSILON) return ReversiMove(e_playerColor, maxX - 1, maxY - 1);
	else return ReversiMove(EChipColor::NoColor, -1, -1);
}
