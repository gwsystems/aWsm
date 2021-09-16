
libuv/CMakeLists.txt:
	wget https://github.com/libuv/libuv/archive/refs/tags/v1.42.0.tar.gz
	tar -xvf v1.42.0.tar.gz
	mv libuv-1.42.0/ libuv
	rm v1.42.0.tar.gz

dist/lib/libuv_a.a: libuv/CMakeLists.txt
	mkdir -p dist/lib/
	cd libuv && mkdir -p build && cd build && cmake .. && cmake --build .
	cp ./libuv/build/libuv_a.a dist/lib/libuv_a.a

.PHONY: libuv.install
libuv.install: dist/lib/libuv_a.a

.PHONY: libuv.uninstall
libuv.uninstall:
	rm -rf libuv
	rm -f dist/lib/libuv_a.a

uvwasi/CMakeLists.txt:
	wget https://github.com/nodejs/uvwasi/archive/refs/tags/v0.0.11.tar.gz
	tar -xvf v0.0.11.tar.gz
	mv uvwasi-0.0.11 uvwasi
	rm v0.0.11.tar.gz

dist/lib/libuvwasi_a.a: uvwasi/CMakeLists.txt
	mkdir -p dist/lib/
	cd uvwasi && mkdir -p out/cmake && cd out/cmake  && cmake ../.. -DBUILD_TESTING=ON && cmake --build . && ctest -C Debug --output-on-failure
	cp ./uvwasi/out/cmake/libuvwasi_a.a dist/lib/libuvwasi_a.a

dist/include/uvwasi.h: uvwasi/CMakeLists.txt
	mkdir -p dist/include/
	cp uvwasi/include/* dist/include/

.PHONY: uvwasi.install
uvwasi.install:	dist/lib/libuvwasi_a.a dist/include/uvwasi.h

.PHONY: uvwasi.uninstall
uvwasi.uninstall:
	rm -rf uvwasi
	rm -f dist/lib/libuvwasi_a.a
	rm -f dist/include/uvwasi.h
	rm -f dist/include/wasi_serdes.h
	rm -f dist/include/wasi_types.h

.PHONY: install
install: libuv.install uvwasi.install

.PHONY: uninstall
uninstall: libuv.uninstall uvwasi.uninstall
