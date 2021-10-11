libuv-1.42.0/CMakeLists.txt:
	wget https://github.com/libuv/libuv/archive/refs/tags/v1.42.0.tar.gz
	tar -xvf v1.42.0.tar.gz
	rm v1.42.0.tar.gz*

dist/lib/libuv_a.a: libuv-1.42.0/CMakeLists.txt
	mkdir -p dist/lib/
	cd libuv-1.42.0 && mkdir -p build && cd build && cmake .. && cmake --build .
	cp ./libuv-1.42.0/build/libuv_a.a dist/lib/libuv_a.a

.PHONY: libuv.install
libuv.install: dist/lib/libuv_a.a

.PHONY: libuv.uninstall
libuv.uninstall:
	rm -rf libuv-1.42.0
	rm -f dist/lib/libuv_a.a

UVWASI_HEADERS = uvwasi-0.0.11/include/uvwasi.h uvwasi-0.0.11/include/wasi_serdes.h uvwasi-0.0.11/include/wasi_types.h
UVWASI_INSTALLED_HEADERS = $(patsubst uvwasi-0.0.11/include/%.h, dist/include/%.h, $(UVWASI_HEADERS))

${UVWASI_HEADERS} uvwasi-0.0.11/CMakeLists.txt &:
	wget https://github.com/nodejs/uvwasi/archive/refs/tags/v0.0.11.tar.gz
	tar -xvf v0.0.11.tar.gz
	rm v0.0.11.tar.gz

# Install Headers from uvwasi. All backings use serdes and types from this library.
dist/include/%.h: uvwasi-0.0.11/include/%.h
	mkdir -p dist/include/
	cp -u $^ $@

dist/lib/libuvwasi_a.a: uvwasi-0.0.11/CMakeLists.txt
	mkdir -p dist/lib/
	cd uvwasi-0.0.11 && mkdir -p out/cmake && cd out/cmake  && cmake ../.. -DBUILD_TESTING=ON && cmake --build . && ctest -C Debug --output-on-failure
	cp ./uvwasi-0.0.11/out/cmake/libuvwasi_a.a dist/lib/libuvwasi_a.a

.PHONY: uvwasi.install
uvwasi.install: ${UVWASI_INSTALLED_HEADERS} dist/lib/libuvwasi_a.a

.PHONY: uvwasi.uninstall
uvwasi.uninstall:
	rm -rf uvwasi-0.0.11
	rm -f dist/lib/libuvwasi_a.a
	rm -f ${UVWASI_INSTALLED_HEADERS}

.PHONY: install
install: libuv.install uvwasi.install

.PHONY: uninstall
uninstall: libuv.uninstall uvwasi.uninstall
