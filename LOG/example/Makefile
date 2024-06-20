.PHONY:all
all:test exam

test:Test.cc
	g++ -o $@ $^ -std=c++11 -lpthread
exam:Exam.cc
	g++ -o $@ $^ -std=c++11 -lpthread

.PHONY:clean
clean:
	rm -rf test exam logfile/