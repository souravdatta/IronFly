all: dist

distfolder:
	mkdir dist

dist: IronFly distfolder
	mv IronFly dist
	cp -R resources dist


IronFly: stdafx.h IronFly.cpp
	clang++ -std=c++14 -O2 -o IronFly IronFly.cpp -lsfml-system -lsfml-window -lsfml-graphics -lsfml-audio

.PHONY:
clean:
	rm -fr dist IronFly

