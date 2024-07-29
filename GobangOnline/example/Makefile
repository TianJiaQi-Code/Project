mysql:mysql.c
	gcc -o $@ $^ -L/usr/lib/x86_64-linux-gnu/ -lmysqlclient
json:json.cpp
	g++ -o $@ $^ -std=c++11 -lpthread -lboost_system -ljsoncpp
wsserver:wsserver.cpp
	g++ -o $@ $^ -std=c++11 -lpthread -lboost_system