Bonjour Alex,

Plusieurs modification, mais tout fonctionne...

�tapes:

1 - Clement accepte la connection de Agnesse et de Bob
2 - Clement les authentifie
3 - On entre dans la fonction scriptedconvo
4 - Clement demande a Agnesse a qui elle veut parler
5 - Agnesse doit r�pondre Bob
6 - Clement valide la r�ponse d'Agnesse (qui r�pond BOB0
7 - Cl�ment envoie un message � Anesse et a Bob pour confirmer la confiance des 2 partie
8 - Cl�ment envoie la cl� de session et la cle mac de session � Agnesse et � Bob ainsi qu'une information de communication pour utiliser une adresseIP et un port sp�cifique pour la communication.
9 - Cl�ment valide les informations reseau et se met en attente d'une connection sur le port mentionn�
10 - Agnesse prend les information et fait une tentative de connection sur l'addresse et port mentionn�
11 - Une fois la connection �tablit, agnesse envoie son premier message, soit, sa cl� de session, le tout encrypt� et avec un MAC
12 - Clement authentifie Agnesse. Son message doit �tre la m�me cl� de session qu'il a, sinon, la communication est rompue.
13 - Bob envoie � Agnesse un message "is BOB" qu'Agnesse authentifie.
14 - La communicatione st officiellement �tablit.
15 - � partir de ce moment, les messages sont crypt�s avec la cl� de session et avec le MAC de session,
16 - On se parle tant qu'un des deux n'�crit pas "end" tout seul en string.

Cheers!

Dan.