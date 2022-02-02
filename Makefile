simples:
	cd ap-paralelo-simples && make simples

complexo:
	cd ap-paralelo-complexo && make complexo

dinamico:
	cd ap-paralelo-dinamico && make dinamico

pipeline:
	cd ap-paralelo-pipeline && make pipeline

remove-images:
	cd dataset1 && rm -rf Resize Thumbnail Watermark stats.csv
	cd dataset2 && rm -rf Resize Thumbnail Watermark stats.csv

zip-stats:
	zip stats.zip dataset1/*.csv dataset2/*.csv
	cd dataset1 && rm -f *.csv
	cd dataset2 && rm -f *.csv

zip-submissao: remove-images
	cd ap-paralelo-simples && make clean
	cd ap-paralelo-complexo && make clean
	cd ap-paralelo-dinamico && make clean
	cd ap-paralelo-pipeline && make clean
	zip -r projeto-pconc ap-paralelo-simples ap-paralelo-complexo ap-paralelo-dinamico ap-paralelo-pipeline Images Makefile README.MD run-teste.py pconcrelatorio-1.pdf
