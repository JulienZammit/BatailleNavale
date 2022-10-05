all: navale.c
	gcc -o navale navale.c
	./navale

git: 
	git add .
	git commit -m "v2"
	git push

import:
	git pull origin master