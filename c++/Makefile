namidi: main.cpp parser.cpp libnamidiyy.a console_writer.cpp fswatcher.cpp namidi.cpp midi_client.cpp player.cpp
	g++ -std=c++11 -o namidi main.cpp namidi.cpp parser.cpp player.cpp console_writer.cpp fswatcher.cpp midi_client.cpp -L. -lnamidiyy -framework Foundation -framework CoreServices -framework CoreMidi

libnamidiyy.a: namidi.tab.o namidi.yy.o
	ar rv libnamidiyy.a namidi.tab.o namidi.yy.o

namidi.tab.o: namidi.tab.c
	gcc -c namidi.tab.c -o namidi.tab.o

namidi.yy.o: namidi.yy.c
	gcc -c namidi.yy.c -o namidi.yy.o 

namidi.yy.c: namidi.l namidi.tab.h
	flex -i -onamidi.yy.c namidi.l

namidi.tab.h: namidi.y
	bison -d --report=state namidi.y

namidi.tab.c: namidi.y
	bison -d --report=state namidi.y

clean:
	rm namidi libnamidiyy.a namidi.tab.o namidi.yy.o
