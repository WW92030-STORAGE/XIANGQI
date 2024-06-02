#include "xiangqi.h"
#include "genetic.h"

#include <iostream>

int main()
{
    XGame game;
    std::cout << game.toString();

	int x, y, ex, ey;
	while (std::cin >> x >> y >> ex >> ey) {
		std::pair<int, int> src = {x, y};
		std::pair<int, int> des = {ex, ey};
		std::pair<int, int> vec = {ex - x, ey - y};
		if (!game.legal(src, vec, true)) {
			std::cout << "INVALID MOVE\n";
			continue;
		}

		game.execute(src, vec);
		std::cout << game.toString() << "\n";
		game.sidetomove = !game.sidetomove;

		for (auto i : game.getAllLegalMoves()) std::cout << i.first.first << " " << i.first.second << " > " << i.second.first << " " << i.second.second << "\n";
	}

    return 0;
}
