Resolution de l'énigme à l'aise
---

Que ce soit verticalement ou horizontalement, les suites se comportent comme buffers circulaires
avec un décalage de 1 lorsque l'on va de gauche à droite ou du haut vers le bas.

Le décalage correspond a la position de la lettre dans l’ensemble des caractères chiffrables,
ainsi la clè B en 2eme position contiens la même suite que la clé A avec un décalage de 1,
B étant en première position et A en dernière.

Il est donc  possible d'utiliser ce mechanism pour calculer la lettre en clair sans a avoir à
reproduire l'ensemble du tableau à deux dimensions.

Dans ma solution j'ai codé en dur le fait que l'on a pris la suite des lettre de A à Z, mais il serait
possible sans trop d'effort de passer cet ensemble en paramètre pour rendre le programme plus facilement
adaptable.

