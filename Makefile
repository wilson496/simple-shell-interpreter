OUT_EXE=ssi
all: ssi

ssi: ssi.cpp
	g++ -g ssi.cpp -lreadline -o ssi

clean:
	rm -f ${OUT_EXE} *.o
