#! /bin/sh

echo ""
echo "Generating doc/ChangeLog from CVS logs..."
echo ""

Script=`which cvs2cl.pl`
if [ "$Script" == "" ] || [[ "$Script" == which* ]]; then
    if ( `test ! -f $MEGALIB/bin/cvs2cl.pl` ); then
        echo "Grummel... latest script not found... Trying to download it..."
        cd /tmp
        cvs -d :pserver:anonymous@cvs.red-bean.com:/usr/local/cvs co cvs2cl/cvs2cl.pl
        if ( `test ! -f /tmp/cvs2cl/cvs2cl.pl` ); then
            echo "Arghhh... download failed... please go, find & install cvs2cl.pl..."
            exit 1;
        fi
        cp /tmp/cvs2cl/cvs2cl.pl $MEGALIB/bin
        rm -rf /tmp/cvs2cl
    fi
fi

cd $MEGALIB
perl $MEGALIB/bin/cvs2cl.pl -f doc/ChangeLog -W 10 -S
rm -f doc/ChangeLog.bak

exit 0