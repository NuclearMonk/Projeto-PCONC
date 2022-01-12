paralelo-simples:
	cd simples && make all

paralelo-complexo:
	cd complexo && make all

test-simples:
	rm -r -f Images/Small/Resize;\
	rm -r -f Images/Small/Thumbnail;\
	rm -r -f Images/Small/Watermark;\
	cd simples;\
	make;\
	make teste;

test-complexo:
	rm -r -f Images/Small/Resize;\
	rm -r -f Images/Small/Thumbnail;\
	rm -r -f Images/Small/Watermark;\
	cd complexo;\
	make;\
	make teste;
