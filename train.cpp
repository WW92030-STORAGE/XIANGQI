#include "genetic.h"
#include "xiangqi.h"

#include <iostream>

// Train a chess bot using artificial selection.

const int ROUNDS = 1;

int main() {
    srand(time(0));
    
    std::vector<XAI> v;
    for (int i = 0; i < 32; i++) {
        XAI ai = Genetic::randomAI();
        v.push_back(ai);
		// std::cout << ai.toString() << "\n";
        std::cout << "X";
    }
    std::cout << "\n";
    
    for (int i = 0; i < 16; i++) {
        std::cout << "GEN " << (i + 1) << "\n";
        std::vector<XAI> res = Genetic::tournament(v, true, ROUNDS);
        
        for (auto i : res) std::cout << i.toString() << std::endl;
        
        v.clear();
        for (int round = 0; round < 4; round++) {
            std::random_shuffle(res.begin(), res.end());
            for (int i = 0; i < res.size() - 1; i += 2) v.push_back(Genetic::mutate(Genetic::cross(res[i], res[i + 1])));
        }
    }
    
    // Reduction
    std::vector<XAI> res;
    while (true) {
        res = Genetic::tournament(v, true, ROUNDS);
        if (res.size() <= 1) break;
    
        v.clear();
		for (int ii = 0; ii < 2; ii++) {
			std::random_shuffle(res.begin(), res.end());
        	for (int i = 0; i < res.size() - 1; i += 2) {
            	v.push_back(Genetic::mutate(Genetic::cross(res[i], res[i + 1])));
        	}
		}
    }
    
    std::cout << "FINAL MODEL " << res[0].toString() << "\n";

	std::cout << "PLAYING AS WHITE\n";
    
    for (int i = 0; i < 32; i++) std::cout << Genetic::test(res[0], Genetic::randomAI(), false) << " ";
	std::cout << "\n";

	std::cout << "PLAYING AS BLACK\n";
    for (int i = 0; i < 32; i++) std::cout << Genetic::test(XAI(), Genetic::randomAI(), false) << " ";
}

/*



*/