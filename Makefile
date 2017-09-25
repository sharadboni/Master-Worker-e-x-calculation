default:  install 
	
install:	
	gcc -o worker worker.c -lm
	gcc -o master master.c
remove:	worker master	
	rm worker master

run:	worker master
	./master --worker_path ./worker --num_workers 5 --wait_mechanism select -x 2 -n 8


