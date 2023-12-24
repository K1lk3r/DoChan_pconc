# all: old-photo-serial

# old-photo-serial: old-photo-serial.c image-lib.c image-lib.h
# 	gcc old-photo-serial.c image-lib.c image-lib.h -g -o old-photo-serial -lgd

all: old-photo-paralelo-A

old-photo-paralelo-A: old-photo-paralelo-A.c image-lib.c image-lib.h 
	gcc	-Wall	-g old-photo-paralelo-A.c image-lib.c image-lib.h  -o old-photo-paralelo-A -lgd -pthread
