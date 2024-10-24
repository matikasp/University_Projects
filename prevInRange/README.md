## This library implements a data structure that allows efficient responses to specific queries about a sequence of numbers X. Furthermore, the successive elements of the sequence may be revealed online.

void init(const vector<int> &x) – initializes the initial sequence X with the values from the vector x (note: the sequence can contain any values that fit in the int type).

int prevInRange(int i, int lo, int hi) – computes the value of prevInRange(i, [lo, hi]). You can assume that 0 ≤ i < |X| and INT_MIN ≤ lo ≤ hi ≤ INT_MAX.

void pushBack(int v) – adds an element with the value v to the end of the current sequence X.

void done() – frees all the memory used to handle the sequence X.
