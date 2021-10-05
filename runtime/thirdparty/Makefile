libuv-1.42.0:
	wget https://github.com/libuv/libuv/archive/refs/tags/v1.42.0.tar.gz
	tar -xvf v1.42.0.tar.gz
	rm v1.42.0.tar.gz*

dist/lib/libuv_a.a: 
	mkdir -p dist/lib/
	cd libuv-1.42.0 && mkdir -p build && cd build && cmake .. && cmake --build .
	cp ./libuv-1.42.0/build/libuv_a.a dist/lib/libuv_a.a

.PHONY: libuv.install
libuv.install: dist/lib/libuv_a.a

.PHONY: libuv.uninstall
libuv.uninstall:
	rm -rf libuv
	rm -f dist/lib/libuvwasi_a.a

uvwasi-0.0.11:
	wget https://github.com/nodejs/uvwasi/archive/refs/tags/v0.0.11.tar.gz
	tar -xvf v0.0.11.tar.gz
	rm v0.0.11.tar.gz

uvwasi-0.0.11/include/uvwasi.h: uvwasi-0.0.11
uvwasi-0.0.11/include/wasi_serdes.h: uvwasi-0.0.11
uvwasi-0.0.11/include/wasi_types.h: uvwasi-0.0.11

# Install Headers from uvwasi. All backings use serdes and types from this library.
dist/include/uvwasi.h: uvwasi-0.0.11/include/uvwasi.h
	cp -u $^ $@
dist/include/wasi_serdes.h: uvwasi-0.0.11/include/wasi_serdes.h
	cp -u $^ $@
dist/include/wasi_types.h: uvwasi-0.0.11/include/wasi_types.h
	cp -u $^ $@

dist/lib/libuvwasi_a.a: uvwasi-0.0.11
	mkdir -p dist/lib/
	mkdir -p dist/include/
	cd uvwasi-0.0.11 && mkdir -p out/cmake && cd out/cmake  && cmake ../.. -DBUILD_TESTING=ON && cmake --build . && ctest -C Debug --output-on-failure
	cp ./uvwasi-0.0.11/out/cmake/libuvwasi_a.a dist/lib/libuvwasi_a.a

.PHONY: uvwasi.install
uvwasi.install: dist/lib/libuvwasi_a.a dist/include/uvwasi.h dist/include/wasi_serdes.h dist/include/wasi_types.h

.PHONY: uvwasi.uninstall
uvwasi.uninstall:
	rm -rf uvwasi
	rm -f dist/lib/libuv_a.a
	rm -f dist/include/uvwasi.h
	rm -f dist/include/wasi_serdes.h
	rm -f dist/include/wasi_types.h

.PHONY: install
install: libuv.install uvwasi.install

.PHONY: uninstall
uninstall: libuv.uninstall uvwasi.uninstall
