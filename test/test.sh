./build.sh
cd test
valgrind ../cscript test.csc > out.txt
diff out.txt correct.txt
