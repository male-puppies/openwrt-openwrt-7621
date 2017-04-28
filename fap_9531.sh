./scripts/feeds update -a
./scripts/feeds install -a
./feeds/puppies/rom/scripts/update_source.sh
#make clean
./feeds/puppies/rom/scripts/make.sh FAP_qca9531 make
#-j4
