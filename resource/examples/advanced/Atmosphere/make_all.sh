

for P in `ls *.cxx`; do
  make -f ${MEGALIB}/resource/standalone/Makefile.standalone PRG=${P}
done

