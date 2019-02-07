.PHONY:all
cc = g++
all:server client 
server:Select.hpp Server_Select.hpp Tcp_Server.cc 
	$(cc) -std=c++11 -o  $@ $^ -g
client:Tcp_Client.cc
	$(cc) -o $@ $^
.PHONY:clean
clean:
	rm -rf server client

