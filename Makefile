hw2:
	gcc -Wall main.c -o main

valgrind:
	valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         --log-file=valgrind-out.txt \
         ./main
clean:
	rm *.log main valgrind-out.txt