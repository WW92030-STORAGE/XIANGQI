#ifndef XIANGQI_H
#define XIANGQI_H

#include <iostream>
#include <cstdint>
#include <vector>
#include <set>
#include <algorithm>

// This system uses some interchangeable names to be vocab-friendly.
// For example the RED side is here also referred to as WHITE (which is the side opposing black)
// Notation also includes a set of alternative symbols that align better with international chess (e.g. N vs. H)

class XPiece {
    public:
    uint16_t value;
    // RED BLACK ... PAWN KNIGHT ELEPHANT ROOK QUEEN KING CANNON
    // char ids[9] = {'_', '*', 'P', 'N', 'B', 'R', 'Q', 'K', 'C'}; // Nonstandard notation symbols more aligned with normal chess
    char ids[9] = {'_', '*', 'S', 'H', 'E', 'R', 'A', 'G', 'C'}; // More standard notation symbols
    
    XPiece() {
        value = 0;
    }
    
    XPiece(uint16_t c) {
        value = c;
    }
    
    XPiece(const XPiece& other) {
        value = other.value;
    }
    
    bool operator<(XPiece& other) { return value < other.value; }
    bool operator==(XPiece& other) { return value == other.value; }
    bool operator!=(XPiece& other) { return value != other.value; }
    
    bool isRed() { return value & (1<<0); }
    bool isWhite() { return isRed(); }
    
    bool isBlack() { return value & (1<<1); }
    
    bool isSoldier() { return value & (1<<2); }
    bool isPawn() { return isSoldier(); }
    
    bool isHorse() { return value & (1<<3); }
    bool isKnight() { return isHorse(); }
    
    bool isElephant() { return value & (1<<4); }
    bool isBishop() { return isElephant(); }
    
    bool isChariot() { return value & (1<<5); }
    bool isRook() { return isChariot(); }
    
    bool isAdvisor() { return value & (1<<6); }
    bool isQueen() { return isAdvisor(); }
    
    bool isGeneral() { return value & (1<<7); }
    bool isKing() { return isGeneral(); }
    
    bool isCannon() { return value & (1<<8); }
    
    bool isEmpty() { return value == 0; }
    bool getColor() { return isRed(); }
    
    int getID() {
        for (int i = 2; i < 9; i++) {
            if (value & (1<<i)) return i;
        }
        return 0;
    }
    
    bool isSlider() { // Cannons are not sliders
        return isKing() || isAdvisor() || isRook() || isElephant() || isPawn();
    }
    
    std::string toString() {
        if (value == 0) return "..";
        
        std::string res = "";
        for (int i = 0; i < 9; i++) {
            if (value & (1<<i)) res = res + ids[i];
        }
        
        if (res.length() > 2) res = res.substr(0, 2);
        while (res.length() < 2) res = res + "~";
        return res;
    }
};

struct Position {
    // lower 4 bits file upper 4 bits rank (x, y)
    uint8_t value = -1;
    
    Position() { value = -1; }
    Position(int file, int rank) { value = 16 * rank + file; }
    Position(std::pair<int, int> p) { value = 16 * p.second + p.first; }
    
    Position(const Position& other) {
        value = other.value;
    }
    
    bool operator<(Position& other) { return value < other.value; }
    bool operator==(Position& other) { return value == other.value; }
    bool operator!=(Position& other) { return value != other.value; }
    
    uint8_t file() {
        uint8_t res = value & 15;
        return res & 15;
    }
    
    uint8_t rank() {
        uint8_t res = value>>4;
        return res & 15;
    }
    
    std::pair<int, int> pos() {
        if (value == -1) return {-1, -1};
        return {(int)(file()), (int)(rank())};
    }

    std::string toString() {
        if (value == -1) return "..";
        std::string res = std::string(1, file() + 'a');
        return res + std::to_string((int)(rank()) + 1);
    }
};

bool operator==(const XPiece& a, const XPiece& b) {
    return a.value == b.value;
}

bool operator<(const XPiece& a, const XPiece& b) {
    return a.value < b.value;
}

struct XGame {
    bool sidetomove; // TRUE = red
    int halfmoveclock = 0;
    std::vector<XPiece> captures;
    
    int maxmoves = 100;
    XPiece board[9][10];
    
    // RED BLACK ... PAWN KNIGHT ELEPHANT ROOK QUEEN KING CANNON
    uint16_t backrank[9] = {(1<<5), (1<<3), (1<<4), (1<<6), (1<<7), (1<<6), (1<<4), (1<<3), (1<<5)};
    
    bool operator<(XGame& other) {
        if (sidetomove != other.sidetomove) return sidetomove < other.sidetomove;
        if (halfmoveclock != other.halfmoveclock) return halfmoveclock < other.halfmoveclock;
        if (captures != other.captures) return captures < other.captures;
        
        for (int x = 0; x < 9; x++) {
            for (int y = 0; y < 10; y++) {
                if (board[x][y] != other.board[x][y]) return board[x][y] < other.board[x][y];
            }
        }
        return false;
    }
    
    XGame() {
        sidetomove = true;
        halfmoveclock = 0;
        reset();
    }

	XGame(const XGame& game) {
		sidetomove = game.sidetomove;
		halfmoveclock = game.halfmoveclock;
		for (int i = 0; i < 9; i++) {
			for (int j = 0; j < 10; j++) board[i][j] = XPiece(game.board[i][j]);
		}
	}
    
    void reset() {
        sidetomove = true;
        halfmoveclock = 0;
        for (int x = 0; x < 9; x++) {
            for (int y = 0; y < 10; y++) board[x][y] = XPiece();
        }
        
        for (int x = 0; x < 9; x++) {
            board[x][0] = XPiece(backrank[x] | (1<<0));
            board[x][9] = XPiece(backrank[x] | (1<<1));
        }
        
        for (int x = 0; x < 9; x += 2) {
            board[x][3] = XPiece((1<<0) | (1<<2));
            board[x][6] = XPiece((1<<1) | (1<<2));
        }
        
        board[1][2] = XPiece((1<<0) | (1<<8));
        board[7][2] = XPiece((1<<0) | (1<<8));
        board[1][7] = XPiece((1<<1) | (1<<8));
        board[7][7] = XPiece((1<<1) | (1<<8));
    }
    
    std::string toString() {
        std::string res = "";
        for (int y = 9; y >= 0; y--) {
            for (int x = 0; x < 9; x++) res = res + board[x][y].toString() + " ";
            res = res + "\n";
        }
        
        res = res + "(" + (sidetomove ? "_RED_" : "BLACK") + ")";
        res = res + "<" + std::to_string(halfmoveclock) + ">\n";
        return res;
    }
    
    XPiece get(Position p) {
        return XPiece(board[p.file()][p.rank()]);
    }

	XPiece get(std::pair<int, int> p) {
		return XPiece(board[p.first][p.second]);
	}

	XPiece get(int x, int y) {
		return XPiece(board[x][y]);
	}
    
    bool isPoliticallyCorrect(Position p) { // Is on the originating side of the river?
		// if (get(p).isEmpty()) return false;
        bool b = sidetomove;
        if (b) return p.rank() < 5;
        else return p.rank() >= 5;
    }

	bool isPoliticallyCorrect(std::pair<int, int> p) {
		return isPoliticallyCorrect(Position(p));
	}

	bool isPoliticallyCorrect(int x, int y) {
		return isPoliticallyCorrect(Position(x, y));
	}
    
    bool isInPalace(Position p) {
		// if (get(p).isEmpty()) return false;
        bool x = sidetomove;
        if (x) return (p.rank() < 3) && (p.file() >= 3) && (p.file() <= 5);
        else return (p.rank() > 6) && (p.file() >= 3) && (p.file() <= 5);
    }

	bool isInPalace(std::pair<int, int> p) {
		return isInPalace(Position(p));
	}

	bool inBounds(std::pair<int, int> p) {
		if (p.first < 0 || p.second < 0) return false;
		if (p.first >= 9 || p.second >= 10) return false;
		return true;
	}

	bool inBounds(int x, int y) {
		return inBounds(std::make_pair(x, y));
	}
    
    // Generals and advisors cannot leave the palace
    // Elephants cannot cross the river
    // All other pieces can cross the river. Pawns in politically incorrect zones can move sideways.
    // In this game the rook is the strongest piece unlike the advisor (queen) because it has more movement freedom.
    
    // Ignoring all other pieces and location is the move vector a valid move?
    bool isLegalVector(XPiece p, int dx, int dy) {
        if (dx == 0 && dy == 0) return false;
		if (p.isGeneral()) return (dx * dy == 0) && (std::max(abs(dx), abs(dy)) == 1);
		if (p.isAdvisor()) return (abs(dx) == 1) && (abs(dy) == 1);
		if (p.isElephant()) return (abs(dx) == 2) && (abs(dy) == 2);
		if (p.isKnight()) return abs(dx * dy) == 2;
		if (p.isPawn()) {
			bool vert = sidetomove ? (dy >= 0) : (dy <= 0);
			return vert && (std::max(abs(dx), abs(dy)) <= 1);
		}
		if (p.isRook()) return (dx * dy == 0);
		if (p.isCannon()) return (dx * dy == 0);
		return false;
    }

	bool isLegalVector(XPiece p, std::pair<int, int> v) {
		return isLegalVector(p, v.first, v.second);
	}

	// Sliding can only occur on cardinals and ordinals. Knights and cannon captures are handled separately (but behave differently from international chess)
	bool isLegalSliding(std::pair<int, int> s, std::pair<int, int> e,  bool verbose = false) {
		if (!inBounds(s) || !inBounds(e)) return false;
		if (verbose) std::cout << "VALID ENDPOINT TEST PASSED\n";
		int dx = e.first - s.first;
		int dy = e.second - s.second;
		if (verbose) std::cout << "VEC " << dx << " " << dy << "\n";
		if (((dx * dy) != 0) && (abs(dx) != abs(dy))) return false;
		if (dx == 0 && dy == 0) return false;
		if (verbose) std::cout << "VALID SLIDING VEC TEST PASSED\n";
		
		int bx = (dx > 0) ? 1 : (dx < 0 ? -1 : 0);
        int by = (dy > 0) ? 1 : (dy < 0 ? -1 : 0);
        
        int x = s.first + bx;
        int y = s.second + by;
        
        for (int i = 0; i < std::max(abs(dx), abs(dy)) - 1; i++) {
            if (!inBounds({x, y})) return false; // what the fuck are you doing???
            if (!board[x][y].isEmpty()) return false; 
            x += bx;
            y += by;
        }
        return board[x][y].isEmpty() || (board[x][y].getColor() != board[s.first][s.second].getColor()); // can capture opposing pieces
	}

	bool pseudolegal(std::pair<int, int> src, std::pair<int, int> vec, bool verbose = false) {
		std::pair<int, int> des = {src.first + vec.first, src.second + vec.second};
		if (!inBounds(des)) return false;
		if (verbose) std::cout << "VALID DESTINATION TEST PASSED\n";

		XPiece piece = get(src);
		if (verbose) std::cout << "OFFENDING PIECE " << piece.toString() << "(" << piece.getColor() << ")\n";
		if (piece.isEmpty() || (piece.getColor() != sidetomove)) return false;
		if (verbose) std::cout << "PIECE VALIDITY TEST PASSED\n";

		if (!isLegalVector(piece, vec)) return false;
		if (verbose) std::cout << "VECTOR TEST PASSED\n";

		if (piece.isSlider()) {
			if (!isLegalSliding(src, des, verbose)) return false;
		}
		if (verbose) std::cout << "SLIDER TEST PASSED\n";
		// Casework
		if (piece.isRook()) return true;
		if (piece.isElephant()) {
			if (verbose) std::cout << Position(src).toString() << "\n";
			if (verbose) std::cout << Position(des).toString() << "\n";
			if (verbose) std::cout << isPoliticallyCorrect(src) << " " << isPoliticallyCorrect(des) << "\n";
			bool res = isPoliticallyCorrect(src) && isPoliticallyCorrect(des);
			if (verbose && !res)std::cout << "ELEPHANT INVALID SIDE\n";
			return res;
		}
		if (piece.isPawn()) {
			if (!isPoliticallyCorrect(src)) return true;
			return vec.first == 0;
		}
		if (piece.isKing()) return isInPalace(src) && isInPalace(des);
		if (piece.isAdvisor()) return isInPalace(src) && isInPalace(des);
		if (piece.isKnight()) {
			if (vec.first == 2) return isLegalSliding(src, {src.first + 1, src.second});
			if (vec.first == -2) return isLegalSliding(src, {src.first - 1, src.second});
			if (vec.second == 2) return isLegalSliding(src, {src.first, src.second + 1});
			if (vec.second == -2) return isLegalSliding(src, {src.first, src.second - 1});
			return false;
		}

		// NOTE - "i_hate_your_guts" is the name of an emote on a certain Discord server. The emote itself depicts an adorable (anthropomorphized) canine creature smiling cutely and holding a pink heart.

		if (piece.isCannon()) {
			int ihateyourguts = 0; // count of intermediate pieces
			int dx = vec.first;
			int dy = vec.second;

			int bx = (dx > 0) ? 1 : (dx < 0 ? -1 : 0);
        	int by = (dy > 0) ? 1 : (dy < 0 ? -1 : 0);
        
        	int x = src.first + bx;
        	int y = src.second + by;
        
        	for (int i = 0; i < std::max(abs(dx), abs(dy)) - 1; i++) {
            	if (!inBounds({x, y})) return false; // what the fuck are you doing???
            	if (!board[x][y].isEmpty()) ihateyourguts++;
            	x += bx;
            	y += by;
        	}

			if (ihateyourguts > 1) return false;
			if (ihateyourguts == 0) return get(des).isEmpty();
			if (ihateyourguts == 1) return (!get(des).isEmpty()) && (get(des).getColor() != piece.getColor());
		}

		return false;
	}

	// NOTE ABOUT DRAWS - The official rules are relatively ambiguous. According to the wikipedia page, arbitrary perpetual checking and chasing or periodic movements can result in a draw. Here we use a version of the 50 move rule.

	// Executes a move regardless of its legality (moves the piece at src along the vector vec)
	void execute(std::pair<int, int> src, std::pair<int, int> vec, bool verbose = false) {
		captures.clear();
		std::pair<int, int> des = {src.first + vec.first, src.second + vec.second};
		XPiece temp = get(src);
		if (!get(des).isEmpty()) halfmoveclock = 0;
		// else if (get(des).isPawn() && abs(vec.second) == 1) halfmoveclock = 0; // Forward pawn moves can reset the clock
		else halfmoveclock++; // Pawns can move sideways when on the opposing side so these do not reset the clock.
		board[src.first][src.second] = XPiece();

		if (!get(des).isEmpty()) {
			captures.push_back(get(des));
		}

		board[des.first][des.second] = temp;
	}

	std::vector<Position> getAllPieces(uint16_t value) {
        std::vector<Position> res;
        for (int x = 0; x < 9; x++) {
            for (int y = 0; y < 10; y++) if (board[x][y].value == value) res.push_back(Position(x, y));
        }
        return res;
    }

	bool noChecks() {
		int you = (sidetomove) ? (1<<0) : (1<<1);
        int opp = (sidetomove) ? (1<<1) : (1<<0);

		XPiece THISKING(you | (1<<7));
		XPiece OPPKING(opp | (1<<7));

		// Pawns
		for (auto p : getAllPieces(opp | (1<<2))) {
			int dx[3] = {01, 00, -1};
			int dy[3] = {00, 01, 00};

			for (int i = 0; i < 3; i++) {
				int x = p.file() + dx[i];
				int y = p.rank() + dy[i];
				if (!inBounds(x, y)) continue;
				if (get(x, y) == THISKING) return false;
			}
		}

		// Knights
		for (auto p : getAllPieces(opp | (1<<3))) {
			int dx[8] = {02, 01, -1, -2, -2, -1, 01, 02};
            int dy[8] = {01, 02, 02, 01, -1, -2, -2, -1};
			int bx[8] = {01, 00, 00, -1, -1, 00, 00, 01};
			int by[8] = {00, 01, 01, 00, 00, -1, -1, 00};

			for (int i = 0; i < 8; i++) {
				if (!inBounds({p.file() + dx[i], p.rank() + dy[i]})) continue;
				if (!inBounds({p.file() + bx[i], p.rank() + by[i]})) continue;
				if (!get(p.file() + bx[i], p.rank() + by[i]).isEmpty()) continue;
				if (get(p.file() + dx[i], p.rank() + dy[i]) == THISKING) return false;
			}
		}

		// Rooks

		for (auto p : getAllPieces(opp | (1<<5))) { // Rooks
            int dx[4] = {00, 01, 00, -1};
            int dy[4] = {01, 00, -1, 00};
            for (int i = 0; i < 4; i++) {
                for (int k = 1; k < 12; k++) {
                    if (!inBounds(p.file() + dx[i] * k, p.rank() + dy[i] * k)) break;
                    if (board[p.file() + dx[i] * k][p.rank() + dy[i] * k] == THISKING) return false;
                    if (!board[p.file() + dx[i] * k][p.rank() + dy[i] * k].isEmpty()) break;
                }
            }
        }

		// Elephants
		for (auto p : getAllPieces(opp | (1<<4))) {
			int dx[4] = {02, 02, -2, -2};
			int dy[4] = {02, -2, 02, -2};
			int bx[4] = {01, 01, -1, -1};
			int by[4] = {01, -1, 01, -1};

			for (int i = 0; i < 4; i++) {
				if (!inBounds({p.file() + dx[i], p.rank() + dy[i]})) continue;
				if (!inBounds({p.file() + bx[i], p.rank() + by[i]})) continue;
				if (get(!p.file() + bx[i], p.rank() + by[i]).isEmpty()) continue;
				if (get(p.file() + dx[i], p.rank() + dy[i]) == THISKING) return false;
			}
		}	

		// Advisors and generals can be ignored since they cannot get close to each other (the opposing rule is an exception however)

		std::vector<Position> TK = getAllPieces(THISKING.value);
		std::vector<Position> OK = getAllPieces(OPPKING.value);
		for (auto tk : TK) {
			for (auto ok : OK) {
				if (tk.file() == ok.file()) {
					bool facing = true;
					for (int i = std::min(tk.rank(), ok.rank()) + 1; i <= std::max(tk.rank(), ok.rank()) - 1; i++) {
						if (!get(tk.file(), i).isEmpty()) {
							facing = false;
							break;
						}
					}
					if (facing) return false;
				}
			}
		}

		std::vector<Position> CX = getAllPieces(opp | (1<<8));
		for (auto cx : CX) {
			for (auto tk : TK) {
				int ihateyourguts = 0;
				if (cx.rank() == tk.rank()) {
					for (int i = std::min(cx.file(), tk.file()) + 1; i <= std::max(cx.file(), tk.file()) - 1; i++) {
						if (!get(i, cx.rank()).isEmpty()) ihateyourguts++;
					}
					if (ihateyourguts == 1) return false;
				}
				else if (cx.file() == tk.file()) {
					for (int i = std::min(cx.rank(), tk.rank()) + 1; i <= std::max(cx.rank(), tk.rank()) - 1; i++) {
						if (!get(cx.file(), i).isEmpty()) ihateyourguts++;
					}
					if (ihateyourguts == 1) return false;
				}
			}
		}

		return true;
	}

	bool legal(std::pair<int, int> src, std::pair<int, int> vec, bool verbose = false) {
		if (!pseudolegal(src, vec, verbose)) return false;
		
		if (verbose) std::cout << "PSEUDOLEGAL TEST PASSED\n";
		XGame game2(*this);
		game2.execute(src, vec, false);
		if (verbose) std::cout << "NOW TESTING CHECKS...\n";
		bool res = game2.noChecks();
		if (verbose) std::cout << "CHECKS TEST " << res << "\n";
		return res;
	}
	std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> getAllLegalMoves() {
		std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> res;
		int you = (sidetomove) ? (1<<0) : (1<<1);
        int opp = (sidetomove) ? (1<<1) : (1<<0);

		XPiece THISKING(you | (1<<7));
		XPiece OPPKING(opp | (1<<7));

		for (auto p : getAllPieces(you | (1<<2))) {
			int dx[4] = {01, 00, -1, 00};
			int dy[4] = {00, 01, 00, -1};
			for (int i = 0; i < 4; i++) {
				if (legal(p.pos(), {dx[i], dy[i]})) res.push_back({p.pos(), {dx[i], dy[i]}});
			}
		}

		for (auto p : getAllPieces(you | (1<<3))) {
			int dx[8] = {02, 01, -1, -2, -2, -1, 01, 02};
			int dy[8] = {01, 02, 02, 01, -1, -2, -2, -1};
			for (int i = 0; i < 8; i++) {
				if (legal(p.pos(), {dx[i], dy[i]})) res.push_back({p.pos(), {dx[i], dy[i]}});
			}
		}

		for (auto p : getAllPieces(you | (1<<4))) {
			int dx[4] = {02, 02, -2, -2};
		int dy[4] = {02, -2, 02, -2};
			for (int i = 0; i < 4; i++) {
				if (legal(p.pos(), {dx[i], dy[i]})) res.push_back({p.pos(), {dx[i], dy[i]}});
			}
		}

		for (auto p : getAllPieces(you | (1<<5))) {
			int dx[4] = {01, 00, -1, 00};
		int dy[4] = {00, -1, 00, -1};
			for (int i = 0; i < 4; i++) {
				for (int d = 1; d < 12; d++) {
					if (legal(p.pos(), {dx[i] * d, dy[i] * d})) res.push_back({p.pos(), {dx[i] * d, dy[i] * d}});
				}
			}
		}

		for (auto p : getAllPieces(you | (1<<6))) {
			int dx[4] = {01, 01, -1, -1};
		int dy[4] = {01, -1, 01, -1};
			for (int i = 0; i < 4; i++) {
				if (legal(p.pos(), {dx[i], dy[i]})) res.push_back({p.pos(), {dx[i], dy[i]}});
			}
		}

		for (auto p : getAllPieces(you | (1<<7))) {
			int dx[4] = {01, 00, -1, 00};
			int dy[4] = {00, -1, 00, -1};
			for (int i = 0; i < 4; i++) {
				if (legal(p.pos(), {dx[i], dy[i]})) res.push_back({p.pos(), {dx[i], dy[i]}});
			}
		}

		for (auto p : getAllPieces(you | (1<<8))) {
			int dx[4] = {01, 00, -1, 00};
			int dy[4] = {00, -1, 00, -1};
			for (int i = 0; i < 4; i++) {
				for (int d = 1; d < 12; d++) {
					if (legal(p.pos(), {dx[i] * d, dy[i] * d})) res.push_back({p.pos(), {dx[i] * d, dy[i] * d}});
				}
			}
		}
		// std::cout << res.size() << "\n";
		return res;
	}

	std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> getDefenses() {
		std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> res;

		int you = (sidetomove) ? (1<<0) : (1<<1);
        int opp = (sidetomove) ? (1<<1) : (1<<0);

		XPiece THISKING(you | (1<<7));
		XPiece OPPKING(opp | (1<<7));

		// Pawns
		for (auto p : getAllPieces(opp | (1<<2))) {
			int dx[3] = {01, 00, -1};
			int dy[3] = {00, 01, 00};

			for (int i = 0; i < 3; i++) {
				int x = p.file() + dx[i];
				int y = p.rank() + dy[i];
				if (!inBounds(x, y)) continue;
				XPiece thing = get(x, y);
				std::pair<int, int> src = p.pos();
				std::pair<int, int> des = {x, y};
				if (!thing.isEmpty() && thing.getColor() == sidetomove) res.push_back({src, des});
			}
		}

		// Knights
		for (auto p : getAllPieces(opp | (1<<3))) {
			int dx[8] = {02, 01, -1, -2, -2, -1, 01, 02};
            int dy[8] = {01, 02, 02, 01, -1, -2, -2, -1};
			int bx[8] = {01, 00, 00, -1, -1, 00, 00, 01};
			int by[8] = {00, 01, 01, 00, 00, -1, -1, 00};

			for (int i = 0; i < 8; i++) {
				if (!inBounds({p.file() + dx[i], p.rank() + dy[i]})) continue;
				if (!inBounds({p.file() + bx[i], p.rank() + by[i]})) continue;
				if (!get(p.file() + bx[i], p.rank() + by[i]).isEmpty()) continue;
				XPiece thing = get(p.file() + dx[i], p.rank() + dy[i]);
				std::pair<int, int> src = p.pos();
				std::pair<int, int> des = {p.file() + dx[i], p.rank() + dy[i]};
				if (!thing.isEmpty() && thing.getColor() == sidetomove) res.push_back({src, des});
			}
		}

		// Rooks

		for (auto p : getAllPieces(opp | (1<<5))) { // Rooks
            int dx[4] = {00, 01, 00, -1};
            int dy[4] = {01, 00, -1, 00};
            for (int i = 0; i < 4; i++) {
                for (int k = 1; k < 12; k++) {
                    if (!inBounds(p.file() + dx[i] * k, p.rank() + dy[i] * k)) break;
                    XPiece thing = get(p.file() + dx[i], p.rank() + dy[i]);
					std::pair<int, int> src = p.pos();
					std::pair<int, int> des = {p.file() + dx[i], p.rank() + dy[i]};
					if (!thing.isEmpty() && thing.getColor() == sidetomove) res.push_back({src, des});

                    if (!board[p.file() + dx[i] * k][p.rank() + dy[i] * k].isEmpty()) break;
                }
            }
        }

		// Elephants
		for (auto p : getAllPieces(opp | (1<<4))) {
			int dx[4] = {02, 02, -2, -2};
			int dy[4] = {02, -2, 02, -2};
			int bx[4] = {01, 01, -1, -1};
			int by[4] = {01, -1, 01, -1};

			for (int i = 0; i < 4; i++) {
				if (!inBounds({p.file() + dx[i], p.rank() + dy[i]})) continue;
				if (!inBounds({p.file() + bx[i], p.rank() + by[i]})) continue;
				XPiece thing = get(p.file() + dx[i], p.rank() + dy[i]);
				std::pair<int, int> src = p.pos();
				std::pair<int, int> des = {p.file() + dx[i], p.rank() + dy[i]};
				if (!isPoliticallyCorrect(des)) continue;
				if (!thing.isEmpty() && thing.getColor() == sidetomove) res.push_back({src, des});
			}
		}	

		// Advisors
		for (auto p : getAllPieces(opp | (1<<6))) {
			int dx[4] = {01, 01, -1, -1};
			int dy[4] = {01, -1, 01, -1};

			for (int i = 0; i < 4; i++) {
				if (!inBounds({p.file() + dx[i], p.rank() + dy[i]})) continue;
				XPiece thing = get(p.file() + dx[i], p.rank() + dy[i]);
				std::pair<int, int> src = p.pos();
				std::pair<int, int> des = {p.file() + dx[i], p.rank() + dy[i]};
				if (!isInPalace(des)) continue;
				if (!thing.isEmpty() && thing.getColor() == sidetomove) res.push_back({src, des});
			}
		}

		// Kings
		for (auto p : getAllPieces(opp | (1<<7))) {
			int dx[4] = {00, 01, 00, -1};
			int dy[4] = {01, 00, 01, 00};

			for (int i = 0; i < 4; i++) {
				if (!inBounds({p.file() + dx[i], p.rank() + dy[i]})) continue;
				XPiece thing = get(p.file() + dx[i], p.rank() + dy[i]);
				std::pair<int, int> src = p.pos();
				std::pair<int, int> des = {p.file() + dx[i], p.rank() + dy[i]};
				if (!isInPalace(des)) continue;
				if (!thing.isEmpty() && thing.getColor() == sidetomove) res.push_back({src, des});
			}
		}

		// Cannons
		for (auto p : getAllPieces(opp | (1<<8))) {
			int dx[4] = {00, 01, 00, -1};
            int dy[4] = {01, 00, -1, 00};
            for (int i = 0; i < 4; i++) {
				int ihateyourguts = 0;
                for (int k = 1; k < 12; k++) {
                    if (!inBounds(p.file() + dx[i] * k, p.rank() + dy[i] * k)) break;
                    XPiece thing = get(p.file() + dx[i], p.rank() + dy[i]);
					std::pair<int, int> src = p.pos();
					std::pair<int, int> des = {p.file() + dx[i], p.rank() + dy[i]};
					if (!thing.isEmpty() && thing.getColor() == sidetomove && ihateyourguts == 1) res.push_back({src, des});

                    if (!board[p.file() + dx[i] * k][p.rank() + dy[i] * k].isEmpty()) ihateyourguts++;
					if (ihateyourguts > 1) break;
                }
            }
		}

		return res;
	}

	bool checkmate() { return getAllLegalMoves().size() == 0 && !noChecks(); }
    bool TLE() { return halfmoveclock >= maxmoves; }
    bool stalemate() { return TLE() || (getAllLegalMoves().size() == 0 && noChecks()); }
    bool gameover() { return checkmate() || stalemate(); }
};

#endif