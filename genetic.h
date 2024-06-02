#ifndef GENETIC_H
#define GENETIC_H

#include "xiangqi.h"

#include <set>
#include <vector>
#include <algorithm>
#include <map>
#include <cfloat>
#include <climits>
#include <string>
#include <cmath>

class XAI {
	public:
	// . . . . . . . .  X  X  P  N  B  R  Q  K  C
	double values[9] = {0, 0, 1, 4, 2, 9, 2, 1000, 4.5};
	double promotedpawn = 2; // value of a pawn that crosses the center
	double mob = 1; // mobility of non-king and non-pawns
	double bndef = 1; // BN defense counter
    double rdef = 1; // rook defense counter
	double cdef = 1; // cannon defense counter
	double qdef = 1; // advisor defense counter
    double kmob = 1; // King mobility excludes castles
    double kdef = -1; // If the king is undefended we deduct some points.
    double chk = 1; // Checks (don't worry about checkmates for now)
    double ckmt = 1000; // Checkmate value that replaces the check value upon the threat of a mate
	double movecount = -0.01;

	XAI() {
		promotedpawn = 2;
		mob = 1;
		bndef = 1;
		rdef = 1;
		cdef = 1;
		qdef = 1;
		kmob = 1;
		kdef = -1;
		chk = 1;
		ckmt = 1000;
		movecount = -0.01;
	}

	XAI(double pp, double mo, double bn, double rd, double cd, double qd, double km, double kd, double ch, double ck, double mv) {
		promotedpawn = pp;
		mob = mo;
		bndef = bn;
		rdef = rd;
		cdef = cd;
		qdef = qd;
		kmob = km;
		kdef = kd;
		chk = ch;
		ckmt = ck;
		movecount = mv;
	}

	XAI(const XAI& other) {
		promotedpawn = other.promotedpawn;
		mob = other.mob;
		bndef = other.bndef;
		rdef = other.rdef;
		cdef = other.cdef;
		qdef = other.qdef;
		kmob = other.kmob;
		kdef = other.kdef;
		chk = other.chk;
		ckmt = other.ckmt;
		movecount = other.movecount;
	}

	double getOneSidedScore(XGame game, bool verbose = false) {
		double material = 0;
		for (int x = 0; x < 9; x++) {
			for (int y = 0; y < 10; y++) {
				XPiece piece = game.get(x, y);
				if (!piece.isEmpty() && piece.getColor() == game.sidetomove) {
					if (piece.isPawn()) {
						if (game.isPoliticallyCorrect(x, y)) material += values[piece.getID()];
						else material += promotedpawn;
					}
					else material += values[piece.getID()];
				}
			}
		}

		std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> legals = game.getAllLegalMoves();
        std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> defs = game.getDefenses();

		double mobs = 0;
		std::map<std::pair<int, int>, int> pmoves;
		for (auto p : legals) {
			XPiece piece = game.get(p.first);
			if (piece.isEmpty() || piece.isPawn() || piece.isKing()) continue;
			if (pmoves.find(p.first) == pmoves.end()) pmoves.insert({p.first, 0});
            (*(pmoves.find(p.first))).second++;
		}

		int kmobs = 0;
        for (auto p : legals) {
            XPiece source = game.get(p.first);
            if (source.isKing() && abs(p.second.second) != 2) kmobs++;
        }

		for (auto p : pmoves) mobs += std::sqrt((double)(p.second));
        
        int bndefs = 0;
        int rdefs = 0;
		int cdefs = 0;
		int qdefs = 0;
        int kdefcnt = 0;
        for (auto p : defs) {
            XPiece source = game.get(p.second);
            if (source.isBishop() || source.isKnight()) bndefs++;
            if (source.isRook()) rdefs++;
			if (source.isCannon()) cdefs++;
			if (source.isAdvisor()) qdefs++;
            if (source.isKing()) kdefcnt++;
        }

		int kdefs = 0;
        
        int you = (game.sidetomove) ? (1<<0) : (1<<1);
        
        if (kdefcnt == 0) {
            for (auto p : game.getAllPieces(you | (1<<7))) {
                int dx[8] = {00, 01, 01, 01, 00, -1, -1, -1};
                int dy[8] = {01, 01, 00, -1, -1, -1, 00, 01};
                for (int i = 0; i < 8; i++) {
                    for (int k = 1; k < 12; k++) {
                        std::pair<int, int> des = {p.file() + dx[i] * k, p.rank() + dy[i] * k};
                        if (!game.inBounds(des)) break;
                        XPiece x = game.board[des.first][des.second];
                        if (!x.isEmpty()) break;
                        kdefs++;
                    }
                }
            }
        }

		XGame game2(game);
		game2.sidetomove = !game.sidetomove;
		int checks = (game2.noChecks()) ? 0 : 1;
		if (game2.checkmate()) checks = ckmt;

		int movecnt = game.halfmoveclock;

		return material + mobs * mob + kmobs * kmob + bndefs * bndef + rdefs * rdef + cdefs * cdef + qdefs * qdef + kdefs * kdef + checks * chk - movecnt * movecount;
	}

	double getScore(XGame game, bool verbose = false) {
		double res = getOneSidedScore(game, verbose);
		XGame game2(game);
		game2.sidetomove = !game.sidetomove;
		return res - getOneSidedScore(game2, verbose);
	}

	std::pair<std::pair<int, int>, std::pair<int, int>> chosenmove = {{0, 0}, {0, 0}};

    int leafcount = 0;

    double abprune(XGame game, int remlayers, double alpha, double beta, bool isMaximizing) { // remlayers must start (outermost call) at an even number
        if (remlayers <= 0) {
            leafcount++;
            return getScore(game);
        }

        if (isMaximizing) {
            double res = -1 * DBL_MAX;
            std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> legals = game.getAllLegalMoves();
            std::random_shuffle(legals.begin(), legals.end());
            for (auto p : legals) {
                XGame game2(game);
                game2.execute(p.first, p.second);

                game2.sidetomove = !game2.sidetomove;
                double value = abprune(game2, remlayers - 1, alpha, beta, false);
                if (value > res) {
                    chosenmove = p;
                    res = value;
                }
                if (value == res && rand() % 2 == 0) {
                    chosenmove = p;
                    res = value;
                }
                alpha = std::max(alpha, res);
                if (beta <= alpha) break;
            }

            return res;
            
        }

        else {
            double res = DBL_MAX;
            std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> legals = game.getAllLegalMoves();
            std::random_shuffle(legals.begin(), legals.end());
            for (auto p : legals) {
                XGame game2(game);
                game2.execute(p.first, p.second);
                game2.sidetomove = !game2.sidetomove;

                double value = abprune(game2, remlayers - 1, alpha, beta, true);
                if (value < res) {
                    res = value;
                    // chosenmove = p;
                }
                if (value == res && rand() % 2 == 0) {
                    // chosenmove = p;
                    res = value;
                }
                beta = std::min(beta, res);
                if (beta <= alpha) break;
            }
            return res;
        }
        return -1;


    }

	std::pair<std::pair<int, int>, std::pair<int, int>> pick(XGame game, bool verbose = false) {
	    // return pickdepth2(game, false);

        leafcount = 0;
        chosenmove = game.getAllLegalMoves()[0];
        abprune(game, 2, -1 * DBL_MAX, DBL_MAX, true);
        if (verbose) std::cout << leafcount << " LEAF NODES CHECKED\n";
        return chosenmove;
	}
    
    // mob / bndef / rdef / cdef / qdef / kmob / kdef / chk / ckmt / movecount

	std::string toString() {
        std::string res = "PP " + std::to_string(promotedpawn) + " MOB " + std::to_string(mob) + " BN " + std::to_string(bndef) + " RDEF " + std::to_string(rdef) + " CDEF " + std::to_string(cdef);
        res = res + " QDEF " + std::to_string(qdef) + " KMOB " + std::to_string(kmob) + " KDEF " + std::to_string(kdef);
        res = res + " CHK " + std::to_string(chk) + " CKMT " + std::to_string(ckmt) + " MCNT " + std::to_string(movecount);
        return res;
    }
};

namespace Genetic {
    
// Play with a1 white and a2 black
int test(XAI a1, XAI a2, bool verbose = false, int games = 1) {
	int res = 0;
	for (int i = 0; i < games; i++) {
    XGame game;
    
    while (true) { // a1 white a2 black
        auto move = game.sidetomove ? (a1.pick(game)) : (a2.pick(game));
        game.execute(move.first, move.second);
        game.sidetomove = !game.sidetomove;
        
        if (verbose) std::cout << game.toString() << "\n";
            
        if (game.checkmate()) {
            if (verbose) std::cout << game.toString() << "\n";
            if (verbose) std::cout << ( game.sidetomove ? "BLACK" : "WHITE" ) << " WINS\n";
            res += game.sidetomove ? (-2) : (2);
			break;
        }
        
        if (game.stalemate()) {
            if (verbose) std::cout << game.toString() << "\n";
            if (verbose) std::cout << "STALEMATE/DRAW\n";
            res += game.sidetomove ? (-1) : 1;
			break;
        }

		// std::cout << game.toString() << "\n";
    }

	}

	return res;
}

std::vector<XAI> tournament(std::vector<XAI> ais, bool verbose = false, int gamesperround = 1) {
    std::random_shuffle(ais.begin(), ais.end());
    std::vector<XAI> res;
    for (int i = 0; i < ais.size() - 1; i += 2) {
        int val = test(ais[i], ais[i + 1], false, gamesperround);
        if (val > 0) res.push_back(XAI(ais[i]));
        else if (val < 0) res.push_back(XAI(ais[i + 1]));
        else {
            if (rand() % 2 == 0) res.push_back(XAI(ais[i]));
            else res.push_back(XAI(ais[i + 1]));
        }
        if (verbose) std::cout << "X";
    }
    if (verbose) std::cout << "\n";
    return res;
}

// mob / rbndef / qdef / kmob / kdef / oo / chk / ckmt / movecount

XAI cross(XAI a1, XAI a2) {
    XAI res(a1);
	if (rand() % 2 == 0) res.promotedpawn = a2.promotedpawn;
    if (rand() % 2 == 0) res.mob = a2.mob;
    if (rand() % 2 == 0) res.bndef = a2.bndef;
	if (rand() % 2 == 0) res.rdef = a2.rdef;
	if (rand() % 2 == 0) res.cdef = a2.cdef;
    if (rand() % 2 == 0) res.qdef = a2.qdef;
    if (rand() % 2 == 0) res.kmob = a2.kmob;
    if (rand() % 2 == 0) res.kdef = a2.kdef;
    if (rand() % 2 == 0) res.chk = a2.chk;
    if (rand() % 2 == 0) res.ckmt = a2.ckmt;
    if (rand() % 2 == 0) res.movecount = a2.movecount;
    return res;
}

double randf() {
    return (double)(rand()) / RAND_MAX;
}

// mob / bndef / rdef / qdef / cdef / kmob / kdef / oo / chk / ckmt / movecount

XAI mutate(XAI ai) {
    XAI res(ai);
    int beep = rand() % 128;
	while (beep > 96) beep = rand() % 128;
    if (beep == 0) res.mob = randf() * 4 - 2;
    if (beep == 1) res.bndef = randf() * 4 - 2;
    if (beep == 2) res.qdef = randf() * 4 - 2;
	if (beep == 3) res.rdef = randf() * 4 - 2;
	if (beep == 4) res.cdef = randf() * 4 - 2;
    if (beep == 5) res.kmob = randf() * 4 - 2;
    if (beep == 6) res.kdef = randf() * -4 + 2;
    if (beep == 7) res.chk = randf() * 4 - 2;
    // if (beep == 7) res.ckmt = randf() * 400;
    if (beep == 8) res.movecount = (0.5 - randf()) * 0.5;
	if (beep == 9) res.promotedpawn = randf() * 8 - 4;
    return res;
}

XAI randomAI() {
    XAI res;
	res.promotedpawn = randf() * 8 - 4;
    res.mob = randf() * 4 - 2;
    res.bndef = randf() * 4 - 2;
	res.rdef = randf() * 4 - 2;
	res.cdef = randf() * 4 - 2;
    res.qdef = randf() * 4 - 2;
    res.kmob = randf() * 4 - 2;
    res.kdef = randf() * -4 + 2;
    res.chk = randf() * 4 - 2;
    // if (beep == 7) res.ckmt = randf() * 400;
    res.movecount = (0.5 - randf()) * 0.5;
    return res;
}

}

#endif