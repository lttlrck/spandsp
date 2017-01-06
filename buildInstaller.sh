#!/bin/bash

rm -rf ./installer
mkdir ./installer
mkdir -p ./installer/test-data/itu/fax/

cp ./fftw*.rpm ./installer/
cp ./tests/.libs/fax_tests ./installer/
cp ./tests/.libs/t38_terminal_tests ./installer/
cp ./test-data/itu/fax/R1200_1200_A4.tif ./installer/test-data/itu/fax/
cp ./test-data/itu/fax/itu8.tif ./installer/test-data/itu/fax/
cp ./src/.libs/libspandsp.so.2 ./installer/
cp ./spandsp-sim/.libs/libspandsp-sim.so.2 ./installer/

echo '#!/bin/bash' >> ./installer/install.sh
echo 'ln -s /home/protocolEngine/data/codebase/user/bin/spandsp/fax_tests /home/protocolEngine/bin/t30' >> ./installer/install.sh
echo 'ln -s /home/protocolEngine/data/codebase/user/bin/spandsp/t38_terminal_tests /home/protocolEngine/bin/t38' >> ./installer/install.sh
echo 'ln -s /home/protocolEngine/data/codebase/user/bin/spandsp/libspandsp-sim.so.2 /home/protocolEngine/bin/libspandsp-sim.so.2' >> ./installer/install.sh
echo 'ln -s /home/protocolEngine/data/codebase/user/bin/spandsp/libspandsp.so.2 /home/protocolEngine/bin/libspandsp.so.2' >> ./installer/install.sh
echo 'ln -s /home/protocolEngine/data/codebase/user/bin/spandsp/test-data faxdocs' >> ./installer/install.sh
echo 'rpm -i /home/protocolEngine/data/codebase/user/bin/spandsp/*.rpm --nodeps --replacefiles' >> ./installer/install.sh
echo 'rpm -U /home/protocolEngine/data/codebase/user/bin/spandsp/*.rpm --nodeps --replacefiles' >> ./installer/install.sh

chmod +x ./installer/install.sh

tar -cvzf spandsp.tar.gz -C ./installer .

