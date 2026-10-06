- on parcours tous les triangles 
    - si dans un triangle -> facesplit
    - si dans une edge 
        - trouver l'edge dans laquelle on est (are colinear)
        - edgesplit
    - sinon 
        - recup tous les bords 
        - prendre une des faces et la split
        - ensuite pour chaque edge restante
            - edgeflip de l'edge entre infini et le sommet dans l'arrete et déja relié à p 
            - (si dans l'ordre on retiens juste le premier sommet de l'edge : AB, BC, CD on fait faceplit de ABinf et ensuit epremier flip -> flip(Binf), deuxieme flip -> Cinf)


# TODO
- finir triangulation
- réparer laplacien
- modifier face split pour ne pas delete de face
- clean le code 
- commenter
- rapport
