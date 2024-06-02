#include <iostream>
#include "xiangqi.h"
#include "genetic.h"

void scoretable() {
	int black = 0;
	int white = 0;
	int x;
	while (std::cin >> x) {
	    if (x == -1000) break;
		if (x == -1) black++;
		if (x == 1) white++;
	}
	std::cout << "WHITE " << white << " BLACK " << black << std::endl;
}

// Example thing to run tournaments on engines. This instance runs one trained model on randomly generated models.

int main() {
    srand(time(0));

	XAI res(1.465682, 0.377270, 1.772698, 0.043397, 0.140934, -1.568957, 0.009095, -0.374828, 0.744469, 1000.000000, 0.062204); // Reigning champion
	XAI res1(0.475478, 1.896725, 0.368725, 1.433821, -1.249733, 0.028993, 0.899625, -0.425733, 1.093600, 1000.000000, -0.064280); // Challenger
    int wb = 0;
    int bb = 0;
    int dw = 0;
	int db = 0;
	int dr = 0;

	for (int i = 0; i < 16; i++) {
        int val = Genetic::test(res1, res, true);
        if (val == 2) wb++;
        else if (val == -2) bb++;
        else if (val == 1) dw++;
		else if (val == -1) db++;
		else dr++;
        std::cout << val << " ";
    }
	std::cout << "\n";
	std::cout << wb << " " << bb << " | " << dw << " " << db << " | " << dr << "\n";

	wb = 0;
	bb = 0;
	dw = 0;
	db = 0;
	dr = 0;

	for (int i = 0; i < 16; i++) {
        int val = Genetic::test(res, res1, false);
        if (val == 2) wb++;
        else if (val == -2) bb++;
        else if (val == 1) dw++;
		else if (val == -1) db++;
		else dr++;
        std::cout << val << " ";
    }
	std::cout << "\n";
	std::cout << wb << " " << bb << " | " << dw << " " << db << " | " << dr << "\n";

	return 0;
}

/*

v4 (fixed side and palace calculations)

PP 3.465316 MOB 1.593005 BN -0.517655 RDEF -1.792474 CDEF 1.862300 QDEF -0.779748 KMOB 0.610431 KDEF 0.198187 CHK 1.378033 CKMT 1000.000000 MCNT 0.017266

v3 (fixed cannon movement)

PP 0.475478 MOB 1.896725 BN 0.368725 RDEF 1.433821 CDEF -1.249733 QDEF 0.028993 KMOB 0.899625 KDEF -0.425733 CHK 1.093600 CKMT 1000.000000 MCNT -0.064280

v2 (promotedpawn is [-4, 4])

(32x16 - 1/r)
PP 2.635701 MOB 1.710440 BN -0.541581 RDEF 1.401593 CDEF -1.638905 QDEF -0.199896 KMOB -0.000671 KDEF 0.203803 CHK 1.336894 CKMT 1000.000000 MCNT 0.229415

v1 (initial release)

(32x16 - 1/r)
PP 1.465682 MOB 0.377270 BN 1.772698 RDEF 0.043397 CDEF 0.140934 QDEF -1.568957 KMOB 0.009095 KDEF -0.374828 CHK 0.744469 CKMT 1000.000000 MCNT 0.062204

(32x16 - 3/r)
PP 1.364483 MOB 1.830439 BN 1.105319 RDEF 1.419172 CDEF 0.406201 QDEF -0.060121 KMOB -1.232032 KDEF 1.258400 CHK -0.020081 CKMT 1000.000000 MCNT 0.143750

*/