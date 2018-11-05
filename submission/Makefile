all:
	gcc -g -pg -c -Wall verzijl/engine.c
	gcc -g -pg verzijl/test.c -o test
	gcc -g -pg verzijl/score.c -lm -lrt -o score

profile:
	gcc -g -pg -O3 -c verzijl/engine.c
	gcc -g -pg -O3 verzijl/test.c -o test
	gcc -g -pg -O3 verzijl/score.c -lm -lrt -o score

fastest:
	gcc -O3 -c verzijl/engine.c
	gcc -O3 verzijl/test.c -o test
	gcc -O3 verzijl/score.c -lm -lrt -o score


intel:
	icc -g -c -Wall verzijl/engine.c
	icc -g verzijl/test.c -o test
	icc -g verzijl/score.c -lm -lrt -o score

clean:
	rm -f engine.o test score a.out *~
