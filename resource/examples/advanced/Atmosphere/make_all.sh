

for P in `ls *.cxx`; do
  make -f ${MEGALIB}/resource/standalone/Makefile.StandAlone PRG=${P}
done

