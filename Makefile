paralelo-simples:
	cd simples && make

paralelo-complexo:
	cd complexo && make

test-simples:
	rm -r -f Images/Small/Resize;\
	rm -r -f Images/Small/Thumbnail;\
	rm -r -f Images/Small/Watermark;\
	cd simples;\
	make;\
	make teste;
