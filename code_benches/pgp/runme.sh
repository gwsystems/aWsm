sim-safe ../src/pgp -sa -z "this is a test" -u taustin@eecs.umich.edu testin.txt austin@umich.edu
diff testin.txt.asc testin.txt.asc.ref

sim-safe ../src/pgp -z "this is a test" -u taustin@eecs.umich.edu testout.txt.asc
diff testout.txt testout.txt.ref
