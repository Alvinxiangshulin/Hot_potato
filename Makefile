TARGETS=ringmaster player

all: $(TARGETS)
clean:
	rm -f $(TARGETS)

ringmaster: ringmaster.cpp
	g++ -std=c++11 -g -o $@ $<

player: player.cpp
	g++ -std=c++11 -g -o $@ $<

