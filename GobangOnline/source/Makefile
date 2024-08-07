.PHONY:gobang
gobang:Gobang.cc
	g++ -g -o $@ $^ -std=c++11 -lpthread -ljsoncpp -L/usr/lib/x86_64-linux-gnu/ -lmysqlclient

.PHONY:clean
clean:
	rm gobang