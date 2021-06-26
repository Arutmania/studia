if [ -e examples ] || [ -L examples ]; then
    echo "$0 creates and uses directory ./examples but one already exists"
    exit 1
fi

echo '---------------------------------------------------------------'
echo 'creating some files and directories'
mkdir -p  examples/{ala,ma,kota}/{ala,ma,kota}
touch examples/file{1..5} examples/{ala,ma,kota}/file{1..5}
ls -lR examples
echo '---------------------------------------------------------------'

echo '---------------------------------------------------------------'
echo 'correct usage'
echo 'help:'
echo './modify.sh'
./modify.sh
echo '---------------------------------------------------------------'

echo '---------------------------------------------------------------'
echo './modify.sh -h'
./modify.sh -h
echo '---------------------------------------------------------------'

echo '---------------------------------------------------------------'
echo 'uppercase some files'
echo './modify.sh -u examples/file{1,3,5} examples/ma/file{2,4}'
./modify.sh -u examples/file{1,3,5} examples/ma/file{2,4}
ls -lR examples
echo '---------------------------------------------------------------'

echo '---------------------------------------------------------------'
echo 'lowercase all files directly in examples - some were uppercase'
echo './modify.sh -l examples/*'
./modify.sh -l examples/*
ls -lR examples
echo '---------------------------------------------------------------'

echo '---------------------------------------------------------------'
echo 'uppercase all files and directories in examples but not examples itself'
echo './modify.sh -ur examples'
./modify.sh -ur examples
ls -lR examples
echo '---------------------------------------------------------------'
# empty target, sed fail, 

echo '---------------------------------------------------------------'
echo 'lowercase recursively files in examples/ala and examples/kota'
echo './modify.sh -lr examples/{ALA,KOTA}'
./modify.sh -lr examples/{ALA,KOTA}
ls -lR examples
echo '---------------------------------------------------------------'

echo '---------------------------------------------------------------'
echo 'replace recurisvely file with f1l3'
echo "./modify.sh -r 's/file/f1l3/g'"
./modify.sh -r 's/file/f1l3/g' examples/
ls -lR examples
echo '---------------------------------------------------------------'

echo '---------------------------------------------------------------'
echo 'incorrect usage'
echo 'bad flags'
echo './modify.sh -badflags'
./modify.sh -badflags examples/
echo '---------------------------------------------------------------'

echo '---------------------------------------------------------------'
echo 'nonexistent files'
echo './modify.sh -l bad-files'
./modify.sh -l bad-files
echo '---------------------------------------------------------------'

echo '---------------------------------------------------------------'
echo 'bad pattern'
echo './modify.sh bad-pattern'
./modify.sh bad-pattern examples/
echo '---------------------------------------------------------------'

echo '---------------------------------------------------------------'
echo 'special characters'
echo 'create files with special characters in their name'
echo "touch 'examples/!!aa' 'examples/&&aa' 'examples/[;aa' 'examples/--aa'"
touch 'examples/!!aa' 'examples/&&aa' 'examples/[;aa' 'examples/--aa'
echo './modify.sh -u examples/*aa'
./modify.sh -u examples/*aa
ls -lR
echo '---------------------------------------------------------------'
