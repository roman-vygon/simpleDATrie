#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <set>

using namespace std;
const int LEAF_BASE_VALUE = -2;
const int ROOT_CHECK_VALUE = -3;
const int EMPTY_VALUE = -1;
const int INITIAL_ROOT_BASE = 1;

class DoubleArrayTrie {

	// The leaf base value
	int alphabetLength;
	
	// The base array.
	vector<int> base;
	
	// The check array.	
	vector<int> check;

	// The free positions, for quick access
	set<int> freePositions;
	int getBase(int position) {
		return base[position];
	}
		
	int getCheck(int position) {
		return check[position];
	}
		
	void setBase(int position, int value) {
		base[position] = value;
		if (value == EMPTY_VALUE) {
			freePositions.insert(position);
		}
		else {
			freePositions.erase(position);
		}
	}

	void setCheck(int position, int value) {
		check[position] = value;
		if (value == EMPTY_VALUE) {
			freePositions.insert(position);
		}
		else {
			freePositions.erase(position);
		}
	}

	int getSize() {
		return base.size();
	}

	

	void updateChildMove(int parentIndex, int forCharacter,
			int newParentBase) {
		assert(getCheck(getBase(parentIndex) + forCharacter) == parentIndex);
	}

	void init() {
		base.clear();
		check.clear();
		// The original offset, everything non-root starts at base(1)
		base.push_back(INITIAL_ROOT_BASE);
		// The root check has no meaning, thus a special value is needed.
		check.push_back(ROOT_CHECK_VALUE);
		freePositions.clear();
	}
	
	public:
		DoubleArrayTrie(int _alphabetLength) {
			/**
			* Constructs a DoubleArrayTrie for the given alphabet length.
			*
			* @param alphabetLength The size of the set of values that
			* 				are to be stored.
			*/
			alphabetLength = _alphabetLength;
			init();
		}
	int nextAvailableHop(int forValue) {

		int value = forValue;
		/*
		 * First we make sure that there exists a free location that is
		 * strictly greater than the value.
		 */
		while (true) {
			if (freePositions.upper_bound(value) != freePositions.end())
			{
				if (*freePositions.upper_bound(value) <= value)
					ensureReachableIndex(base.size() + 1); // This adds to the freePositions store
				else {
					break;
				}
			}
			else
			{
				ensureReachableIndex(base.size() + 1);
			}
			
		}
		/*
		 * From the termination condition of the loop above, the next line
		 * CANNOT throw NullPointerException
		 * Note that we return the position minus the value. That is because
		 * the result is the ordinal of the new state which is translated
		 * to a store index. Therefore, since we add the value to the base
		 * to find the next state, here we must subtract.
		 */
		int result = *freePositions.upper_bound(value) - forValue;
		// This assertion must pass thanks to the loop above
		assert(result >= 0);
		return result;
	}
	void ensureReachableIndex(int limit) {
		while (getSize() <= limit) {
			/*
			 * In essence, we let all enlargement operations to the implementing
			 * class of the backing store. Since this currently is a ArrayList,
			 * simply adding values until we are done will work.
			 */
			base.push_back(EMPTY_VALUE);
			check.push_back(EMPTY_VALUE);
			// All new positions are free by default.
			freePositions.insert(base.size() - 1);
		}
	}
	int findConsecutiveFree(int amount) {

		assert(amount >= 0);
		/*
		 * Quick way out, that also ensures the invariants
		 * of the main loop.
		 */
		if (!freePositions.size()) {
			return -1;
		}

		set<int>::iterator it = freePositions.begin();
		int from; 		// The location from where the positions begin
		int current;	// The next integer in the set
		int previous;	// The previously checked index
		int consecutive;	// How many consecutive positions have we seen so far 

		from = *it;	// Guaranteed to succeed, from the if at the start
		previous = from;	// The first previous is the first in the series
		consecutive = 1;	// 1, since from is a valid location
		while (consecutive < amount && ++it!=freePositions.end()) {
			current = *it;
			if (current - previous == 1) {
				previous = current;
				consecutive++;
			}
			else {
				from = current;
				previous = from;
				consecutive = 1;
			}
		}
		if (consecutive == amount) {
			return from;
		}
		else {
			return -1;
		}
	}

	int nextAvailableMove(set<int> values) {
		// In the case of a single child, the problem is solved.
		if (values.size() == 1) {
			return nextAvailableHop(*values.begin());
		}

		int minValue = *values.begin();
		int maxValue = *values.rbegin();
		int neededPositions = maxValue - minValue + 1;

		int possible = findConsecutiveFree(neededPositions);
		if (possible - minValue >= 0) {
			return possible - minValue;
		}

		ensureReachableIndex(base.size() + neededPositions);
		return base.size() - neededPositions - minValue;
	}

	/**
	 * Adds this string to the trie.
	 *
	 * @param string The string to add
	 */
	bool addToTrie(vector<int> string) {
		bool changed = false;
		// Start from the root
		int state = 0;		// The current DFA state ordinal
		int transition = 0;	// The candidate for the transition end state
		int i = 0;			// The input string index
		int c = 0;			// The current input string character
		// For every input character
		while (i < string.size()) {
			assert(state >= 0);
			//assert getBase(state) >= 0;
			//c = string.get(i);
			//FZ: the following asserttion will fire in case we inserted "a" and 
			//    then "ab" since inserting "a" will result in base('a') to be set to
			//    LEAF_BASE_VALUE which is -1
			//assert getBase(state) >= 0;
			//FZ: thus the following fix. 
			c = string[i];
			int stateBase = getBase(state);

			if (i > 0 && stateBase == LEAF_BASE_VALUE) {
				setBase(transition, nextAvailableHop(c)); // Add a state
				changed = true;
			}
			else {
				assert(getBase(state) >= 0);
			}
			//FZ: end of fix. 
			// Calculate next hop. It is the base contents of the current state
			// plus the input character.
			transition = getBase(state) + c;
			assert(transition > 0);
			ensureReachableIndex(transition);
			/*
			 * If the next hop index is empty
			 * (-1), then simply add a new state of the DFA in that spot, with
			 * owner state the current state and next hop address the next available
			 * space.
			 */
			if (getCheck(transition) == EMPTY_VALUE) {
				setCheck(transition, state);
				if (i == string.size() - 1) { 				// The string is done
					setBase(transition, LEAF_BASE_VALUE); 	// So this is a leaf
					changed = true;
				}
				else {
					setBase(transition, nextAvailableHop(string[i + 1])); // Add a state
					changed = true;
				}
			}
			else if (getCheck(transition) != state) { // We have been through here before
				/*
				 *
				 * The place we must add a new children state is already
				 * occupied. Move this state's base to a new location.
				 */
				resolveConflict(state, c);
				changed = true;
				// We must redo this character
				continue;
			}
			/*
			 * There is another case that is the default and always executed
			 * by the if above. That is simply transition through the DFA
			 * and advance the string index. This is done after we notify
			 * for the transition event.
			 */
			//updateInsert(state, i - 1, string);
			state = transition;
			i++;
		}
		return changed;
	}

	/**
	 * This method is the most complex part of the algorithm.
	 * First of all, keep in mind that the children of a state
	 * are stored in ordered locations. That means that there is the possibility
	 * that although a new child for state s must be added, the position
	 * has already been taken. This is the conflict that is resolved here.
	 * There are two ways. One is to move the obstructing state to a new
	 * location and the other is to move the obstructed state. Here the
	 * latter is chosen. This also ensures that the root node is never moved.
	 * @param s The state to move
	 * @param newValue The value that causes the conflict.
	 */
	void resolveConflict(int s, int newValue) {

		// The set of children values
		set<int> values;

		// Add the value-to-add 
		values.insert(newValue);

		// Find all existing children and add them too.
		for (int c = 0; c < alphabetLength; c++) {
			int tempNext = getBase(s) + c;
			if (tempNext >= 0 && tempNext < getSize() && getCheck(tempNext) == s)
				values.insert(c);
		}

		// Find a place to move them.
		int newLocation = nextAvailableMove(values);

		// newValue is not yet a child of s, so we should not check for it.
		values.erase(newValue);

		/*
		 * This is where the job is done. For each child of s,
		 */
		for (set<int>::iterator it = values.begin(); it != values.end(); it++) {
			int c = *it;		// The child state to move
			int tempNext = getBase(s) + c;	// 
			assert(tempNext < getSize());
			assert(getCheck(tempNext) == s);
			/*
			 * base(s)+c state is child of s.
			 * Mark new position as owned by s.
			 */
			assert(getCheck(newLocation + c) == EMPTY_VALUE);
			setCheck(newLocation + c, s);

			/*
			 * Copy pointers to children for this child of s.
			 * Note that even if this child is a leaf, this is needed.
			 */
			assert(getBase(newLocation + c) == EMPTY_VALUE);
			setBase(newLocation + c, getBase(getBase(s) + c));
			updateChildMove(s, c, newLocation);
			/*
			 * Here the child c is moved, but not *its* children. They must be
			 * updated so that their check values point to the new position of their
			 * parent (i.e. c)
			 */
			if (getBase(getBase(s) + c) != LEAF_BASE_VALUE) {
				// First, iterate over all possible children of c
				for (int d = 0; d < alphabetLength; d++) {
					/*
					 *  Get the child. This could well be beyond the store size
					 *  since we don't know how many children c has.
					 */
					int tempNextChild = getBase(getBase(s) + c) + d;
					/*
					 * Here we could also check if tempNext > 0, since
					 * negative values end the universe. However, since the
					 * implementation of nextAvailableHop never returns
					 * negative values, this should never happen. Presto, a
					 * nice way of catching bugs.
					 */
					if (tempNextChild < getSize() && getCheck(tempNextChild) == getBase(s) + c) {
						// Update its check value, so that it shows to the new position of this child of s.
						setCheck(getBase(getBase(s) + c) + d, newLocation + c);
					}
					else if (tempNextChild >= getSize()) {
						/*
						 *  Minor optimization here. If the above if fails then tempNextChild > check.size()
						 *  or the tempNextChild position is already owned by some other state. Remember
						 *  that children states are stored in increasing order (though not necessarily
						 *  right next to each other, since other states can be between the gaps they leave).
						 *  That means that failure of the second part of the conjuction of the if above
						 *  does not mean failure, since the next child can exist. Failure of the first conjuct
						 *  however means we are done, since all the rest of the children will only be further
						 *  down the store and therefore beyond its end also. Nothing left to do but break
						 */
						break;
					}
				}
				// Finally, free the position held by this child of s
				setBase(getBase(s) + c, EMPTY_VALUE);
				setCheck(getBase(s) + c, EMPTY_VALUE);
			}
		}
		// Here, all children and grandchildren (if existent) of s have been
		// moved or updated. That which remains is for the state s to show
		// to its new children
		setBase(s, newLocation);
		
		//updateStateMove(s, newLocation);
		//there is no implementation of this function in the original code
		//I hope it's not important
	}

	
	int getAlphabetSize() {
		return alphabetLength;
	}

};

vector <int> stringToTokens(string input) {
	vector <int> output;
	for (int i = 0; i < input.size(); ++i)	
		output.push_back(input[i] - 'a');
	return output;
}
void addStrings(vector<string> strings, DoubleArrayTrie& datrie) {
	for (int i = 0; i < strings.size(); ++i)
	{
		datrie.addToTrie(stringToTokens(strings[i]));
	}
}
int main() {
	
	DoubleArrayTrie datrie(26);
	addStrings({ "cat", "car", "cargo", "cabin", "metal", "mental", "metallica", "mask", "tree", "train", "dispatcher"}, datrie);
	cout << datrie.getAlphabetSize();

}