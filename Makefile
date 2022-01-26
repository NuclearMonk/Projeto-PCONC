simples:
	cd ap-paralelo-simples && make simples

complexo:
	cd ap-paralelo-complexo && make complexo

dinamico:
	cd ap-paralelo-dinamico && make dinamico

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
