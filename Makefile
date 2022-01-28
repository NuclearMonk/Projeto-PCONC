simples:
	cd ap-paralelo-simples && make simples

complexo:
	cd ap-paralelo-complexo && make complexo

dinamico:
	cd ap-paralelo-dinamico && make dinamico

remove-images:
	cd Images/Small && rm -rf Resize Thumbnail Watermark stats.csv

zip-stats:
	zip stats.zip Images/Small/*.csv
	cd Images/Small && rm -f *.csv

zip-submissao: remove-images
	cd ap-paralelo-simples && make clean
	cd ap-paralelo-complexo && make clean
	cd ap-paralelo-dinamico && make clean
	zip -r projeto-pconc ap-paralelo-simples ap-paralelo-complexo ap-paralelo-dinamico Images Makefile README.MD run-teste.py pconcrelatorio-1.pdf 
pipeline:
	cd ap-paralelo-pipeline && make pipeline
