all: ../bin/serveur3-TCPoverUPDinTCA ../bin/serveur2-TCPoverUPDinTCA ../bin/serveur1-TCPoverUPDinTCA

../bin/serveur1-TCPoverUPDinTCA : serveur1-TCPoverUPDinTCA.c
	gcc serveur1-TCPoverUPDinTCA.c -o ../bin/serveur1-TCPoverUPDinTCA

../bin/serveur2-TCPoverUPDinTCA : serveur2-TCPoverUPDinTCA.c
	gcc serveur2-TCPoverUPDinTCA.c -o ../bin/serveur2-TCPoverUPDinTCA

../bin/serveur3-TCPoverUPDinTCA : serveur3-TCPoverUPDinTCA.c
	gcc serveur3-TCPoverUPDinTCA.c -o ../bin/serveur3-TCPoverUPDinTCA

clean:
	rm -f *.o

