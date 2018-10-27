Bonjour Alex,

Plusieurs modification, mais tout fonctionne...

Étapes:

1 - Clement accepte la connection de Agnesse et de Bob
2 - Clement les authentifie
3 - On entre dans la fonction scriptedconvo
4 - Clement demande a Agnesse a qui elle veut parler
5 - Agnesse doit répondre Bob
6 - Clement valide la réponse d'Agnesse (qui répond BOB0
7 - Clément envoie un message à Anesse et a Bob pour confirmer la confiance des 2 partie
8 - Clément envoie la clé de session et la cle mac de session à Agnesse et à Bob ainsi qu'une information de communication pour utiliser une adresseIP et un port spécifique pour la communication.
9 - Clément valide les informations reseau et se met en attente d'une connection sur le port mentionné
10 - Agnesse prend les information et fait une tentative de connection sur l'addresse et port mentionné
11 - Une fois la connection établit, agnesse envoie son premier message, soit, sa clé de session, le tout encrypté et avec un MAC
12 - Clement authentifie Agnesse. Son message doit être la même clé de session qu'il a, sinon, la communication est rompue.
13 - Bob envoie à Agnesse un message "is BOB" qu'Agnesse authentifie.
14 - La communicatione st officiellement établit.
15 - À partir de ce moment, les messages sont cryptés avec la clé de session et avec le MAC de session,
16 - On se parle tant qu'un des deux n'écrit pas "end" tout seul en string.

Cheers!

Dan.