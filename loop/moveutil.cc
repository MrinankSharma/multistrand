/*
Copyright (c) 2017 California Institute of Technology. All rights reserved.
Multistrand nucleic acid kinetic simulator
help@multistrand.org
*/

#include <moveutil.h>
#include <sequtil.h>
#include <assert.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <scomplex.h>

using std::vector;
using std::map;
using std::cout;
using std::stringstream;


std::string quartContextString[HALFCONTEXT_SIZE] = { "end", "loop", "stack" };

int moveutil::typeMult(MoveType left, MoveType right) {

	return (int) (valuesPrime[left] * valuesPrime[right]);

}

//moveutil::useArr = energyModel.

QuartContext moveutil::getContext(char input) {

	if (input > 0) { // there is a stack on the exteriour

		return stackC;

	} else {

		return endC;

	}

}

int moveutil::getPrimeCode(MoveType left, MoveType right) {

	return valuesPrime[left] * valuesPrime[right];

}

string moveutil::primeToDesc(int input) {

	stringstream ss;

	for (int i = 0; i < MOVETYPE_SIZE; i++) {

		int myPrime = moveutil::valuesPrime[i];

		if ((input % myPrime) == 0) {

			ss << MoveToString[i] << moveutil::MoveToString2[i];

			if ((input % (myPrime * myPrime) == 0)) {

				ss << MoveToString[i] << moveutil::MoveToString2[i];

			}

		}

	}

	return ss.str();

}

std::ostream& operator<<(std::ostream &ss, OpenInfo& m) {

	for (std::pair<HalfContext, BaseCount> value : m.tally) {

		ss << value.first << " ";
		ss << value.second << "   --   ";

	}

	ss << "Intern / Total = " << m.numExposedInternal << " / ";
	ss << m.numExposed << "	\n";

	ss << "\n";

	return ss;

}

void OpenInfo::clear(void) {

	tally.clear();
	numExposedInternal = 0;
	numExposed = 0;

}

// simply store the vector of halfContext onto the list we already have
void OpenInfo::increment(QuartContext left, char base, QuartContext right) {

	HalfContext con = HalfContext(left, right);

	if (tally.count(con)) { // if count > 0 then proceed

		tally.find(con)->second.count[base]++;

	} else {

		BaseCount count = BaseCount();
		count.count[base]++;
		tally.insert(std::pair<HalfContext, BaseCount>(con, count));

	}

}

void OpenInfo::increment(HalfContext con, BaseCount& count) {

	if (tally.count(con)) {

		// if found, increment from the reference

		tally.find(con)->second.increment(count);

	} else { // if not found, don't store the reference, make a new object.

		BaseCount countNew = BaseCount();
		countNew.increment(count);

		tally.insert(std::pair<HalfContext, BaseCount>(con, countNew));

	}

}

void OpenInfo::decrement(HalfContext con, BaseCount& count) {

	assert(tally.count(con));

	tally.find(con)->second.decrement(count);

}

void OpenInfo::increment(OpenInfo& other) {

	map<HalfContext, BaseCount> tally;

	for (std::pair<HalfContext, BaseCount> myPair : other.tally) {

		increment(myPair.first, myPair.second);

	}

	numExposedInternal += other.numExposedInternal;
	numExposed += other.numExposed;

}

void OpenInfo::decrement(OpenInfo& other) {

	map<HalfContext, BaseCount> tally;

	for (std::pair<HalfContext, BaseCount> myPair : other.tally) {

		decrement(myPair.first, myPair.second);

	}

	numExposedInternal -= other.numExposedInternal;
	numExposed -= other.numExposed;

}

// simply compute the crossed-rate between these exposed nucleotides.

double OpenInfo::crossRate(OpenInfo& other, EnergyModel& eModel) {

	double output = 0.0;

	for (std::pair<HalfContext, BaseCount> here : tally) {

		HalfContext& top = here.first;
		BaseCount& countTop = here.second;

		for (std::pair<HalfContext, BaseCount> there : other.tally) {

			HalfContext& bot = there.first;
			BaseCount& countBot = there.second;

			int crossings = countTop.multiCount(countBot);

			if (crossings > 0) {

				MoveType left = moveutil::combineBi(top.left, bot.right);
				MoveType right = moveutil::combineBi(top.right, bot.left);


				double joinRate = eModel.applyPrefactors(eModel.getJoinRate(), left, right);

				double rate = crossings * joinRate;

				output += rate;

			}

		}

	}

	return output;

}

JoinCriteria::JoinCriteria() {

// empty constructor

}

std::ostream & operator<<(std::ostream & ss, JoinCriteria & m) {

	ss << " criteria \n";

	ss << "Types = " << (int) m.types[0] << " " << (int) m.types[1] << "\n";
	ss << "Index = " << m.index[0] << " " << m.index[1] << " \n";
	ss << "Half = " << m.half[0] << " " << m.half[1] << "\n";
	ss << "Complex0 = " << m.complexes[0]->ordering->toString() << "\n";
	ss << "Complex1 = " << m.complexes[1]->ordering->toString() << "\n";

	return ss;

}

MoveType moveutil::combineBi(QuartContext & one, QuartContext & two) {

// c++ doesn't do double variable switch
// FD: This switch should ONLY be used for bimolecular rates.

	if (one == endC) {

		if (two == endC) {

			return endMove;

		}

		if (two == strandC) {

			return loopEndMove;

		}

		if (two == stackC) {

			return stackEndMove;

		}

	}

	if (one == strandC) {

		if (two == endC) {

			return loopEndMove;

		}

		if (two == strandC) {

			return loopMove;

		}

		if (two == stackC) {

			return stackLoopMove;

		}

	}

	if (one == stackC) {

		if (two == endC) {

			return stackEndMove;

		}

		if (two == strandC) {

			return stackLoopMove;

		}

		if (two == stackC) {

			return stackStackMove;

		}

	}

	cout << "Failure to recongize quarter context in external nucleotides";
	assert(0);

	return MOVETYPE_SIZE;

}

bool moveutil::isPair(BaseType one, BaseType two) {

	return (one + two == 5);

}

// constructor assigns the base
HalfContext::HalfContext() {

}

HalfContext::HalfContext(QuartContext in1, QuartContext in2) {

	left = in1;
	right = in2;

}

std::ostream & operator<<(std::ostream & os, HalfContext & m) {

	os << "(" << quartContextString[m.left] << ", ";
	os << quartContextString[m.right] << ") ";

	return os;

}

bool HalfContext::operator==(const HalfContext& other) const {

	return (left == other.left) && (right == other.right);

}

// c++ expects partial ordering instead of equality for mapping
bool HalfContext::operator<(const HalfContext& other) const {

	return (left < other.left) || (left == other.left && right < other.right);

}

