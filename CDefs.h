static const int BOARD_SIZE = 8;

enum EChipColor {
	White = 0,
	Black = 1,
	NoColor = 2
};

struct ReversiPosition {
	ReversiPosition(int c = -1, int r = -1) : row(r), col(c) {}

	int row;
	int col;

	bool isValid() const {
		return (row >= 0 && col >= 0 && row < BOARD_SIZE && col < BOARD_SIZE);
	}
};

struct ReversiMove : public ReversiPosition {
	ReversiMove(EChipColor col = NoColor, int c = -1, int r = -1) : ReversiPosition(c, r), color(col) {}
	ReversiMove(EChipColor col, const ReversiPosition& position) : ReversiPosition(position), color(col) {}

	EChipColor color;

	bool isValid() const {
		return (color != NoColor && row >= 0 && col >= 0 && row < BOARD_SIZE && col < BOARD_SIZE);
	}
};
