all: check
check: test-c test-cpp
	./test-c && ./test-cpp && echo "All tests passed!"
test-c: test.c
	$(CC) -g -ansi -pedantic -Wall -Wextra -Werror -o $@ $^ -I..
test-cpp: test.cpp
	$(CXX) -g -ansi -pedantic -Wall -Wextra -Werror -o $@ $^ -I..
clean:
	rm -f test-c test-cpp
.PHONY: all check clean
