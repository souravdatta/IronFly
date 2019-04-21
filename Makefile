all:
	@echo "Please specify target (linux -> linux build, macos -> Xcode project generation)"

linux: dist

distfolder:
	mkdir dist

dist: IronFly distfolder
	mv IronFly dist
	cp -R resources dist


IronFly: stdafx.h IronFly.cpp
	clang++ -std=c++14 -O2 -o IronFly IronFly.cpp -lsfml-system -lsfml-window -lsfml-graphics -lsfml-audio


macos: macoscopy
	@echo "Please open the project file in Xcode and build"

macoscopy:
	cp IronFly.cpp ./IronFlyMacOS/IronFlyMacOS/main.cpp
	cp *.h ./IronFlyMacOS/IronFlyMacOS/
	cp -r resources ./IronFlyMacOS/IronFlyMacOS/


.PHONY:
clean:
	rm -fr dist IronFly
	rm -fr ./IronFlyMacOS/IronFlyMacOS/*.cpp ./IronFlyMacOS/IronFlyMacOS/resources ./IronFlyMacOS/IronFlyMacOS/*.h

