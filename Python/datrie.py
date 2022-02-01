LEAF_BASE_VALUE = -2
ROOT_CHECK_VALUE = -3
EMPTY_VALUE = -1
INITIAL_ROOT_BASE = 1
from sortedcontainers import SortedSet


class DoubleArrayTrie:
    def __init__(self, alphabetLength):
        self.alphabetLength = alphabetLength
        self.base = []
        self.check = []
        self.freePositions = SortedSet()
        self.base.append(INITIAL_ROOT_BASE)
        self.check.append(ROOT_CHECK_VALUE)

    def getBase(self, position):
        return self.base[position]

    def getCheck(self, position):
        return self.check[position]

    def setBase(self, position, value):
        self.base[position] = value
        if value == EMPTY_VALUE:
            self.freePositions.add(position)
        else:
            if position in self.freePositions:
                self.freePositions.remove(position)

    def setCheck(self, position, value):
        self.check[position] = value
        if value == EMPTY_VALUE:
            self.freePositions.add(position)
        else:
            if position in self.freePositions:
                self.freePositions.remove(position)

    def getSize(self):
        return len(self.base)

    def updateChildMove(self, parentIndex, forCharacter, newParentBase):
        assert self.getCheck(self.getBase(parentIndex) + forCharacter) == parentIndex

    def nextAvailableHop(self, forValue):
        while self.freePositions.bisect_right(forValue) >= len(self.freePositions):
            self.ensureReachableIndex(self.getSize() + 1)
        result = self.freePositions[self.freePositions.bisect_right(forValue)] - forValue
        assert result >= 0
        return result

    def ensureReachableIndex(self, limit):
        # TODO This can be implemented in O(1)
        while self.getSize() <= limit:
            self.base.append(EMPTY_VALUE)
            self.check.append(EMPTY_VALUE)
            self.freePositions.add(self.getSize() - 1)

    def findConsecutiveFree(self, amount: int):
        assert amount >= 0
        if not len(self.freePositions):
            return -1
        i = 1
        _from = self.freePositions[0]
        previous = _from
        consecutive = 1
        # TODO This cycle can be more intuitive
        while consecutive < amount and i < len(self.freePositions):
            current = self.freePositions[i]
            if current - previous == 1:
                previous = current
                consecutive += 1
            else:
                _from = current
                previous = _from
                consecutive = 1
            i += 1
        if consecutive == amount:
            return _from
        else:
            return -1

    def nextAvailableMove(self, values: SortedSet):
        if len(values) == 1:
            return self.nextAvailableHop(values[0])
        minValue = values[0]
        maxValue = values[-1]
        neededPositions = maxValue - minValue + 1
        possible = self.findConsecutiveFree(neededPositions)
        if possible - minValue >= 0:
            return possible - minValue
        self.ensureReachableIndex(self.getSize() + neededPositions)
        return self.getSize() - neededPositions - minValue

    def addToTrie(self, string: list):
        changed = False
        state = 0
        transition = 0
        i = 0
        c = 0
        while i < len(string):
            assert state >= 0
            c = string[i]
            stateBase = self.getBase(state)
            if i > 0 and stateBase == LEAF_BASE_VALUE:
                self.setBase(transition, self.nextAvailableHop(c))
                changed = True
            else:
                assert self.getBase(state) >= 0
            transition = self.getBase(state) + c
            assert transition > 0
            self.ensureReachableIndex(transition)
            if self.getCheck(transition) == EMPTY_VALUE:  ##
                self.setCheck(transition, state)
                if i == len(string) - 1:
                    self.setBase(transition, LEAF_BASE_VALUE)
                    changed = True
                else:
                    self.setBase(transition, self.nextAvailableHop(string[i + 1]))
                    changed = True
            else:
                if self.getCheck(transition) != state:
                    self.resolveConflict(state, c)
                    changed = True
                    continue
            state = transition
            i += 1
        return changed

    def resolveConflict(self, s, newValue):
        values = SortedSet()
        values.add(newValue)
        for c in range(self.alphabetLength):
            tempNext = self.getBase(s) + c
            if tempNext >= 0 and tempNext < self.getSize() and self.getCheck(tempNext) == s:
                values.add(c)
        newLocation = self.nextAvailableMove(values)
        values.remove(newValue)
        for i in range(len(values)):
            c = values[i]
            tempNext = self.getBase(s) + c
            assert tempNext < self.getSize()
            assert self.getCheck(tempNext) == s
            assert self.getCheck(newLocation + c) == EMPTY_VALUE
            self.setCheck(newLocation + c, s)
            assert self.getBase(newLocation + c) == EMPTY_VALUE
            self.setBase(newLocation + c, self.getBase(self.getBase(s) + c))
            self.updateChildMove(s, c, newLocation)

            if self.getBase(self.getBase(s) + c) != LEAF_BASE_VALUE:
                for d in range(self.alphabetLength):
                    tempNextChild = self.getBase(self.getBase(s) + c) + d
                    if tempNextChild < self.getSize() and self.getCheck(tempNextChild) == self.getBase(s) + c:
                        self.setCheck(self.getBase(self.getBase(s) + c) + d, newLocation + c)
                    elif tempNext >= self.getSize():
                        break
                self.setBase(self.getBase(s) + c, EMPTY_VALUE)
                self.setCheck(self.getBase(s) + c, EMPTY_VALUE)
        self.setBase(s, newLocation)


def stringToTokens(input):
    output = []
    for i in range(len(input)):
        output.append(ord(input[i]) - ord('a'))
    return output


def addStrings(strings, datrie):
    for i in range(len(strings)):
        datrie.addToTrie(stringToTokens(strings[i]))


if __name__ == '__main__':
    datrie = DoubleArrayTrie(26)
    addStrings(["cat", "car", "cargo", "cabin", "metal", "mental", "metallica", "mask", "tree", "train", "dispatcher"],
               datrie)
    print('BASE:', datrie.base)
    print('CHECK:', datrie.check)
