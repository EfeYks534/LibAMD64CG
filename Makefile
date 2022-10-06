all:
	@echo "make install - Installs static library"
	@echo "make lib     - Makes static library"
	@echo "make test    - Makes example test program"
	@echo "make clean   - Cleans binaries"

install: lib
	sudo cp ./libamd64cg.a /usr/lib/libamd64cg.a

lib: libamdcg64.a

libamdcg64.a: Gen.o
	ar -crs libamd64cg.a Gen.o

test: Gen.o Test.o
	clang *.o -o cgt

Gen.o: Gen.c AMD64CG.h
	clang -c Gen.c -o Gen.o
Test.o: Test.c AMD64CG.h
	clang -c Test.c -o Test.o

.PHONY: clean test lib install

clean:
	rm -f ./*.o
	rm -f ./cgt
	rm -f ./libamd64cg.a
