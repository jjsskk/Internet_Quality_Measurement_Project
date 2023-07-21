all: 
	g++ -std=c++11 perf_server.cpp -o perf_server -lboost_system -pthread
	g++ -std=c++11 perf_client.cpp -o perf_client -lboost_system -pthread

clean:
	rm perf_server 
	rm perf_client
	